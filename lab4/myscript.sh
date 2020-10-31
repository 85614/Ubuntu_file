#! /bin/bash
echo "gcc main2.0.c  -o main -lpthread"
gcc main2.0.c  -o main -lpthread
echo "rm -rf ../lab4-copy"
rm -rf ../lab4-copy
echo
echo "./main . ../lab4-copy"
./main . ../lab4-copy
#echo "./main . ../lab4-copy --detail"
#./main . ../lab4-copy --detail
echo
echo "find ./ -type f -print0 | xargs -0 md5sum"
find ./ -type f -print0 | xargs -0 md5sum
echo "find ./lab4-copy/ -type f -print0 | xargs -0 md5sum"
find ../lab4-copy/ -type f -print0 | xargs -0 md5sum
