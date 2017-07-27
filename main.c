#include <stdio.h>
#include <stdlib.h>
#include "bf.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s [file]\n", argv[0]);
        exit(0);
    }
    jit_context_t context = jit_context_create();

    char * buffer = 0;
    long length;
    FILE * f = fopen(argv[1], "rb");

    if (f) {
      fseek(f, 0, SEEK_END);
      length = ftell(f);
      fseek(f, 0, SEEK_SET);
      buffer = malloc(length + 1);
      if (buffer) {
        fread(buffer, 1, length, f);
      }
      fclose(f);
    } else {
        printf("Coudn't open file '%s'\n", argv[1]);
        exit(1);
    }
    buffer[length] = 0;

    Function function = get_compiled(context, buffer);
    function();
    jit_context_destroy(context);
    free(buffer);
    return 0;
}
