#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios origin_termios;

void disableRawMode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_termios);
}

void enableRawMode(void) {
    /* tcgetattr() is used to read current attribute into a struct */
    tcgetattr(STDIN_FILENO, &origin_termios);

    /* 
     * atexit method is used when the program exits 
     * either by returning to main() or by calling exit()
     */
    atexit(disableRawMode);

    struct termios raw = origin_termios;

    /* 
     * ECHO features enable program to print each key that are pressed
     * the code we write to disable ECHO
     * the tilde (~) is a bitwise NOT OPERATOR
     * using ICON flag allow use to turn off canonical mode 
    */
    raw.c_lflag &= ~(ECHO | ICANON);

    /*
     * tcsetattr() method is a POSIX system call that sets terminal attributes/parameters
     * the first param is file descripter(filde) for the terminal(like STDIN_FILENO).
     * the second param is when to apply changes. since we are suing TCSAFLUSH, it will
     * discard any input that hasn't been read.
     * the third param is the pointer to the termios struc containing new attribute.
     * this method will return an integer (0 on success and -1 on error).
    */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void) {
    enableRawMode();

    char c;
    /* read method enable use to read one byte from standard input
     * into a varibale c
     * here we add when user press q, it will exit the program
    */
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    return 0;
}
