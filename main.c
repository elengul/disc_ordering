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
    printf("  6. End Program\n");
    printf("\n  ? ");
}

void
namelist_import(struct namelist *list)
{
    char first[32], last[32];
    FILE *in = fopen("namelist.txt", "r");
    if (in == NULL) {
        printf("There is no namelist.txt in the current directory.\n");
        printf("Returning to the menu!\n\n");
        return;
    }
    while (fscanf(in, "%s %s\n", first, last) != EOF)
        namelist_push(list, first, last);
    fclose(in);
}

void
namelist_export(struct namelist *list)
{
    FILE *out = fopen("namelist.txt","w");
    for (size_t i = 0; i < list->count; i++)
        fprintf(out, "%s %s\n", list->first[i], list->last[i]);
}

void
namelist_print(struct namelist *list)
{
    printf("People currently on the list:\n");
    for (size_t i = 0; i < list->count; i++)
        printf("%zu. %s %s\n", i+1, list->first[i], list->last[i]);
}

void
namelist_augment(struct namelist *list)
{
    char first[32], last[32];
    char opt;
    unsigned pos;
    do {
        printf("Please enter the first name of the person to be added: ");
        scanf("%s", first);
        printf("Please enter the last name of the person to be added: ");
        scanf("%s", last);
        printf("Where should they appear on the current list? ");
        scanf("%u", &pos);
        if (pos > list->count)
            namelist_push(list, first, last);
        else {
            size_t idx = list->count - 1;
            namelist_push(list, list->first[idx], list->last[idx]);
            for (size_t i = idx-1; i >= (pos-1); i--) {
                strcpy(list->first[i+1], list->first[i]);
                strcpy(list->last[i+1], list->last[i]);
            }
            strcpy(list->first[pos-1], first);
            strcpy(list->last[pos-1], last);
        }
        do {
            printf("\nWould you like to enter another name? (y/n) ");
            scanf("%c", &opt);
            printf("\n");
        } while (!((opt == 'y') || (opt == 'n')));
    } while (opt != 'n');
}

void
generate_seating(struct namelist *list)
{
    size_t *order = malloc((1 + list->count)*sizeof(*order));
    unsigned is_odd = list->count % 2;
    size_t num =  is_odd ? (list->count - 1) : (list->count - 2);
    size_t idx = 0;
    for (;;) {
        order[idx++] = num;
        if (num == 0)
            break;
        num -= 2;
    }
    order[idx++] = 9999;
    num =  is_odd ? (list->count - 2) : (list->count - 1);
    for (size_t i = 1; i <= num; i += 2)
        order[idx++] = i;
    for (size_t i = 0; i <= list->count; i++) {
        if (order[i] == 9999)
            printf("%zu. Shi Ye\n", i);
        else
            printf("%zu. %s %s\n", i, list->first[order[i]],
                   list->last[order[i]]);
    }
    free(order);
}

int main(void)
{
    struct namelist *list = namelist_create();
    unsigned opt = 0;
    do {
        print_opt_list();
        scanf("%u", &opt);
        printf("\n");
        switch (opt) {
        case 1:
            namelist_import(list);
            break;
        case 2:
            namelist_print(list);
            break;
        case 3:
            namelist_augment(list);
            break;
        case 4:
            generate_seating(list);
            break;
        case 5:
            namelist_export(list);
            break;
        case 6:
            continue;
        default:
            printf("Please select a valid option!\n\n");
        }
    } while (opt != 6);
    namelist_destroy(list);
    return 0;
}
