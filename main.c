#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct namelist {
    size_t count, size;
    char **first;
    char **last;
};

struct namelist *
namelist_create(void)
{
    struct namelist *out = malloc(sizeof(*out));
    out->count = 0;
    out->size  = 16;
    out->first = malloc(out->size * sizeof(out->first[0]));
    out->last  = malloc(out->size * sizeof(out->last[0]));
    for (size_t i = 0; i < out->size; i++) {
        out->first[i] = NULL;
        out->last[i]  = NULL;
    }
    return out;
}

void
namelist_push(struct namelist *list, char *first, char *last)
{
    if (list->count == list->size) {
        size_t old  = list->size;
        list->size *= 2;
        list->first = realloc(list->first, list->size * sizeof(list->first[0]));
        list->last  = realloc(list->last, list->size * sizeof(list->last[0]));
        for (size_t i = old; i < list->size; i++) {
            list->first[i] = NULL;
            list->last[i]  = NULL;
        }
    }
    list->first[list->count] = malloc(32 * sizeof(list->first[0][0]));
    list->last[list->count]  = malloc(32 * sizeof(list->last[0][0]));
    strcpy(list->first[list->count], first);
    strcpy(list->last[list->count], last);
    list->count++;
}

void
namelist_destroy(struct namelist *list)
{
    for (size_t i = 0; i < list->count; i++) {
        free(list->first[i]);
        free(list->last[i]);
    }
    free(list->first);
    free(list->last);
}

void
print_opt_list(void)
{
    printf("Select from the following options:\n");
    printf("  1. Import list of names from file\n");
    printf("  2. Print current list of names\n");
    printf("  3. Add a new name to the list\n");
    printf("  4. Generate seating order\n");
    printf("  5. Save current list of names to file (WARNING: This will overwrite the previous file\n");
}

int main(void)
{
    struct namelist *list = namelist_create();
    print_opt_list();
    namelist_destroy(list);
    return 0;
}
