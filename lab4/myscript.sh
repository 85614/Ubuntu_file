#! /bin/bash
echo "Compile"
echo "gcc muti-copy-dir2.1.c  -o muti-copy-dir -lpthread"
gcc muti-copy-dir2.1.c  -o muti-copy-dir -lpthread

echo "rm -rf ../lab4-copy"
rm -rf ../lab4-copy
echo
echo "Before copy"
echo "ls ../"
ls ../

echo
echo "Start copy"
echo "./muti-copy-dir . ../lab4-copy"
./muti-copy-dir . ../lab4-copy
#echo "./muti-copy-dir . ../lab4-copy --detail"
#./muti-copy-dir . ../lab4-copy --detail
echo

echo "After copy"
echo "find ./ -type f -print0 | xargs -0 md5sum"
find ./ -type f -print0 | xargs -0 md5sum
echo "find ../lab4-copy/ -type f -print0 | xargs -0 md5sum"
find ../lab4-copy/ -type f -print0 | xargs -0 md5sum
