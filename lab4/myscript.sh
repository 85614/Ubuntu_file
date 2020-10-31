#! /bin/bash
echo "gcc main2.0.c  -o main -lpthread"
gcc main2.0.c  -o main -lpthread
echo "rm -rf ../copy"
rm -rf ../copy
echo
echo "./main . ../copy"
./main . ../copy
#echo "./main . ../copy --detail"
#./main . ../copy --detail
echo
echo "find ./ -type f -print0 | xargs -0 md5sum"
find ./ -type f -print0 | xargs -0 md5sum
echo "find ./copy/ -type f -print0 | xargs -0 md5sum"
find ../copy/ -type f -print0 | xargs -0 md5sum
