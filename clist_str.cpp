#include "clist_str.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * @note fdumo - name of .html dump file
 * @note gdump - name of buffer file to graphviz visualistion
 */
const char* fdump = "dump.html";
const char* gdump = "dmp.dot";

void listCtor (List* lst, const size_t len) {
    if (lst == NULL) {
        return;
    }

    if (lst->arr == NULL) {
        lst->arr = (ElemList*)malloc((len + 1) * sizeof(ElemList));

        lst->arr[0].next  = 0;
        lst->arr[0].prev  = 0;
        lst->arr[0].value = POISON;

        for (size_t i = 1; i < len + 1; i++) {
            lst->arr[i].next  = (i + 1) % (len + 1);
            lst->arr[i].prev  = -1;
            lst->arr[i].value = POISON;
        }
    } else if (len > lst->capacity) {
        lst->arr = (ElemList*)realloc(lst->arr, (len + 1) * sizeof(ElemList));

        if (lst->free == 0) {
            lst->free = lst->capacity + 1;
        } else {
            size_t buf = lst->free;
            while (lst->arr[buf].next != 0) {
                buf = lst->arr[buf].next;
            }
            lst->arr[buf].next = lst->capacity + 1;
        }

        for (size_t i = lst->capacity + 1; i < len + 1; i++) {  /// !!!!!!!!!!!!!!
            lst->arr[i].next  = (i + 1) % (len + 1);
            lst->arr[i].prev  = -1;
            lst->arr[i].value = POISON;
        }
    } else {
        return;
    }

    lst->capacity = len;
}

void listNew (List* lst) {
    lst->arr  = NULL;
    lst->free = 1;
    lst->size = 0;
    lst->capacity = 1;
    lst->fastfind = false;

    listCtor(lst, 1);
}

size_t listAdd (List* lst, const size_t pos, const Elem_t value) {
    if (lst->arr[pos].prev == EMPTY) {
        return EMPTY;
    }

    if (lst->capacity <= lst->size) {
        listCtor(lst, 2 * lst->capacity);
    }

    size_t freebuf = lst->free;

    lst->fastfind = false;

    lst->free = lst->arr[freebuf].next;

    lst->arr[freebuf].value.val  = value.val;
    lst->arr[freebuf].value.name = (char*)calloc(strlen(value.name) + 1, sizeof(char));
    strcpy(lst->arr[freebuf].value.name, value.name);

    lst->arr[freebuf].next  = lst->arr[pos].next;
    lst->arr[freebuf].prev  = pos;

    lst->arr[lst->arr[pos].next].prev = freebuf;
    lst->arr[pos].next = freebuf;

    lst->size++;

    return freebuf;    
}

Elem_t listRem (List* lst, const size_t pos) {
    if (pos == 0) {
        return POISON;
    }

    if (lst->arr[pos].prev == EMPTY) {
        return POISON;
    }

    lst->fastfind = false;

    Elem_t buf = lst->arr[pos].value;

    lst->arr[lst->arr[pos].next].prev = lst->arr[pos].prev;
    lst->arr[lst->arr[pos].prev].next = lst->arr[pos].next;

    lst->arr[pos].value = POISON;
    lst->arr[pos].next  = lst->free;
    lst->arr[pos].prev  = -1;

    lst->free = pos;

    lst->size--;

    return buf;
}

void listDump (List* lst) {
    /// make graphviz visualisation
    FILE* graph = fopen(gdump, "w");
    fprintf(graph, "digraph g {\n\t\trankdir=LR\t{\n\t\tnode [shape=record];\n");
    for (size_t i = 0; i <= lst->capacity; i++) {
        if (lst->arr[i].prev != EMPTY) {
            fprintf(graph, "\t\tstruct%ld [label=\"<id>Num: %ld | value: %s = %lf | {<pr>prev: %ld| <nt>next: %ld}\" color=\"olivedrab1\"];\n",
                            i, i, lst->arr[i].value.name, lst->arr[i].value.val, lst->arr[i].prev, lst->arr[i].next);
        } else {
            fprintf(graph, "\t\tstruct%ld [label=\"<id>Num: %ld | value: %s = %lf | {<pr>prev: %ld| <nt>next: %ld}\" color=\"firebrick3\"];\n",
                            i, i, "empty", lst->arr[i].value.val, lst->arr[i].prev, lst->arr[i].next);
        }
    }

    fprintf(graph, "\n\t\t free\n\t}\n\n");

    for (size_t i = 0; i < lst->capacity; i++) {
        fprintf(graph, "\tstruct%ld:id -> struct%ld:id[style=\"invis\" weight=\"1000\"]\n", i, i + 1);
    }

    for (size_t i = 0; i <= lst->capacity; i++) {
        if (lst->arr[i].prev != EMPTY) {
            fprintf(graph, "\tstruct%ld:nt -> struct%ld:pr\n", i, lst->arr[i].next);
        } else {
            fprintf(graph, "\tstruct%ld:nt -> struct%ld:pr\n", i, lst->arr[i].next);
        }
    }
    fprintf(graph, "\n\tfree -> struct%ld\n}", lst->free);

    fclose(graph);

    // make html dump

    static size_t iter = 0;
    char comand[100] = "";
    sprintf(comand, "dot -Tpng dmp.dot > source/pic%ld.png", iter);
    system(comand);

    FILE* hdump = fopen(fdump, "a");

    if (iter == 0) {
        fclose(hdump);
        hdump = fopen(fdump, "w");
        fprintf(hdump, "<pre>\n\n");
    }

    fprintf(hdump, "<h2>DUMP no:%ld</h2>\n\ncapasity: %ld\tsize: %ld\n", iter, lst->capacity, lst->size);

    /*for (size_t i = 0; i <= lst->capacity; i++) {
        fprintf(hdump, "\t%c[%ld]:\t%s\tnt: %ld\tpr: %ld\n", (lst->arr[i].prev != EMPTY) ? (' ') : ('*'),
                                            i, lst->arr[i].value, lst->arr[i].next, lst->arr[i].prev);
    }*/

    fprintf(hdump, "\t<img src=\"source/pic%ld.png\" alt=\"Dump no: %ld\" height=\"150\">\n", iter, iter);

    iter++;
    fclose(hdump);
}

void listLin  (List* lst) { /// make hard
    ElemList* bufarr = (ElemList*)malloc((lst->capacity + 1) * sizeof(ElemList));

    size_t bufpos = 0;

    for (size_t i = 0; i <= lst->size; i++) {
        bufarr[i] = lst->arr[bufpos];
        bufpos = lst->arr[bufpos].next;
    }

    free(lst->arr);
    lst->arr = bufarr;
    
    for (size_t i = 0; i <= lst->size; i++) {
        lst->arr[i].next = (lst->size + i + 2) % (lst->size + 1);
        lst->arr[i].prev = (lst->size + i) % (lst->size + 1);
    }

    lst->free = lst->size + 1;
    lst->fastfind = true;

    for (size_t i = lst->free; i <= lst->capacity; i++) {
        lst->arr[i].value = POISON;
        lst->arr[i].next = (lst->capacity + i + 2) % (lst->capacity + 1);
        lst->arr[i].prev = EMPTY;
    }
}

size_t listFind (const List* lst, const size_t pos) { /// make hard
    if (lst->fastfind) {
        return lst->arr[0].next + pos - 1; // другая формула
    }
    
    size_t posbuf = lst->arr[0].next;

    for (size_t i = 0; i < lst->size; i++) {
        if (pos == posbuf) return (i + 1);

        posbuf = lst->arr[posbuf].next;
    }

    return EMPTY;
}

size_t listSearch (const List* lst, const char* str) {
    size_t pos = lst->arr[0].next;
    for (size_t i = 0; i < lst->size; i++) {
        if (!strcmp(str, lst->arr[pos].value.name)) {
            return pos;
        }

        pos = lst->arr[pos].next;
    }

    return EMPTY;
}
