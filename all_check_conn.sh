workdir=$(cd $(dirname $0); pwd)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${workdir}
./osm_data_check $1