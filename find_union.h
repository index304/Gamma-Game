/** @file
 * Interfejs struktury find_union
 * zmodyfikowanej pod wykorzystanie w zadaniu gammaa.
 *
 * @author Jakub Bedełek
 */

#ifndef FIND_UNION_H
#define FIND_UNION_H

#include <stdint.h>
#include <stdbool.h>
   

/**
 * Struktura przechowująca dane potrzebne do wykonania algorytmu find_union.
 */
typedef struct find_s find_t; 


/** @brief Łączy obszary o reprezentantach @p number i @p val. 
 * @param[in] f       – wskaźnik na strukturę przechowującą dane,
 * @param[in] number    - numer pierwszego pola,
 * @param[in] val       - numer drugiego pola.
 @return Zwraca reprezentana pola o numerze @p numer.
 */
void funion(find_t *f, uint32_t number, uint32_t val);

/** @brief Inicjalizuje tablicę reprezentantów i tablicę rank. 
 * @param[in] f       - wskaźnik na strukturę przechowującą dane,
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia
 */
void init(find_t *f, uint32_t width, uint32_t height); 

/** @brief Wypełnia wartościami początkowymi tablicę reprezentantów i 
 * tablicę rank. 
 * @param[in] f       - wskaźnik na strukturę przechowującą dane,
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia
 */
void fill_find(find_t *f, uint32_t width, uint32_t height);

/** @brief Usuwa strukturę przechowującą dane.
 * @param[in] f       - wskaźnik na strukturę przechowującą dane,
 */
void delete_funion(find_t *f);

/** @brief Zmienia wielkość tablicy rank dla numeru @p number na @p val.
 * @param[in] f       - wskaźnik na strukturę przechowującą dane,
 * @param[in] number       - numer,
 * @param[in] val       - zmieniana wartośc. 
 */
void change_rank(find_t *f, uint32_t number, uint32_t val);

/** @brief Zmienia reprezentanta pola  o numerze @p number na @p val.
 * @param[in] f         - wskaźnik na strukturę przechowującą dane,
 * @param[in] number    - numer,
 * @param[in] val       - zmieniana wartość. 
 */
void change_rep(find_t *f, uint32_t number, uint32_t val);

/** @brief Sprawdza czy pamięć została przydzielona dla parametru @p f.
 * @param[in] f       - wskaźnik na strukturę przechowującą dane,
 * @return Zwraca jeden jeśli została przydzielona pamięć dla @p f lub 
 * 0 w przeciwnym przypadku. 
 */
bool check_alloc_find(find_t *f);

/** @brief Zwiększa wartość tablicy rank dla parametru @p rep_number o 1.
 * @param[in] f       - wskaźnik na strukturę przechowującą dane,    
 * @param[in] rep_number - index tablicy, w którym ma nastąpić zmiana.
 */
void increase_rank(find_t *f, uint32_t rep_number); 

/** @brief Zwraca reprezentana pola o numerze @p rep_number. 
 * @param[in] rep_number       - numer pola,
 * @param[in] f       – wskaźnik na strukturę przechowującą dane,
 @return Zwraca reprezentana pola o numerze @p rep_number.
 */
uint32_t find(uint32_t rep_number, find_t *f);


/** @brief Tworzy nową strukturę przechowującą dane.
 * @return Zwraca nową powstałą strukturę. 
 */
find_t *new_find_t();

/** @brief Usuwa strukturę przechowującą dane w przypadku błędnej alokacji.
 * @param[in] f       - wskaźnik na strukturę przechowującą dane,
 */
void delete_funion_all(find_t *f);

#endif /* FIND_UNION_H */
