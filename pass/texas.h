#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The functions in this file are exclusively functions which are called by the
 * compiler pass at runtime. Functions used by the runtime are located in
 * `<texas/rt.h>`
 *
 * When porting to a new platform, stub out the functions you are interested in here
 */

#ifdef TEXAS_DEBUG
#define TEXAS_LOG(...) printf(__VA_ARGS__)
#else
#define TEXAS_LOG(...)
#endif

// Flags to @texas_guard
// Access flags
#define TEXAS_GUARD_LOAD (1 << 0)
#define TEXAS_GUARD_STORE (1 << 1)

// DEBUG for the runtime
#define TEXAS_GUARD_HOIST (1 << 2)
#define TEXAS_GUARD_MERGED (1 << 3)

// For the `TexasProtection` pass
void texas_guard(void *ptr, uint64_t size, int flags);

#ifdef __cplusplus
}
#endif
