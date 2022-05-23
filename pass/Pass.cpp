// This llvm pass is intended to be a final pass before

#include "llvm/Pass.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#include <deque>
#include <queue>
#include <set>
#include <unordered_map>

using namespace llvm;

// This implements intel MPX instructions. If this is 0 it will do bounds
// checking instead
#define CYCLE_GUARD 1
#include "./texas.h"

struct Guard {
  llvm::Instruction *I = nullptr;
  llvm::Value *pointer = nullptr;
  int flags = 0;
  Guard() = default;

  Guard(Instruction *I, Value *p, int flags) : I(I), pointer(p), flags(flags) {
    // errs() << "created guard of " << *p << " with flags " << flags << "\n";
  }
};

struct TexasProtectionPass : public ModulePass {
  static char ID;

  TexasProtectionPass();
  bool runOnModule(Module &M) override;
  bool doInitialization(Module &M) override;
  Value *getInternalPointer(Value *ptr);
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};



TexasProtectionPass::TexasProtectionPass(void) : ModulePass(ID) {}
bool TexasProtectionPass::doInitialization(Module &M) { return false; }
void TexasProtectionPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<LoopInfoWrapperPass>();
  return;
}


Value *TexasProtectionPass::getInternalPointer(Value *ptr) {
  if (auto gep = dyn_cast<GetElementPtrInst>(ptr))
    return gep->getPointerOperand();

  if (auto bitcast = dyn_cast<BitCastInst>(ptr)) {
    auto casted_from = bitcast->getOperand(0);
    if (auto ptype = dyn_cast<PointerType>(casted_from->getType())) {
      return casted_from;
    }
  }

  return NULL;
}

bool TexasProtectionPass::runOnModule(Module &M) {
  std::set<std::string> functionsInProgram;
  std::unordered_map<std::string, int> functionCalls;
  bool modified = false;

  std::map<std::string, uint64_t> metrics;

  // COUNTERS

  LLVMContext &ctx = M.getContext();
  auto int32Type = Type::getInt32Ty(ctx);
  auto int64Type = Type::getInt64Ty(ctx);
  auto voidPtrType = Type::getInt8PtrTy(ctx, 0);

  auto guardFunctionType = FunctionType::get(
      Type::getVoidTy(ctx), {voidPtrType, int32Type}, false);
  auto guardFunction = M.getOrInsertFunction("texas_guard", guardFunctionType);

  ConstantInt *nonsenseConstant =
      ConstantInt::get(ctx, llvm::APInt(64, 0, false));
  Constant *nonsensePointer =
      ConstantExpr::getIntToPtr(nonsenseConstant, voidPtrType, false);

  // This is populated for every function in the entire module. It's state is
  // not finalized until the following for loop finished executing.
  std::unordered_map<Instruction *, Guard> allGuards;

  // Go through every function in the module and find all important guards
  for (auto &F : M) {
    if (F.empty()) {
      continue;
    }

    auto *firstBB = &*F.begin();
    auto *firstInst = &*firstBB->begin();
    std::unordered_map<Instruction *, Guard> toGuard;

    auto insertGuard = [&](Instruction *inst, Value *pointer,
                           int flags) -> void {
      // Get the internal pointer recursively (Basically, if we have a GEP,
      // determine if it's from an alloca or not). This may be imperfect w/
      // the access size argument, but that should be used for debugging
      // anyways.
			auto baseptr = pointer;
      while (auto internal = getInternalPointer(baseptr)) {
        metrics["internal recurse"]++;
        baseptr = internal;
      }

      // Don't guard alloca instructions. They are on the stack and we will
      // assume it is valid to access them (for optimization reasons). The
      // backend will read and write the stack even if we don't guard it
      // anyways, so it's pointless here.
      if (isa<AllocaInst>(baseptr)) {
        flags = 0;
        metrics["stack"]++;
        return;
      }

			// If it wasn't eventually a guard, use the original pointer for guards
			// to avoid funky geps bypassing protection passes

      // If the pointer comes from a place that the runtime guarentees will
      // be valid, we can avoid guarding it.
      if (auto call = dyn_cast<CallInst>(pointer)) {
        auto func = call->getCalledFunction();
        if (func != nullptr) {
          auto name = func->getName();
          if (name.equals("malloc") || name.equals("calloc")) {
            metrics["redundant"]++;
            flags = 0;
            return;
          }
        }
      }

      toGuard[inst] = Guard(inst, pointer, flags);
      return;
    };

    // Go over all the loads and stores
    for (auto &B : F) {
      for (auto &I : B) {
        if (isa<StoreInst>(I)) {
          metrics["stores"]++;
          auto storeInst = cast<StoreInst>(&I);
          auto pointerOfStore = storeInst->getPointerOperand();
          insertGuard(&I, pointerOfStore, TEXAS_GUARD_STORE);
          continue;
        }

        if (isa<LoadInst>(I)) {
          metrics["loads"]++;
          auto loadInst = cast<LoadInst>(&I);
          auto pointerOfStore = loadInst->getPointerOperand();
          insertGuard(&I, pointerOfStore, TEXAS_GUARD_LOAD);
          continue;
        }
      }
    }

    // Record the guards globally
    for (auto &kv : toGuard) {
      if (kv.second.flags == 0) {
        continue;
      }
      allGuards[kv.first] = kv.second;
    }
  }

  // Insert all the guards that we need to
  for (auto &kv : allGuards) {
    auto guard = kv.second;

    IRBuilder<> builder(guard.I);

    std::vector<Value *> args;
    auto ptr = builder.CreatePointerCast(guard.pointer, voidPtrType);
    args.push_back(ptr);
    args.push_back(ConstantInt::get(ctx, llvm::APInt(32, guard.flags, false)));

    builder.CreateCall(guardFunction, args);
    metrics["guards"]++;
  }
  fprintf(stderr, "---- Metrics ----\n");
  for (auto &kv : metrics) {
    fprintf(stderr, " %7s: %lu\n", kv.first.data(), kv.second);
  }

  return true;
}

char TexasProtectionPass::ID = 0;
static RegisterPass<TexasProtectionPass> X("TexasProtection",
                                           "Bounds protection for CARAT");

static TexasProtectionPass *_PassMaker = NULL;
static RegisterStandardPasses
    _RegPass1(PassManagerBuilder::EP_OptimizerLast,
              [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
                if (!_PassMaker) {
                  PM.add(_PassMaker = new TexasProtectionPass());
                }
              }); // ** for -Ox
static RegisterStandardPasses
    _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
              [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
                if (!_PassMaker) {
                  PM.add(_PassMaker = new TexasProtectionPass());
                }
              });
