#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct namelist {
    size_t count, size;
    char **first;
    char **last;
};

struct ordering {
    size_t count, size;
    size_t *idx;
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
namelist_pop(struct namelist *list, unsigned idx)
{
    free(list->first[idx]);
    free(list->last[idx]);
    for (size_t i = idx+1; i < list->count; i++) {
        list->first[i-1] = list->first[i];
        list->last[i-1]  = list->last[i];
    }
    list->first[list->count-1] = NULL;
    list->last[list->count-1]  = NULL;
    list->count--;
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
    free(list);
}

struct ordering *
ordering_create(void)
{
    struct ordering *out = malloc(sizeof(*out));
    out->size  = 8;
    out->count = 0;
    out->idx   = malloc(out->size * sizeof(out->idx[0]));
    return out;
}

void
ordering_push(struct ordering *order, size_t num)
{
    if (order->size == order->count) {
        order->size *= 2;
        order->idx   = realloc(order->idx, order->size * sizeof(order->idx[0]));
    }
    order->idx[order->count++] = num;
}

int
comparator(const void *p1, const void *p2)
{
    size_t l = *(const int *)p1;
    size_t r = *(const int *)p2;
    return (l-r);
}

void
ordering_sort(struct ordering *order)
{
    size_t *arr = order->idx;
    qsort((void *)arr, order->count, sizeof(arr[0]), comparator);
}

void
ordering_destroy(struct ordering *order)
{
    free(order->idx);
    free(order);
}

void
print_opt_list(void)
{
    printf("Select from the following options:\n");
    printf("  1. Import list of names from file\n");
    printf("  2. Print current list of names\n");
    printf("  3. Add a new name to the list\n");
    printf("  4. Remove a new name to the list\n");
    printf("  5. Change the position of a new name to the list\n");
    printf("  6. Generate seating order\n");
    printf("  7. Save current list of names to file (WARNING: This will overwrite the previous file)\n");
    printf("  8. End Program\n");
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
namelist_print(struct namelist *list, struct ordering *order)
{
    printf("People currently on the list:\n");
    for (size_t i = 0; i < list->count; i++) {
        if (order) {
            unsigned no_print = 0;
            for (size_t j = 0; j < order->count; j++)
                if (order->idx[j] == i) {
                    no_print = 1;
                    break;
                }
            if (no_print)
                continue;
        }
        printf("%zu. %s %s\n", i+1, list->first[i], list->last[i]);
    }
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
            scanf(" %c", &opt);
        } while (!((opt == 'y') || (opt == 'n')));
    } while (opt != 'n');
}

void
namelist_strip(struct namelist *list)
{
    unsigned idx;
    namelist_print(list, NULL);
    printf("Please select the number of the person to be removed: ");
    scanf("%u", &idx);
    namelist_pop(list, idx-1);
}

void
namelist_shuffle(struct namelist *list)
{
    unsigned idx1, idx2;
    namelist_print(list, NULL);
    printf("Please select the number of the person to be moved: ");
    scanf("%u", &idx1);
    printf("What position should they be? ");
    scanf(" %u", &idx2);
    char *F_temp = list->first[idx1-1];
    char *L_temp = list->last[idx1-1];
    if (idx1 < idx2)
        for (size_t i = idx1; i < idx2; i++) {
            list->first[i-1] = list->first[i];
            list->last[i-1]  = list->last[i];
        }
    else
        for (size_t i = idx1-1; i >= idx2; i--) {
            list->first[i] = list->first[i-1];
            list->last[i]  = list->last[i-1];
        }
    list->first[idx2-1] = F_temp;
    list->last[idx2-1]  = L_temp;
}

void
generate_seating(struct namelist *list)
{
    struct ordering *order = ordering_create();
    char cont = 'n';
    unsigned choice;
    do {
        namelist_print(list, order);
        printf("Please select a person that is here (by their number): ");
        scanf(" %u", &choice);
        ordering_push(order, choice-1);
        printf("Would you like to select another name? (y/n) ");
        scanf(" %c", &cont);
    } while (cont != 'n');
    ordering_sort(order);
    size_t seat_order[order->count];
    unsigned is_odd = order->count % 2;
    size_t num =  is_odd ? (order->count - 1) : (order->count - 2);
    size_t idx = 0;
    for (;;) {
        seat_order[idx++] = num;
        if (num == 0)
            break;
        num -= 2;
    }
    seat_order[idx++] = 9999;
    num =  is_odd ? (order->count - 2) : (order->count - 1);
    for (size_t i = 1; i <= num; i += 2)
        seat_order[idx++] = i;
    for (size_t i = 0; i <= order->count; i++) {
        if (seat_order[i] == 9999)
            printf("%zu. Shi Ye\n", i+1);
        else {
            idx = order->idx[seat_order[i]];
            printf("%zu. %s %s\n", i+1, list->first[idx],
                   list->last[idx]);
        }
    }
    ordering_destroy(order);
    return;
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
            namelist_print(list, NULL);
            break;
        case 3:
            namelist_augment(list);
            break;
        case 4:
            namelist_strip(list);
            break;
        case 5:
            namelist_shuffle(list);
            break;
        case 6:
            generate_seating(list);
            break;
        case 7:
            namelist_export(list);
            break;
        case 8:
            break;
        default:
            printf("Please select a valid option!\n\n");
        }
    } while (opt != 8);
    namelist_destroy(list);
    return 0;
}
