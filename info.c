
#include <stdio.h>
#include <yaml.h>
#include <stdarg.h>
#include <stdbool.h>

static void die (const char * format, ...) {
    va_list vargs;
    va_start (vargs, format);
    vfprintf (stderr, format, vargs);
    fprintf (stderr, "\n");
    exit (1);
}

static const char* find_config_file() {
    return "ez/agnostic.yaml";
}

static int checkout() { 
    const char* cfg_file = find_config_file();
    if (!cfg_file) {
        die("Config file not found");
    }

    FILE *fh = fopen(cfg_file, "r");
    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        die("Failed to initialize parser!");
    }
    if (fh == NULL) {
        die("Failed to open file %s", cfg_file);
    }

    yaml_parser_set_input_file(&parser, fh);

    bool is_key = false;
    bool is_name = false;

    do {
        yaml_parser_scan(&parser, &token);
        switch(token.type) {
            case YAML_KEY_TOKEN:   
                is_key = true;
                break;
            case YAML_VALUE_TOKEN: 
                is_key = false;
                break;
            case YAML_SCALAR_TOKEN:  
                if (is_key) {
                    is_name = !strcmp((const char *)token.data.scalar.value, "name");
                } else {
                    if (is_name) {
                        printf("%s\n", token.data.scalar.value);
                        is_name = false;
                    }
                }
                break;
            default:
                break;
        }
        if(token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while(token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);

    yaml_parser_delete(&parser);
    fclose(fh);
    return 0;
}

static void help() {
    printf("%s\n", "help");
}

static void unknown_cmd(const char* cmd) {
    die("Unknown command: %s", cmd);
}

int main(int argc, char **av) {
    const char **argv = (const char **) av;

    if (1 >= argc) {
        help();
    }

    while (1 < argc) {
        --argc;
        ++argv;

        const char *cmd = *argv;

        if (!strcmp(cmd, "checkout")) {
            checkout();
            exit(0);
        } else if (!strcmp(cmd, "help")) {
            help();
        } else {
            unknown_cmd(cmd);
        }
    }

    return 0;
}
