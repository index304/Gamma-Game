/** @file
 * Implementacja interfejsu funkcji odpowiedzialnej za wczytywanie wejścia i
 * wywoływanie odpowiednich stanów gry. 
 *
 * @author Jakub Bedełek <jb417705@students.mimuw.edu.pl>
 * @copyright Jakub Bedełek
 * @date 10.05.2020
 */
 
 
/**
 * Dyrektywa preprocesoraa potrzeba do prawidłowego importu funkcji getline.
 */ 
#define _GNU_SOURCE

#include "batch.h"
#include "inter.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>


/**
 * Maksymalna liczba w jednej linii dla prawidłowego wejścia.
 */
#define MAX_SIZE 5

/**
 * Maksymalna liczba cyfr dla liczby typu uint32_t.
 */
#define UINT32_MAX_NUM_OF_DIGITS 10

/**
 * Numer pierwszego słowa w tablicy. 
 */
#define FIRST_WORD 0


/**
 * Struktura przechowywująca wszystkie dane potrzebne do prawidłowego działania
 * trybu wsadowego i prawidłowego przetwarzania wejścia użytkownika.
 */
typedef struct s_batch batch_t;

/**
 * Struktura przechowywująca wszystkie dane potrzebne do prawidłowego działania
 * trybu wsadowego i prawidłowego przetwarzania wejścia użytkownika.
 */
struct s_batch {
    int g_end;                   /**< Liczba wczytanych argumentów. */
    
    bool inter;                  /**< Zmienna o wartości true jeśli tryb 
                                      interaktywny został uruchomiony. */
    bool b_batch;                /**< Zmienna o wartości true jeśli tryb 
                                      wsadowy został uruchomiony. */
    bool show;                   /**< Zmienna o wartości true jeśli w danym 
                                      ruchu zostanie wypisana plansza. */
    bool changed_batch_mode;     /**< Zmienna o wartości true jeśli w danym ruchu
                                      miał być uruchomiony tryb wsadowy pomimo
                                      bycia już w trybie wsadowym. */
    bool bug;                    /**< Zmienna o wartości true jeśli w danym
                                      ruchu nie udała się inicjalizaja gammy. */
    char *words[MAX_SIZE];       /**< Tablica przechowywująca wczytane słowa. */
    
    gamma_t *g;                  /**< Struktura przechowywująca stan gry. */

    uint32_t LINE;               /**< Numer wypisywanej lini. */
    uint32_t liczby[MAX_SIZE + 1]; /**< Wczytane słowa zamienione na uint32_t. */
};


/**
 * Obiekt przechowywujący wszystkie dane potrzebne do prawidłowego działania
 * trybu wsadowego i prawidłowego przetwarzania wejścia użytkownika.
 */ 
static batch_t batch; 

/** @brief Sprawdza czy pamięć została przydzielona do @p ptr.
 * @param[in] ptr - wskaźnik na strukturę przechowywującą dane.
 */
static void memory_char(void *ptr) {
    if (ptr == NULL) {
        exit(EXIT_FAILURE);
    }
}

/** @brief Podaje mniejszą z dwóch liczb @p a i @p batch.
 * @param[in] a - pierwsza liczba,
 * @param[in] b - druga liczba,
 * @return Zwraca mniejszą z dwóch liczb @p a i @p batch. 
 */
static int min(int a, int b) {
    if (a <= b)
        return a; 
    else 
        return b; 
}

/** @brief Funkcja sprwadza, czy podana liczba w tablicy @p word składa się tylko
 * z cyfr od 0 do 9. 
 * @param[in] word  - tablica przechowywująca cyfry,
 * @param[in] length - długość tablicy przechowywującej cyfry,
 * @return Zwraca true jeśli tablicy @p word składa się tylko z cyfry od 0 do 9,
 * w przeciwnym przypadku zwraca false.
 */  
static bool ispositivenumber(char word[], int length) {
    for (int i = 0; i < length; i++) {
        if (isdigit(word[i]) == 0)
            return false;
    }    
    return true; 
}

/** @brief Zmienia przechowywanie cyfry z tablicy char[] na liczbę uint32_t.
 * Zmienione dane zapisuję w tablicy liczby.
 * @param[in] begin - numer indeksu, od którego będą sprawdzane cyfry,
 * @param[in] end   - numer indeksu, do którego będą sprawdzane cyfry.
 * @return Zwraca true jeśli przechowywane liczba w tablicy char[] jest 
 * prawidłową liczbą typu uint32_t lub false w przeciwnym przypadku.
 */
static bool change(int begin, int end) {
    for (int i = begin; i <= end; i++) {
        size_t length = strlen(batch.words[i]);
        
        if (length > UINT32_MAX_NUM_OF_DIGITS) {
            return false; 
        }
        
        // Brak akceptacji zer wiodących.
        if (length >= 2 && batch.words[i][0] == '0') {
            return false; 
        }
        
        if (!ispositivenumber(batch.words[i], length)) {
            return false; 
        }
        
        unsigned long number = strtoul(batch.words[i], NULL, 0);
        if (number > UINT32_MAX) {
            return false;
        }
        else {
            batch.liczby[i] = number;
        }
    }
    
    batch.g_end = end;
    return true; 
}

/** @brief Sprawdza czy odpowiednia komenda jest wywołana z właściwą liczbą
 * argumentów. 
 * @param[in] letter - komenda występująca na początku linii.
 * @return Zwraca true jeśli komenda jest prawidłowa lub false w przeciwnym
 * przypadku.
 */
static bool check_commands(char letter[]) {
    if (strlen(letter) == 0) {
        return false;
    }
    // Używam strcmp, ponieważ wyrażenie letter[0] == 'B' nie sprawdza długości
    // wyrazu letter, który może mieć więcej niż jeden znak.
    else if (strcmp(letter, "B") == 0 && batch.g_end == 4 && !batch.b_batch 
             && !batch.inter) {
        return true; 
    }
    else if (strcmp(letter, "I") == 0 && batch.g_end == 4 && !batch.b_batch 
             && !batch.inter) {
        return true;
    }
    else if (strcmp(letter, "m") == 0 && batch.g_end == 3 && batch.b_batch) {
        return true;
    }
    else if (strcmp(letter, "g") == 0 && batch.g_end == 3 && batch.b_batch) {
        return true;
    }
    else if (strcmp(letter, "b") == 0 && batch.g_end == 1 && batch.b_batch) {
        return true;
    }
    else if (strcmp(letter, "f") == 0 && batch.g_end == 1 && batch.b_batch) {
        return true;
    }
    else if (strcmp(letter, "q") == 0 && batch.g_end == 1 && batch.b_batch) {
        return true;
    }
    else if (strcmp(letter, "p") == 0 && batch.g_end == 0 && batch.b_batch) {
        return true;
    }
    else {
        return false;
    }
}

/** @brief Tworzy nowy tryb wsadowy lub interactywny.
 * @param[in] batch_mode - true jeśli tryb ma być wsadowy lub false jeśli nie.
 * @return Zwraca true jeśli udało się poprawnie stworzyć nowy tryb rozgrywki
 * lub false w przecinwym przypadku.
 */
static bool new_mode(bool batch_mode) {
    batch.g = gamma_new(batch.liczby[1], batch.liczby[2], batch.liczby[3],
                        batch.liczby[4]);
    if (batch.g == NULL) {
        batch.bug = true;
        return false;
    }
    else if (batch_mode) {
        batch.b_batch = true;
        batch.changed_batch_mode = true;
        return true;
    }
    else {
        batch.inter = true;
        return true;
    }
}

/** @brief Funkcja odpowiedzialna za uruchomienie wpisanej komendy.
 * @param[in] letter - komenda występująca na początku linii.
 * @return Zwraca wartość funkcji odpowiedniej dla danej komendy lub false
 * jeśli wywołanie komendy się nie powiodło. 
 */ 
static uint32_t start_commands(char letter[]) {
    if (strlen(letter) == 0)
        return false;
    // Używam strcmp, ponieważ wyrażenie letter[0] == 'B' nie sprawdza długości
    // wyrazu letter, który może mieć więcej niż jeden znak.
    else if (strcmp(letter, "B") == 0 && batch.g_end == 4 && !batch.b_batch 
             && !batch.inter) {
        return new_mode(true);
    }
    else if (strcmp(letter, "I") == 0 && batch.g_end == 4 && !batch.b_batch
             && !batch.inter) {
        return new_mode(false);
    }
    else if (strcmp(letter, "m") == 0 && batch.g_end == 3 && batch.b_batch) {
        return gamma_move(batch.g, batch.liczby[1], batch.liczby[2], 
                          batch.liczby[3]);
    }
    else if (strcmp(letter, "g") == 0 && batch.g_end == 3 && batch.b_batch) {
        return gamma_golden_move(batch.g, batch.liczby[1], batch.liczby[2],
                                 batch.liczby[3]);
    }
    else if (strcmp(letter, "b") == 0 && batch.g_end == 1 && batch.b_batch) {
        return gamma_busy_fields(batch.g, batch.liczby[1]);
    }
    else if (strcmp(letter, "f") == 0 && batch.g_end == 1 && batch.b_batch) {
        return gamma_free_fields(batch.g, batch.liczby[1]);
    }
    else if (strcmp(letter, "q") == 0 && batch.g_end == 1 && batch.b_batch) {
        return gamma_golden_possible(batch.g, batch.liczby[1]);
    }
    else if (strcmp(letter, "p") == 0 && batch.g_end == 0 && batch.b_batch) {
        batch.show = true;
        char *board = gamma_board(batch.g);
        if (board == NULL) {
            batch.bug = true;
            return false;
        }
        printf("%s", board); 
        free(board);  
        return true;
    }
    else {
        return false;
    }
}

/** @brief Zwalnia pamięc przechowywaną w tablicy words. 
 * @param[in] wordsNumber - liczba wczytanych słów.
 */
static void zwolnij(int wordsNumber) {
    for (int i = 0; i < min(wordsNumber, MAX_SIZE); i++) {
        free(batch.words[i]);
    }
}

/** @brief Wypisuję podsumowanie wczytanej linii na wyjście.
 * @param[in] error - true jeśli ma być wypisany błąd,
 * @param[in] score - wartość wypisywanej liczby.
 */
static void write_line(bool error, uint64_t score) {
    if (error) {
        fprintf(stderr, "ERROR %" PRIu32 "\n", batch.LINE);        
    }
    else if (batch.bug) {
        fprintf(stderr, "ERROR %" PRIu32 "\n", batch.LINE);
    }
    else if (batch.show) {
        batch.show = false;
    }
    else if (batch.changed_batch_mode) {
        batch.changed_batch_mode = false;
        printf("OK %" PRIu32 "\n", batch.LINE);
    }
    else {
        printf("%" PRIu64 "\n", score);
    }
}

/** @brief Funkcja przetwarza wczytane słowa z jednej linijki. 
 * @param[in] wordsNumber - liczba słów, 
 * @param[in] inputLine   - wczytana linijka.
 * @return Zwraca true jeśli linijka została poprawnie przetworzona lub 
 * false w przeciwnym przypadku.
 */
static bool use_words(int wordsNumber, char inputLine[]) {
    if (wordsNumber > MAX_SIZE || wordsNumber == 0) {
        zwolnij(wordsNumber);
        return false; 
    }
    else if (inputLine[0] != 0 && isspace(inputLine[0])) {
        zwolnij(wordsNumber);
        return false;
    }
    else {
      // Sprawdzanie czy można zamienić słowa z wejścia na liczby oraz 
      // czy podane komendy są poprawane.
        bool result = change(1, wordsNumber - 1);
        bool result2 = check_commands(batch.words[FIRST_WORD]);

        if (result && result2) {
            uint32_t score = start_commands(batch.words[FIRST_WORD]);
            write_line(false, score);
            
            zwolnij(wordsNumber);    
            return true; 
        }
    
        zwolnij(wordsNumber);
        return false;
    }
}

/** @brief Funckja opdowiedzialna za przetwarzanie wczytanego wejścia.
 * @param[in] readSize  - liczba znaków we wczytanej linijce,
 * @param[in] inputLine - wczytana linijka.
 * @return Zwraca true jeśli komenda jest prawidłowa lub false w przeciwnym
 * przypadku.
 */
static bool process_input(ssize_t readSize, char inputLine[]) {
    int wordsNumber = 0;
    int wordBeginPos = 0;    
    int prevWhiteSpace = 1;   
    
    // Przetwarzanie wejścia.
    for (int i = 0; i < readSize; i++) {
        if (!prevWhiteSpace && isspace(inputLine[i])) {
            if (wordsNumber == MAX_SIZE) {
                wordsNumber++;
                break;
            }
            batch.words[wordsNumber] = NULL;
            batch.words[wordsNumber] = malloc(i - wordBeginPos + 1);
            memory_char(batch.words[wordsNumber]);
            
            // Kopiowanie słowa to tablicy słów.
            strncpy(batch.words[wordsNumber], inputLine + wordBeginPos, i - wordBeginPos); 
            batch.words[wordsNumber][i - wordBeginPos] = '\0';
            
            wordsNumber++;
            prevWhiteSpace = 1;
            wordBeginPos = i; 
        } 
        else if (!isspace(inputLine[i])) {     
            if (prevWhiteSpace)
                wordBeginPos = i; 
            prevWhiteSpace = 0; 
        }
    }

    return use_words(wordsNumber, inputLine);
}

/** @brief Inicjalizuję początkowe wartości dla obiektu batch. 
 */
static void init_input() {
    batch.g_end = 1; 
    batch.inter = false; 
    batch.b_batch = false;

    batch.show = false; 
    batch.changed_batch_mode = false;
    batch.bug = false;
    batch.g = NULL;
    batch.LINE = 0;
}

void read_input() {
   init_input();
   // Losowa wartość potrzebna do funkcji getline.
   size_t N = 15;
   ssize_t readSize;
   char *inputLine = NULL; 
   int wyjscie = false;
   
   while (!batch.inter && (readSize = getline(&inputLine, &N, stdin)) 
                           != -1 && !wyjscie) {
      // readSize musi być liczbą dodatnią, więc można ją zrzutować na typ
      // bez znaku o większym zakresie.
      if ((size_t)readSize != strlen(inputLine)) {
         if (inputLine[0] == 0 && readSize == 1) {
            wyjscie = true;
         } 
         // Ustawiamy taki znak, aby wyjście było błędne.
         inputLine[0] = '!';
      }
     
      memory_char(inputLine);

      batch.LINE++;
      if (inputLine[0] != '#' && inputLine[0] != '\n' && !wyjscie) {
         if (process_input(readSize, inputLine)) {
            batch.bug = false;
            if (batch.inter) {
                start_interactive(batch.g, batch.liczby[1], batch.liczby[2],
                batch.liczby[3]);
            }
         }
         else {
            write_line(true, 0);
         }
      }
      
      free (inputLine); 
      inputLine = NULL;
   }
   
   if (inputLine != NULL) {
      free(inputLine);
   }
  
   if (batch.g != NULL)
      gamma_delete(batch.g);
}
