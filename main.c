#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct {
    uint64_t offset;
    uint8_t  length; // Why would you need it longer than 255?
    char*    file_name;
} Args;

char* next_arg(int* argc, char*** argv) {
    if (*argc < 0) return 0;

    char* arg = *argv[0];

    if (*argc > 0) {
        *argc -= 1;
        *argv += 1;
    }

    return arg;
}

void print_help_menu() {
    printf(
        "usage: moffset [-o <offset>] [-l <length>] [-f <file name>]\n"
        "               [-h | -help | --help]\n"
        "\n"
        "Arguments:\n"
        "       -o <offset>\n"
        "               The memory offset in the file.\n"
        "       -l <length>\n"
        "               The length of the offset region in the file.\n"
        "       -f <file name>\n"
        "               The name/path of the file you want to use.\n"
        "       -h | -home | --home\n"
        "               Shows this help menu.\n"
        "\n"
        "Examples:\n"
        "       moffset -o 512 -l 64 -f password.png\n"
    );
}

Args parse_arguments(int *argc, char*** argv) {
    Args arguments = {0};
    char* arg;
    if (*argc <= 0) {
        fprintf(stderr,
                "usage: moffset [-o <offset>] [-l <length>] [-f <file name>]\n"
                "               [-h | -help | --help]\n");
        exit(1);
    }
    while ((arg = next_arg(argc, argv))) {
        // Offset argument
        if (strcmp("-o", arg) == 0) {
            // Check if there is an argument after.
            if (!(arg = next_arg(argc, argv))) {
                fprintf(stderr,
                        "Expected an offset after argument '-o'.\n"
                        "But no argument was found.\n");
                exit(1);
            }
            // Get offset. If there is no number in the arg then the offset is 0.
            int offset = atoi(arg);
            arguments.offset = offset;
        } else
        // Length argument
        if (strcmp("-l", arg) == 0) {
            // Check if there is an argument after.
            if (!(arg = next_arg(argc, argv))) {
                fprintf(stderr,
                        "Expected an offset length after argument '-l'.\n"
                        "But no argument was found.\n");
                exit(1);
            }
            // Check if length is higher than zero.
            int length = atoi(arg);
            if (length <= 0) {
                fprintf(stderr,
                        "Expected an offset length that is higher than 0.\n");
                exit(1);
            }
            arguments.length = length;
        } else
        // File name argument
        if (strcmp("-f", arg) == 0) {
            // Check if there is an argument after.
            if (!(arg = next_arg(argc, argv))) {
                fprintf(stderr, 
                        "Expected a file path after argument '-f'.\n"
                        "But no argument was found.\n");
                exit(1);
            }
            arguments.file_name = arg;
        } else
        // Help menu argument
        if (strcmp("-h", arg) == 0
         || strcmp("-help", arg) == 0
         || strcmp("--help", arg) == 0) {
            print_help_menu();
            exit(0);
        }
        // Unknown argument
        else {
            fprintf(stderr, "Unknown argument: '%s'\n", arg);
            exit(0);
        }
    }

    // Check that valid arguments are given
    if (arguments.length <= 0) {
        fprintf(stderr, "Missing offset length.\n\n");
        print_help_menu();
        exit(1);
    }
    if (arguments.file_name == NULL) {
        fprintf(stderr, "Missing file name.\n\n");
        print_help_menu();
        exit(1);
    }

    return arguments;
}

bool file_exists(char* path) {
    return access(path, F_OK) != 0;
}

uint64_t read_entire_file(char* path, char** buffer) {
    uint64_t length = 0;
    FILE* file = fopen(path, "rb");

    if (file) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        *buffer = malloc(length);
        if (*buffer) {
            fread(*buffer, 1, length, file);
        }
        fclose(file);
    } else {
        fprintf(stderr, "Failed to read file '%s'.\n", path);
        exit(1);
    }

    if (buffer == NULL) {
        fprintf(stderr, "Failed to read file '%s'.\n", path);
        exit(1);
    }

    return length;
}

char* bytes_to_hex(char* data, uint8_t count) {
    const char* chars = "0123456789ABCDEF";
    char* hex_str = malloc(count * 2 + 1);
    if (!hex_str) {
        fprintf(stderr, "Buy more ram.\n");
        exit(1);
    }
    for (uint8_t i = 0; i < count; ++i) {
        uint8_t byte = data[i];
        uint8_t lower_nibble = byte & 0x0F;
        uint8_t upper_nibble = byte >> 4 & 0x0F;
        hex_str[i*2+0] = chars[upper_nibble];
        hex_str[i*2+1] = chars[lower_nibble];
    }
    hex_str[count*2] = 0;
    return hex_str;
}

int main(int argc, char** argv) {
    // skip arg 0
    next_arg(&argc, &argv);
    Args arguments = parse_arguments(&argc, &argv);

    if (file_exists(arguments.file_name)) {
        printf("File '%s' doesn't exist.\n", arguments.file_name);
        exit(1);
    }

    char* buffer = NULL;
    uint64_t length = read_entire_file(arguments.file_name, &buffer);

    if (arguments.offset > length) {
        fprintf(stderr,
                "Offset is outside of file.\n"
                "File length: %ld.\n",
                length);
        exit(1);
    }
    if (arguments.offset + arguments.length > length) {
        fprintf(stderr,
                "End of requested region is outside of file.\n"
                "End of reqion: %ld, length of file: %ld\n",
                arguments.offset + arguments.length,
                length);
        exit(1);
    }

    char* region = malloc(arguments.length);
    memcpy(region, buffer + arguments.offset, arguments.length);

    char* hex = bytes_to_hex(region, arguments.length);
    printf("%s\n", hex);

    free(hex);
    free(region);
    free(buffer);
}
