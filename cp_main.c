#include <stdio.h>
#include <stdlib.h>

int cp_main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s source destination\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *source = fopen(argv[1], "rb");
    if (source == NULL) {
        perror("Error opening source file");
        return EXIT_FAILURE;
    }

    FILE *dest = fopen(argv[2], "wb");
    if (dest == NULL) {
        perror("Error opening destination file");
        fclose(source);
        return EXIT_FAILURE;
    }

    char buffer[1024];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytes, dest) != bytes) {
            perror("Error writing to destination file");
            fclose(source);
            fclose(dest);
            return EXIT_FAILURE;
        }
    }

    if (ferror(source)) {
        perror("Error reading source file");
        fclose(source);
        fclose(dest);
        return EXIT_FAILURE;
    }

    fclose(source);
    fclose(dest);

    return EXIT_SUCCESS;
}
