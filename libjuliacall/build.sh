libpython_files=$(python -m find_libpython)
if [ -z "$libpython_files" ]; then
    echo "libpython not found"
    exit 1
fi

# replace it with your include
include_dir="C:/Users/TR/AppData/Local/miniforge3/include"


lib_dir=$(dirname "$libpython_files")
full_basename=$(basename "$libpython_files")
libpython="${full_basename%.*}"
echo "libpython: $libpython"
echo "lib_dir: $lib_dir"
echo "include_dir: $include_dir"



g++ -fPIC -shared juliacall.cpp -o libjuliacall.dll -I./include -I$include_dir -L$lib_dir -l$libpython
cp libjuliacall.dll ../tyjuliasetup/src/libjuliacall.dll

