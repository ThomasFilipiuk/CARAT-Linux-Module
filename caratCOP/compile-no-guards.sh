make all CC=clang V=1
mv test.ko src
./clean.sh
make no-guards CC=clang V=1
mv src/test.ko .
cd user-interface
make
