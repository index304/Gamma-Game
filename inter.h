/** @file
 * Interfejs funkcji odpowiedzialnej za start trybu interaktywnego.
 *
 * @author Jakub Bedełek <jb417705@students.mimuw.edu.pl>
 * @copyright Jakub Bedełek
 * @date 10.05.2020
 */
    
#ifndef INTER_H
#define INTER_H

#include "gamma.h"

/** @brief Funkcja odpowiedzialna za start trybu interaktywnego.
 * param[in] g        - struktura przechowywująca staan gry,
 * param[in] width    - szerokość planszy,
 * param[in] height   - wysokość planszy,
 * param[in] players  - liczba graczy.
 */ 
void start_interactive(gamma_t *g, uint32_t width,
                       uint32_t height, uint32_t players);

#endif
