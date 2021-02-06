/** @file
 * Implementacja interfejsu struktury find_union
 * zmodyfikowanej pod wykorzystanie w zadaniu gamma.
 *
 * @author Jakub Bedełek
 */
 
#include "find_union.h"
#include <stdlib.h>

/**
 * Struktura przechowywująca tablice o rozmiarze liczba pól gry. 
 */
typedef struct array_f array_t;

/**
 * Struktura przechowująca dane potrzebne do wykonania algorytmu find_union.
 */
struct find_s {
   array_t *arrays;             /**< Struktura przechowywująca tablice o rozmiarze
                                     liczba pól gry. */
};

/**
 * Struktura przechowywująca tablice o rozmiarze liczba pól gry. 
 */
struct array_f {
   uint32_t rep;               /**< Tablica przyporządkowująca każdemu polu 
                                     reprezentanta. */
   uint32_t rank;              /**< Tablica przyporządkowującaa każdemu
                                     polu wielkość obszaru, do którego należy.*/
};

void init(find_t *f, uint32_t width, uint32_t height) {
   f->arrays = calloc((uint64_t)width * (uint64_t)height, sizeof(array_t));
}

void delete_funion(find_t *f) {
    free(f->arrays);
}

void fill_find(find_t *f, uint32_t width, uint32_t height) {
   for (uint32_t i = 0; i < width * height; i ++) {
      f->arrays[i].rank = 1;
      f->arrays[i].rep = i; 
   }
}

bool check_alloc_find(find_t *f) {
   return (f->arrays == NULL);
}

void increase_rank(find_t *f, uint32_t rep_number) {
   f->arrays[rep_number].rank++;
}


uint32_t find(uint32_t number1, find_t *f) {
   if (f->arrays[number1].rep != number1) 
      f->arrays[number1].rep = find(f->arrays[number1].rep, f); 
   
   return f->arrays[number1].rep; 
}


void funion(find_t *f, uint32_t number1, uint32_t val) {
   // Szukamy reprezentantów parametrów number1 i val. 
   uint32_t f1 = find(number1, f); 
   uint32_t f2 = find(val, f); 
   
   // Łączymy obaszary zawierające pola o numerze number1 i val w jeden obszar. 
   if (f->arrays[f1].rank >= f->arrays[f2].rank) {
      f->arrays[f1].rank += f->arrays[f2].rank; 
      f->arrays[f2].rep = f1;  
   }
   else {
      f->arrays[f2].rank += f->arrays[f1].rank; 
      f->arrays[f1].rep = f2;  
   }
}

void change_rep(find_t *f, uint32_t number1, uint32_t val) {
   f->arrays[number1].rep = val;
}

void change_rank(find_t *f, uint32_t number1, uint32_t val) {
   f->arrays[number1].rank = val;
}

find_t* new_find_t() {
   return malloc(sizeof(find_t));
}

void delete_funion_all(find_t *f) {
    if (f != NULL) {
       if (f->arrays != NULL) 
          free(f->arrays);
    }
}
