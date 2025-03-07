#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void) {
    int fd = open("mytest", O_CREATE | O_WRONLY);

    if (fd < 0) {
        printf(1, "Error: Cannot open file for writing\n");
        exit();
    }

    // Write multiple lines with correct lengths
    write(fd, "This is the first line\n", 23); // Length is 23
    write(fd, "This is the first line\n", 23); // Length is 23
    write(fd, "This is the first line\n", 23); // Length is 23
    write(fd, "This is the forth line\n", 23);  // Length is 24
    write(fd, "This is the forth line\n", 23);  // Length is 24
    write(fd, "This is the sixth line\n", 23);  // Length is 24

    // Close the file
    close(fd);
    exit();
}
