# embed.c
A simple 0-dependency program to embed text or binary files into your C or C++ program.

This repository includes a small CMakeLists.txt that is intended to live next to embed.c in your project. It provides `embed::embed` as an alias
for the exact path to the executable, so that it can be used for processing in `add_custom_command`.

## Example usage
```cmake
add_subdirectory(tools/embed)

set(EMBED_INPUT data/myfile.txt)
set(EMBED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/myfile.h")
add_custom_command(
    OUTPUT "${EMBED_OUTPUT}"
    COMMAND embed::embed "${EMBED_INPUT}" "${EMBED_OUTPUT}" myfile_txt
    DEPENDS "${EMBED_INPUT}" embed::embed
    COMMENT "Embedding ${EMBED_INPUT} into ${EMBED_OUTPUT}"
)
```
