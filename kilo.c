#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios origin_termios;

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_termios) == -1)
    die("tcsetattr");
}

void enableRawMode(void) {
  /* tcgetattr() is used to read current attribute into a struct */
  if (tcgetattr(STDIN_FILENO, &origin_termios) == -1)
    die("tcgetattr");

  /*
   * atexit method is used when the program exits
   * either by returning to main() or by calling exit()
   */
  atexit(disableRawMode);

  struct termios raw = origin_termios;

  /**
   * adding BRKINT, INPCK, ISTRIP, and CS8 was considered (by someone) to be
   * part of enabling “raw mode”, so we carry on the tradition (of whoever that
   * someone was) in our program.
   */

  /**
   * By default, Ctrl-S and Ctrl-Q are used for software flow control.
   * Ctrl-S stops data from being transmitted to the terminal until you press
   * Ctrl-Q. IXON comes from <termios.h>. The I stands for “input flag” (which
   * it is, unlike the other I flags we’ve seen so far) and XON comes from the
   * names of the two control characters that Ctrl-S and Ctrl-Q produce: XOFF to
   * pause transmission and XON to resume transmission.
   *
   * turning off ctrl-m since it's being read as 10, when it should be 13.
   * ICRNL comes from <termios.h>. The I stands for “input flag”, CR stands for
   * “carriage return”, and NL stands for “new line”.
   *
   * When BRKINT is turned on, a break condition will cause a SIGINT signal to
   * be sent to the program, like pressing Ctrl-C. INPCK enables parity
   * checking, which doesn’t seem to apply to modern terminal emulators. ISTRIP
   * causes the 8th bit of each input byte to be stripped, meaning it will set
   * it to 0. This is probably already turned off.
   */
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  /**
   * It turns out that the terminal does a similar translation on the output
   * side. It translates each newline ("\n") we print into a carriage return
   * followed by a newline ("\r\n").
   * The terminal requires both of these characters in order to start a new line
   * of text. The carriage return moves the cursor back to the beginning of the
   * current line, and the newline moves the cursor down a line, scrolling the
   * screen if necessary.
   * To turn off all ouput processing features, we need to add OPOST flag.
   * O means it’s an output flag, and I assume POST stands for “post-processing
   * of output”.
   * If we run the program now, we’ll see that the newline characters we’re
   * printing are only moving the cursor down, and not to the left side of the
   * screen.
   * Since we want to print to new line we need to use "\r\n" instead of using
   * "\n" in printf method.
   */
  raw.c_oflag &= ~(OPOST);

  /**
   * CS8 is not a flag, it is a bit mask with multiple bits, which we set using
   * the bitwise-OR (|) operator unlike all the flags we are turning off. It
   * sets the character size (CS) to 8 bits per byte. On my system, it’s already
   * set that way.
   */
  raw.c_cflag |= (CS8);

  /*
   * ECHO features enable program to print each key that are pressed
   * the code we write to disable ECHO
   * the tilde (~) is a bitwise NOT OPERATOR
   *
   * using ICON flag allow us to turn off canonical mode
   *
   * using ISIG flag allow us to turn off `ctrl-c` and `ctrl-z` signal
   *
   * `ctrl-c` sends a SIGNINT signal to the current process which causes it to
   * terminate, `ctrl-z` sends a SIGSTP signal to the current process which
   * causes it to suspend.
   *
   * Turning off ctrl-v by adding IEXTEN flag. This also turning off ctrl-o in
   * macOS. Ctrl-V can now be read as a 22 byte, and Ctrl-O as a 15 byte.
   */
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  /**
   * set a timeout, so that read() returns if it doesn’t get any input for a
   * certain amount of time.
   *
   * They are indexes into the c_cc field, which stands for “control
   * characters”, an array of bytes that control various terminal settings.
   *
   * The VMIN value sets the minimum number of bytes of input needed before
   * read() can return. We set it to 0 so that read() returns as soon as there
   * is any input to be read.
   *
   * The VTIME value sets the maximum amount of time to wait before read()
   * returns.
   *
   * It is in tenths of a second, so we set it to 1/10 of a second, or 100
   * milliseconds. If read() times out, it will return 0, which makes sense
   * because its usual return value is the number of bytes read.
   */
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  /*
   * tcsetattr() method is a POSIX system call that sets terminal
   * attributes/parameters the first param is file descripter(filde) for the
   * terminal(like STDIN_FILENO). the second param is when to apply changes.
   * since we are suing TCSAFLUSH, it will discard any input that hasn't been
   * read. the third param is the pointer to the termios struc containing new
   * attribute. this method will return an integer (0 on success and -1 on
   * error).
   */
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void) {
  enableRawMode();

  /**
   * read method enable use to read one byte from standard input
   * into a varibale c
   * here we add when user press q, it will exit the program
   */
  while (1) {
    char c = '\0';
    read(STDIN_FILENO, &c, 1);

    /*
     * iscntrl() method comes from <ctype.h>
     * it uses is to tests whether a character is a control character based on
     * ASCII control characters are nonprintable character that we don't want to
     * print to the screen. ASCII code 0-31 are all control characters, and 127
     * is also a control character. ASCII code 32-126 are all printable.
     */
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }

    if (c == 'q')
      break;
  }
  return 0;
}
