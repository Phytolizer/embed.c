cmake_minimum_required(VERSION 3.5)

project(embed LANGUAGES C)

add_executable(embed embed.c)
target_compile_features(embed PRIVATE c_std_90)
add_executable(embed::embed ALIAS embed)
