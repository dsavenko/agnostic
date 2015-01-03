
#include "common.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void die(const char * format, ...) {
    va_list vargs;
    va_start (vargs, format);
    vfprintf (stderr, format, vargs);
    fprintf (stderr, "\n");
    exit (1);
}

pid_t run_cmd_line(const char* cmd_line) {
    pid_t child_pid = fork();
    if (0 == child_pid) {
        fclose(stdout);
        fclose(stderr);
        execl("/bin/sh", "sh", "-c", cmd_line, (char*)NULL);
        return -1;
    }
    return child_pid;
}
