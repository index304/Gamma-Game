/** @file
 * Funkcja main, start programu.
 */

/* CMake w wersji release wyłącza asercje. */
#ifdef NDEBUG
  #undef NDEBUG
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "gamma.h"

/** @brief Główna funkcja testująca program.
 * @return Zwraca 0.
 */
int main() {
   const char board[] =
    "1.........\n"
    "..........\n"
    "..........\n"
    "......2...\n"
    ".....2....\n"
    "..........\n"
    "..........\n"
    "1.........\n"
    "1221......\n"
    "1.........\n";

   gamma_t *g;

   g = gamma_new(0, 0, 0, 0);
   assert(g == NULL);
   
   g = gamma_new(10, 10, 2, 3);
   assert(g != NULL);
   
   assert(gamma_move(g, 1, 0, 0));
   assert(gamma_busy_fields(g, 1) == 1);
   assert(gamma_busy_fields(g, 2) == 0);
   assert(gamma_free_fields(g, 1) == 99);
   assert(gamma_free_fields(g, 2) == 99);
   assert(!gamma_golden_possible(g, 1));
   assert(gamma_move(g, 2, 3, 1));
   assert(gamma_busy_fields(g, 1) == 1);
   assert(gamma_busy_fields(g, 2) == 1);
   assert(gamma_free_fields(g, 1) == 98);
   assert(gamma_free_fields(g, 2) == 98);
   assert(gamma_move(g, 1, 0, 2));
   assert(gamma_move(g, 1, 0, 9));
   assert(!gamma_move(g, 1, 5, 5));
   assert(gamma_free_fields(g, 1) == 6);
   assert(gamma_move(g, 1, 0, 1));
   assert(gamma_free_fields(g, 1) == 95);
   assert(gamma_move(g, 1, 5, 5));
   assert(!gamma_move(g, 1, 6, 6));
   assert(gamma_busy_fields(g, 1) == 5);
   assert(gamma_free_fields(g, 1) == 10);
   assert(gamma_move(g, 2, 2, 1));
   assert(gamma_move(g, 2, 1, 1));
   assert(gamma_free_fields(g, 1) == 9);
   assert(gamma_free_fields(g, 2) == 92);
   assert(!gamma_move(g, 2, 0, 1));
   assert(gamma_golden_possible(g, 2));
   assert(!gamma_golden_move(g, 2, 0, 1));
   assert(gamma_golden_move(g, 2, 5, 5));
   assert(!gamma_golden_possible(g, 2));
   assert(gamma_move(g, 2, 6, 6));
   assert(gamma_busy_fields(g, 1) == 4);
   assert(gamma_free_fields(g, 1) == 91);
   assert(gamma_busy_fields(g, 2) == 5);
   assert(gamma_free_fields(g, 2) == 13);
   assert(gamma_golden_move(g, 1, 3, 1));
   assert(gamma_busy_fields(g, 1) == 5);
   assert(gamma_free_fields(g, 1) == 8);
   assert(gamma_busy_fields(g, 2) == 4);
   assert(gamma_free_fields(g, 2) == 10);
   
   char *p = gamma_board(g);
   assert(p);
   assert(strcmp(p, board) == 0);
   free(p);
   
   gamma_delete(g);
   return 0;
}
