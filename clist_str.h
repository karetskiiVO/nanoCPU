#ifndef CLIST
#define CLIST

#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "mathtypes.h"

/**
 * @brief Elem_t   - type of list elements
 * @brief Elem_out - flag to scanf
 * @brief POISON   - empty element
 */
typedef var_t Elem_t;
const Elem_t POISON = {NULL, NAN};
const size_t EMPTY  = (size_t)(-1);

typedef struct {
    Elem_t value;
    size_t next;
    size_t prev;
} ElemList;

typedef struct {
    ElemList* arr;
    size_t size;
    size_t capacity;
    size_t free;
    bool fastfind;
} List;

/**
 * @brief this function reserved memory to list
 * 
 * @param [in] lst - pointer to list struct
 * @param [in] len - new capacity 
 */
void listCtor (List* lst, const size_t len);

/**
 * @brief this function make empty list
 * 
 * @param [in] lst - pointer to list struct
 */
void listNew (List* lst);

/**
 * @brief this function add new element in list after (pos) position
 * 
 * @param [in] lst   - pointer to list struct 
 * @param [in] pos   - position after that you need to add element
 * @param [in] value - value of new element
 * 
 * @return size_t    - "physical" position in array of elements in list
 */
size_t listAdd (List* lst, const size_t pos, const Elem_t value);

/**
 * @brief this function remove element on (pos) position
 * 
 * @param [in] lst - pointer to list struct 
 * @param [in] pos - position of element that you remove
 * 
 * @return Elem_t  - value of removed element
 */
Elem_t listRem (List* lst, const size_t pos);

/**
 * @brief this function make graphical dump
 * 
 * @param [in] lst - pointer to list struct 
 */
void listDump (List* lst);

/**
 * @brief this function sorted list
 * 
 * @param [in] lst - pointer to list struct 
 * 
 * @note Attention, this function is slow
 */
void listLin  (List* lst);

/**
 * @brief this function fin logical position of element
 * 
 * @param [in] lst - pointer to list struct 
 * @param [in] pos - "physical" position of element
 * 
 * @return size_t - logical position of element
 * 
 * @note Attention, this function is slow if list wasn't sorted
 */
size_t listFind (const List* lst, const size_t pos);

size_t listSearch (const List* lst, const char* str);

#endif // clist.h