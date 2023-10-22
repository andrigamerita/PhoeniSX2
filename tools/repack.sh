#!/bin/bash
# usage: repack.sh file.a
# https://stackoverflow.com/questions/49630984/strip-remove-debug-symbols-and-archive-names-from-a-static-library

if [ -z "$1" ]; then
    echo "usage: repack file.a"
    exit 1
fi

if [ -d tmprepack ]; then
    /bin/rm -rf tmprepack
fi

mkdir tmprepack
cp $1 tmprepack
pushd tmprepack

basename=${1##*/}

ar xv $basename
/bin/rm -f $basename
i=1000
for p in *.o ; do
    strip -d $p
    mv $p ${i}.o
    ((i++))
done

ar crus $basename *.o
mv $basename ..

popd
/bin/rm -rf tmprepack
exit 0
