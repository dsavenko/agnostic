
#ifndef RUN_CMD
#define RUN_CMD

#include <unistd.h>

pid_t run_cmd_line(const char* cmd_line);

void die(const char * format, ...);

#endif // RUN_CMD

