libpython_files=$(python -m find_libpython)
if [ -z "$libpython_files" ]; then
    echo "error! libpython not found, try 'pip install find_libpython'"
    exit 1
fi

# replace it with your include
include_dir="C:/Users/TR/AppData/Local/miniforge3/include"

if [ -z "$include_dir" ]; then
    echo "error! python include not set, please set it in build.sh"
    exit 1
fi

lib_dir=$(dirname "$libpython_files")

echo "lib_dir: $lib_dir"
echo "include_dir: $include_dir"

g++ -fPIC -shared juliacall.cpp -o libjuliacall.dll -I. -I./include -I$include_dir -L$lib_dir -lpython3
cp libjuliacall.dll ../tyjuliasetup/src/libjuliacall.dll

