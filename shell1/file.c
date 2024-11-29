#include <stdio.h>
#include "colors.h"

int main() {
    const char *message = "Hello, dear user! Happy to see you on MyShell.\nEnjoy.\n";

    print_rainbow_text("Hello, dear user! Happy to see you on MyShell.");
    printf("Typos and logic errors in cmd's are highlited in attention messages\n");
    print_rainbow_text("Enjoy.");
    print_error_message("Error: ", "Something went wrong. Please try again.");
    return 0;
}
