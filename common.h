
#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>

#define TERM_COLOR_RED     "\x1b[31m"
#define TERM_COLOR_GREEN   "\x1b[32m"
#define TERM_COLOR_YELLOW  "\x1b[33m"
#define TERM_COLOR_BLUE    "\x1b[34m"
#define TERM_COLOR_MAGENTA "\x1b[35m"
#define TERM_COLOR_CYAN    "\x1b[36m"
#define TERM_COLOR_RESET   "\x1b[0m"

#define PROP_COLOR         TERM_COLOR_GREEN
#define WARN_COLOR         TERM_COLOR_YELLOW
#define COLOR_RESET        TERM_COLOR_RESET

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define debug_print(fmt, ...) \
            do { if (DEBUG_TEST) printf(fmt, __VA_ARGS__); } while (0)

// Function to call for exit. Should be called instead of exit() or _Exit().
extern void (*xexit)(int status);

// Wrapper for fork(), which sets xexit() to _Exit for child processes.
// Should be called instead of fork().
pid_t xfork();

// Terminates program abnormally with the given message.
void die(const char * format, ...);

// Memory and string wrapper functions, which die in case of not enough memory.
void* xcalloc(size_t count, size_t size);
void* xmalloc(size_t size);
void* xrealloc(void* ptr, size_t size);
char* xstrdup(const char* s1);

// Returns parent directory by absolute path.
char* parent_dir(char* absolute_path);

// Returns 1, if the given string is NULL or empty. Otherwise, returns 0.
int empty(const char* s);

// Returns 1 if the given dir exists, and 0, if it doesn't.
int dir_exists(const char* path);

// Creates a temp file with the given prefix (if non-NULL), write the given content into it (if non-NULL).
// Returns pointer to the file name, which should be freed later.
// Returns NULL on failure.
char* create_temp_file(const char* prefix, const char* content);

// Runs the given command line. Returns child process PID, or -1 on failure.
pid_t run_cmd_line(const char* cmd_line, int supress_output);

// Runs script with the given file name from the given directory. Returns child process PID, or -1 on failure.
pid_t run_script(const char* dir, const char* script_file_name);

struct list {
    void* data;
    struct list* next;
};

// Creates a new list node. If data is NULL, returns NULL. If next is not NULL, sets it as the next node for the newly created node. 
// I.e. adds a node as head to the existing list.
struct list* list_create(void* data, struct list* next);

// Frees the list. If free_data is NULL, performs shallow free (does not remove data for each node). 
// If free_data is not NULL, it is called for data of each element and should free it.
void list_free(struct list* list, void (*free_data)(void*));

// Adds a new node to the tail of the existing list. 'Head' and 'tail' should point to the list's head and tail respectively.
// '*head' and/or '*tail' may be NULL. 
void list_add(struct list** head, struct list** tail, void* data);

// Removes node from the head of the given list. Returns the node's data. 
void* list_pop(struct list** head);

#endif

