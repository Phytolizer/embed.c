# embed.c
A simple 0-dependency program to embed text or binary files into your C or C++ program.

This repository includes a small CMakeLists.txt that is intended to live next to embed.c in your project. It provides `embed::embed` as an alias
for the exact path to the executable, so that it can be used for processing in `add_custom_command`.
