
#include "run-cmd.h"

#include <stdio.h>

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
