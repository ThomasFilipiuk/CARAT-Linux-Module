make CC=clang V=1 all
cp carat-cop.c test
cd test
make CC=clang V=1 all
cd ..
cp test/test.ko .
make -C user-interface user-space-app
