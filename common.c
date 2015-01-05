
#include "common.h"

#include <assert.h>
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
    assert(cmd_line);

    pid_t child_pid = fork();
    if (0 == child_pid) {
        fclose(stdout);
        fclose(stderr);
        execl("/bin/sh", "sh", "-c", cmd_line, (char*)NULL);
        return -1;
    }
    return child_pid;
}

char* create_temp_file(const char* prefix, const char* content) {
    char* fname = NULL;
    if (-1 == asprintf(&fname, "/tmp/%sXXXXXXXXXX", prefix)) {
        return NULL;
    }
    if (!mktemp(fname)) {
        free(fname);
        return NULL;
    }
    FILE* f = fopen(fname, "w");
    if (content) {
        fprintf(f, "%s", content);
    }
    fclose(f);
    return fname;
}

pid_t run_script(const char* dir, const char* script_file_name) {
    assert(script_file_name);

    pid_t child_pid = fork();
    if (0 == child_pid) {
        if (dir && chdir(dir)) {
            return -1;
        }
        execl("/bin/sh", "sh", "-xe", script_file_name, (char*)NULL);
        return -1;
    }
    return child_pid;
}
