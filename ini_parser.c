#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "queue.h"
#include "ini_parser.h"

struct ini_element {
        char *name; /** element name */
        char *value; /** element value */
        SIMPLEQ_ENTRY(ini_element) next;
};

struct ini_section {
        char *name; /** section name */
        SIMPLEQ_ENTRY(ini_section) next;
        SIMPLEQ_HEAD(elem_head, ini_element) elem_head;
};

struct ini_context {
        char *name; /** context name, just filename here */
        SIMPLEQ_HEAD(sec_head, ini_section) sec_head;
};
        
static struct ini_context *parse_file(FILE *fp)
{
        char *line = NULL;
        size_t len = 0;
        ssize_t read = 0;
        struct ini_context *ctx = NULL;
        struct ini_section *last_sec = NULL;

        ctx = (struct ini_context *)malloc(sizeof(*ctx));
        if (!ctx) {
                return NULL;
        }
        SIMPLEQ_INIT(&ctx->sec_head);
        
        while ((read = getline(&line, &len, fp)) != -1) {
                char *p = line;

                while (*p == ' ')p++; /** trim leading whitespaces */
                if (strlen(p) <= 1) continue; /** skip blank line */
                else if (*p == '#') continue; /** skip comment */
                else if (*p == '[') { /** section start */
                        p++; /** skip [ */
                        last_sec = (struct ini_section *)
                                malloc(sizeof(*last_sec));
                        SIMPLEQ_INIT(&last_sec->elem_head);
                        if (!last_sec) continue;
                        /** trim whitespaces between [ and section name */
                        while (*p == ' ')p++;
                        /** copy section name */
                        last_sec->name = strdup(p);
                        /** trim whitespaces between section name and ] */
                        p = last_sec->name;
                        while (*p) {
                                if (*p == ' ' || *p == ']') {
                                        *p = '\0';
                                        break;
                                }
                                p++;
                        }

                        /** Insert the section to context */
                        SIMPLEQ_INSERT_TAIL(&ctx->sec_head, last_sec, next);
                } else { /** element */
                        struct ini_element *elem = NULL;
                        char *tmp = NULL;

                        elem = (struct ini_element *)malloc(sizeof(*elem));
                        tmp = strtok(p, "=");
                        elem->name = strdup(tmp);
                        tmp = strtok(NULL, "=");
                        while (*tmp == ' ')tmp++; /** trim leading whitespaces */
                        elem->value = strdup(tmp);
                        /** trim whitespaces for name and value */
                        tmp = elem->name;
                        while (*tmp) {
                                if (*tmp == ' ') {
                                        *tmp = '\0';
                                        break;
                                }
                                tmp++;
                        }
                        tmp = elem->value;
                        while (*tmp) {
                                if (*tmp == '\n') {
                                        *tmp = '\0';
                                        break;
                                }
                                tmp++;
                        }
                        SIMPLEQ_INSERT_TAIL(&last_sec->elem_head, elem, next);
                }
        }
        free(line);
        return ctx;
}

struct ini_context *ini_load(const char *filename)
{
        struct ini_context *ctx = NULL;
        FILE *fp = NULL;

        if (!filename) {
                return NULL;
        }
        
        fp = fopen(filename, "r");
        if (!fp) {
                return NULL;
        }

        ctx = parse_file(fp);
        if (ctx) {
                ctx->name = strdup(filename);
        }

        fclose(fp);
        return ctx;
}

void ini_unload(struct ini_context *ctx)
{
        struct ini_section *sec = NULL;
        struct ini_element *elem = NULL;
        
        if (!ctx) {
                return;
        }

        while (!SIMPLEQ_EMPTY(&ctx->sec_head)) {
                sec = SIMPLEQ_FIRST(&ctx->sec_head);
                SIMPLEQ_REMOVE_HEAD(&ctx->sec_head, next);
                while (!SIMPLEQ_EMPTY(&sec->elem_head)) {
                        elem = SIMPLEQ_FIRST(&sec->elem_head);
                        SIMPLEQ_REMOVE_HEAD(&sec->elem_head, next);
                        free(elem->name);
                        free(elem->value);
                        free(elem);
                }
                free(sec->name);
                free(sec);
        }

        free(ctx->name);
        free(ctx);
        ctx = NULL;
}

void ini_dump(struct ini_context *ctx)
{
        struct ini_section *sec = NULL;
        struct ini_element *elem = NULL;
        
        if (!ctx) {
                return;
        }
        
        SIMPLEQ_FOREACH(sec, &(ctx->sec_head), next) {
                printf("[%s]\n", sec->name);
                SIMPLEQ_FOREACH(elem, &(sec->elem_head), next) {
                        printf("%s=%s\n", elem->name, elem->value);
                }
        }
}

char *ini_get_string(struct ini_context *ctx, const char *sec_name,
                     const char *elem_name, char *default_value)
{
        struct ini_section *sec = NULL;
        struct ini_element *elem = NULL;

        if (!ctx || !sec_name || !elem_name) {
                return default_value;
        }

        SIMPLEQ_FOREACH(sec, &(ctx->sec_head), next) {
                if (strcmp(sec->name, sec_name) != 0) {
                        continue;
                }
                SIMPLEQ_FOREACH(elem, &(sec->elem_head), next) {
                        if (strcmp(elem->name, elem_name) == 0) {
                                return elem->value;
                        }
                }
        }
        return default_value;
}

int ini_get_int(struct ini_context *ctx, const char *sec_name,
                const char *elem_name, int default_value)
{
        char *value = ini_get_string(ctx, sec_name, elem_name, NULL);
        if (!value) {
                return default_value;
        }
        return atoi(value);
}

int ini_get_bool(struct ini_context *ctx, const char *sec_name,
                 const char *elem_name, int default_value)
{
        char *value = ini_get_string(ctx, sec_name, elem_name, NULL);
        if (!value) {
                return default_value;
        }
        if (strcasecmp(value, "true") == 0) {
                return 1;
        } else if (strcasecmp(value, "false") == 0) {
                return 0;
        } else {
                return atoi(value) > 0;
        }
}

#ifdef DEBUG

int main(int argc, char **argv)
{
        struct ini_context *ctx = NULL;
        
        ctx = ini_load(argv[1]);
        ini_dump(ctx);
        printf("sec1->elem1 = %s\n",
               ini_get_string(ctx, "test_section1", "test_element1", "test1"));
        printf("name = %s\n", ini_get_string(ctx, "test", "name", "mackong"));
        ini_unload(ctx);
        
        return 0;
}

#endif /** DEBUG */
