#ifndef COLORS_H
#define COLORS_H

#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD_RED "\033[1;31m"

#define COLOR_BLACK   "\033[0;30m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN    "\033[0;36m"
#define COLOR_WHITE   "\033[0;37m"

#define COLOR_RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"

void print_rainbow_text(const char *text);
void print_error_message(const char *part1, const char *part2);

#endif
