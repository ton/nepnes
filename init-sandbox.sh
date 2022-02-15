export SANDBOX_NAME=nepnes
export LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}

sandbox_details_fn() {
  if [ -f build/CMakeCache.txt ]; then
    sed -n 's/^CMAKE_BUILD_TYPE:STRING=\(.*\)$/\L\1/p' build/CMakeCache.txt
  fi
}
