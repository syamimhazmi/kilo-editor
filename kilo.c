#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios origin_termios;

void disableRawMode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_termios);
}

void enableRawMode(void) {
    tcgetattr(STDIN_FILENO, &origin_termios);
    atexit(disableRawMode);

    struct termios raw = origin_termios;
    raw.c_lflag &= ~(ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void) {
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    return 0;
}
