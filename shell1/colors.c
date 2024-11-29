#include "colors.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

void print_rainbow_text(const char *text) {
    int length = strlen(text);
    for (int i = 0; i < length; i++) {
        int r = (int)(sin(0.3 * i + 0) * 127 + 128);
        int g = (int)(sin(0.3 * i + 2) * 127 + 128);
        int b = (int)(sin(0.3 * i + 4) * 127 + 128);

        printf("\033[38;2;%d;%d;%dm%c", r, g, b, text[i]);
    }
    printf(COLOR_RESET "\n");
}

void print_error_message(const char *part1, const char *part2) {
    fprintf(stderr, COLOR_BOLD_RED "%s" COLOR_RESET, part1);
    fprintf(stderr, "%s\n", part2);
}