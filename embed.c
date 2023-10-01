#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

typedef struct {
#ifdef _WIN32
    HANDLE handle;
#else
    int fd;
#endif
    int valid;
} platform_file;

static platform_file platform_fopen(char const* path) {
#ifdef _WIN32
    platform_file result;
    result.handle = CreateFileA(
            path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    result.valid = result.handle != INVALID_HANDLE_VALUE;
    return result;
#else
    platform_file result;
    result.fd = open(path, O_RDONLY);
    result.valid = result.fd != -1;
    return result;
#endif
}

static void platform_fclose(platform_file file) {
    if (file.valid) {
#ifdef _WIN32
        CloseHandle(file.handle);
#else
        close(file.fd);
#endif
    }
}

static size_t get_filelen(platform_file f) {
#ifdef _WIN32
    LARGE_INTEGER size;
    GetFileSizeEx(f.handle, &size);
    return (size_t)size.QuadPart;
#else
    struct stat st;
    fstat(f.fd, &st);
    return (size_t)st.st_size;
#endif
}

static size_t platform_fread(platform_file f, uint8_t* buf, size_t len) {
#ifdef _WIN32
    DWORD read;
    ReadFile(f.handle, buf, (DWORD)len, &read, NULL);
    return (size_t)read;
#else
    return (size_t)read(f.fd, buf, len);
#endif
}

static int slurp_file(
        char const* path, uint8_t** out_buf, size_t* out_len, char const** out_error) {
    size_t len;
    size_t nread;
    unsigned char* buf;
    platform_file f;

    f = platform_fopen(path);
    if (!f.valid) {
        *out_error = "failed to open file";
        return 0;
    }

    len = get_filelen(f);

    buf = malloc(len + 1);
    if (buf == NULL) {
        *out_error = "failed to allocate buffer";
        platform_fclose(f);
        return 0;
    }

    nread = platform_fread(f, buf, len);
    if (nread != len) {
        *out_error = "failed to read file";
        platform_fclose(f);
        return 0;
    }

    buf[len] = '\0';
    platform_fclose(f);

    *out_buf = buf;
    *out_len = len;
    return 1;
}

static char* make_header_guard(char const* name) {
    char* result;
    size_t input_len;

    input_len = strlen(name);
    result = malloc(input_len + 2);
    if (!result)
        abort();
    sprintf(result, "%s_", name);
    return result;
}

int main(int argc, char** argv) {
    char const* input_path;
    char const* output_path;
    char const* name;
    uint8_t* data;
    size_t data_len;
    char* header_guard;
    FILE* header_fp;
    char const* error;
    size_t i;

    if (argc != 4) {
        printf("Usage: %s <input> <output> <name>\n", argv[0]);
        return 1;
    }

    input_path = argv[1];
    output_path = argv[2];
    name = argv[3];
    if (!slurp_file(input_path, &data, &data_len, &error)) {
        printf("%s\n", error);
        return 1;
    }

    header_fp = fopen(output_path, "w");
    if (header_fp == NULL) {
        printf("Could not open %s for writing\n", argv[2]);
        free(data);
        return 1;
    }

    header_guard = make_header_guard(name);
    fprintf(header_fp, "#ifndef %s\n", header_guard);
    fprintf(header_fp, "#define %s\n", header_guard);
    fprintf(header_fp, "static unsigned char %s[%zu]={", name, data_len);
    for (i = 0; i < data_len; i++) {
        fprintf(header_fp, "%hhu,", data[i]);
    }
    fprintf(header_fp, "};\n#endif\n");
    fclose(header_fp);

    free(header_guard);
    free(data);
    return 0;
}
