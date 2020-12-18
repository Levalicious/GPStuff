cmake_minimum_required(VERSION 3.16)
project(RNS2 C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "")

add_executable(RNS2 main.c vm.c vm.h rns.c rns.h util.h)

target_link_libraries(RNS2 -lgmp -lm)