#!/bin/bash
dir=$(cd $(dirname $0); pwd)
rm -rf $dir/build_release
mkdir $dir/build_release
cd $dir/build_release
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make $1 -j4
