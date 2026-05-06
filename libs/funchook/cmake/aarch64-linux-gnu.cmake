# /^-----^\   data: 2026-04-30
# V  o o  V  plik: libs/funchook/cmake/aarch64-linux-gnu.cmake
#  |  Y  |   autor: pupnoodle
#   \ Q /
#   / - \
#   |    \
#   |     \     )
#   || (___\====

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(TOOLCHAIN_PREFIX aarch64-linux-gnu)

# cross compilers to use for C and C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})
