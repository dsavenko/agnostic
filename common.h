
#ifndef RUN_CMD
#define RUN_CMD

#include <unistd.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define debug_print(fmt, ...) \
            do { if (DEBUG_TEST) printf(fmt, __VA_ARGS__); } while (0)

// Terminates program abnormally with the given message.
void die(const char * format, ...);

// Creates a temp file with the given prefix (if non-NULL), write the given content into it (if non-NULL).
// Returns pointer to the file name, which should be freed later.
// Returns NULL on failure.
char* create_temp_file(const char* prefix, const char* content);

// Runs the given command line. Returns child process PID, or -1 on failure.
pid_t run_cmd_line(const char* cmd_line);

// Runs script with the given file name from the given directory. Returns child process PID, or -1 on failure.
pid_t run_script(const char* dir, const char* script_file_name);

#endif // RUN_CMD

