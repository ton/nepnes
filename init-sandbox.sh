export SANDBOX_NAME=nepnes
export LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}

sandbox_details_fn() {
  cmake_cache_file=~/dev/emulation/nes/nepnes/build/CMakeCache.txt
  if [ -f "$cmake_cache_file" ]; then
    sed -n 's/^CMAKE_BUILD_TYPE:STRING=\(.*\)$/\L\1/p' "$cmake_cache_file"
  fi
}
