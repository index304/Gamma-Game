/** @file
 * Implementacja interfejsu klasy przechowującej stan gry. 
 *
 * @author Jakub Bedełek <jb417705@students.mimuw.edu.pl>
 * @copyright Jakub Bedełek
 * @date 10.04.2020
 */
   
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "gamma.h"
#include <inttypes.h>
#include "find_union.h"

/**
 * Stała oznaczająca maksymalną liczbe sąsiadów dla dowolnego pola gry.
 */
#define NUM 4 

/**
 * Struktura przechowywują stan gry.
 */
typedef struct gamma gamma_t;

/**
 * Struktura przechowywująca tablice o rozmiarze liczba graczy. 
 */
typedef struct array_s array_t;

/**
 * Zmienna globalna wykorzystywana do przechodzenie graafu w głąb. 
 */
uint32_t counter = 1; 

/**
 * Struktura przechowywująca stan gry.
 */
struct gamma {
    uint32_t **board;            /**< Tablica przechowywująca stan planszy. */
    uint32_t width;              /**< Szerokość planszy. */
    uint32_t height;             /**< Wysokość planszy. */
    uint32_t areas;              /**< Maksymalna liczba obszarów. */
    uint32_t players;            /**< Liczba graczy. */
    uint32_t num_of_busy_fields; /**< Liczba zajętych pól */ 
    array_t *arrays;             /**< Tablice o rozmiarze liczba graczy. */
    find_t *find_union;          /**< Struktura przechowująca dane potrzebne do
                                     wykonania algorytmu find_union. */
    uint32_t *visited;           /**< Tablica sprawdzająca czy dane pole zostało
                                     już odwiedzone w danym przejściu dfs. */ 
};

/**
 * Struktura przechowywująca tablice o rozmiarze liczba graczy. 
 */
struct array_s {
    uint32_t neighbour_fields;  /**< Tablica przyporządkowująca każdemu graczowi
                                     liczbę wolnych pól sąsiadujących z polami
                                     zajętymi przez danego gracza. */ 
    uint32_t golden_move;       /**< Tablica zawierająca 1 jeśli diff wykonał
                                     złoty ruch lub 0 w przeciwnych przypadku. */
    uint32_t num_of_areas;      /**< Tablica przechowywująca liczbę obszarów zajętych
                                     przez każdego gracza. */
    uint32_t num_of_fields;     /**< Tablica przechowywująca liczbę pól zajętych
                                     przez każdego graczaa. */
};

void gamma_delete(gamma_t *g) {
    if (g != NULL) {
        // Usuwamy całą zaalokowaną pamieć. 
        for (uint32_t i = 0; i < g->height; i++) {
            free(g->board[i]); 
        }
        free(g->board);
        free(g->arrays);
        free(g->visited);
        delete_funion(g->find_union); 
        free(g->find_union);
        free(g); 
    }
}

/** @brief Sprawdza, czy pamięć została przydzielona do @p ptr. 
 * @param[in] ptr       – wskaźnik na strukturę przechowującą dane.
 * @return Zwraca 1 jeśli pamięć została przydzielona do wskaźnika @p ptr
 * lub 0 w przeciwnym przypadku.
 */
static bool check_alloc(void *ptr) {
   return (ptr == NULL);
}

/** @brief Inicjalizuje tablicę przechowującą ruchy graczy.
 * @param[in] g       – wskaźnik na strukturę przechowującą dane.
 */
static void init_board(gamma_t *g) {
   g->board = (uint32_t **)calloc(g->height, sizeof(uint32_t *));
   if (g->board == NULL)
      return;
   
   for (uint32_t i = 0; i < g->height; i++) {
      g->board[i] = (uint32_t *)calloc(g->width, sizeof(uint32_t));
   }
   
   // Wypełniamy board zerami, co oznacza, że pola są wolne.
   for (uint32_t i = 0; i < g->height; i++) {
      for (uint32_t j = 0; j < g->width; j++) {
         g->board[i][j] = 0;  
      }
   }
}

/** @brief Inicjalizuje liczby przechowywane w strukturze @ref gamma.
 * @param[in] g       – wskaźnik na strukturę przechowującą dane.
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia,
 * @param[in] players – liczba graczy, liczba dodatnia,
 * @param[in] areas   – maksymalna liczba obszarów,
 *                      jakie może zająć jeden diff.
 */
static void init_numbers(gamma_t *g, uint32_t width, uint32_t height,
                         uint32_t players, uint32_t areas) {
   g->width = width; 
   g->height = height; 
   g->players = players;
   g->areas = areas; 
   g->num_of_busy_fields = 0;
}

/** @brief Inicjalizuje jendnowymiarowe tablice
 *  przechowywane w strukturze @ref gamma.
 * @param[in] g       – wskaźnik na strukturę przechowującą dane.
 */
static void init_arrays(gamma_t *g) {
   uint32_t players = g->players + 1;  
   
   g->arrays = calloc(players, sizeof(array_t));
   g->visited = calloc((uint64_t)g->width * (uint64_t)g->height, sizeof(uint32_t));
}

/** @brief Nadaje początkowe wartości nowo zdefiniowanym obiektom. 
 * @param[in] g       – wskaźnik na strukturę przechowującą dane.
 */
static void fill_arrays(gamma_t *g) {
   // Nadajemy początkowe wartości w tablicach jako 0. 
   for (uint32_t i = 0; i <= g->players; i++) {
      g->arrays[i].golden_move = g->arrays[i].neighbour_fields = 0;
      g->arrays[i].num_of_areas = g->arrays[i].num_of_fields = 0; 
   }
   
   // Na początku każdemu polu przydzielamy siebie jako reprezentanta.
   // Każde pole składa się z obszaru zajmującego tylko to pole, czyli 
   // ma wartośc 1. 
   for (uint64_t i = 0; i < (uint64_t)g->width * (uint64_t)g->height; i++) {
      g->visited[i] = 0; 
   }
   
   fill_find(g->find_union, g->width, g->height);
}

/** @brief Usuwa g w przypadku złej alokacji.
 * @param[in] g - strukturap przechowująca stan gry.
 */
static void gamma_delete_all(gamma_t *g) {
    if (g != NULL) {
      // Usuwamy całą zaalokowaną pamieć. 
        if (g->board != NULL) {
           for (uint32_t i = 0; i < g->height; i++) {
              if (g->board[i] != NULL)
                 free(g->board[i]); 
           }
        }
        if (g->board != NULL)
           free(g->board);

        if (g->arrays != NULL) 
           free(g->arrays);
        if (g->visited != NULL) 
           free(g->visited);
        if (g->find_union != NULL)
           delete_funion_all(g->find_union); 
        if (g->find_union != NULL)
           free(g->find_union);
        free(g); 
    }
}

gamma_t* gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas) {
   // Sprawdzamy, czy parametry zostały wprowadzone prawidłowo. 
   if (width <= 0 || height <= 0 || players <= 0 || areas <= 0)
      return NULL; 
   // Alokujemy pamięć.
   gamma_t *g = malloc(sizeof(gamma_t));
   
   // Sprawdzamy, czy pamięć została zaalokowana
   if (g == NULL)
      return NULL;
      
      
   g->find_union = new_find_t();
   if (g->find_union == NULL) {
      free(g);
      return NULL;
   }
   
   // W przypadku niepowodzenia alokacji w tej funkcji funkcja 
   // check_alloc_find zwróc wartość FALSE.
   init(g->find_union, width, height);
   init_numbers(g, width, height, players, areas); 
   init_board(g); 
   init_arrays(g); 
   
   
   if (check_alloc_find(g->find_union) || g->visited == NULL || g->board == NULL
       || g->arrays == NULL) {
      delete_funion_all(g->find_union);
      gamma_delete_all(g);
      return false;
   }
   
   
   for (uint32_t i = 0; i < g->height; i++) {
      if (g->board[i] == NULL) {
        delete_funion_all(g->find_union);
        gamma_delete_all(g);
        return false;
      }
   }
   
   // Wypełniamy wszystkie tablicę domyślnymi wartościami początkowymi.
   fill_arrays(g);
   
   return g;
}

/** @brief Sprawdza, czy parametry @p x i @p y są prawidłowe. 
 * @param[in] g       – wskaźnik na strukturę przechowującą dane,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return zwraca 1 jeśli współrzędne są prawidłowe lub 0 w przeciwnym przypadku.
 */
static bool check_x_y(gamma_t *g, uint32_t x, uint32_t y) {
    return (x < g->width && y < g->height);
}

/** @brief Sprawdza, czy parametr @p player jest prawidłowy. 
 * @param[in] g       – wskaźnik na strukturę przechowującą dane,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @return zwraca 1 jeśli numer gracza jest prawidłowy lub 0 w przeciwnym 
 * przypadku.
 */
static bool check_player(gamma_t *g, uint32_t player ) {
    return (0 < player && player <= g->players);
}

/** @brief Sprawdza, czy parametr pole o współrzędnych @p x i @p y sąsiaduje
 * z polami zajętymi przez gracza @p player.  
 * @param[in] g       – wskaźnik na strukturę przechowującą dane,
 * @param[in] player – liczba graczy, liczba dodatnia,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return zwraca 1 pole o współrzędnych @p x i @p y sąsiaduje
 * z polami zajętymi przez gracza @p player.  
 */
static bool have_neighbour(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
   // Sprawdzamy poprawność podanych współrzędnych. 
   if (!check_x_y(g, x, y))
      return false;
   
   uint32_t n = g->width; 
   uint32_t m = g->height;
   
   // Sprawdzamy, czy pole o wsp. x i y sąsiaduje z polem gracza player.
   if ((x + 1 <= n - 1) && g->board[y][x+1] == player)
      return true; 
   else if (x > 0 && g->board[y][x - 1] == player)
      return true; 
   else if (y > 0 && g->board[y - 1][x] == player)
      return true; 
   else if ((y + 1 <= m - 1) && g->board[y + 1][x] == player)
      return true; 
   else 
      return false; 
}

/** @brief Zwraca numer pola o współrzędnych @p x i @p y. 
 * @param[in] g       – wskaźnik na strukturę przechowującą dane,
 * @param[in] x       – numer kolumny,
 * @param[in] y       - numer wiersza.
 @return Zwraca numer pola o współrzędnych @p x i @p y. 
 */
static uint64_t numer(gamma_t *g, uint32_t x, uint32_t y) {
   return (uint64_t)(g->width) * (uint64_t)y + (uint64_t)x; 
}


/** @brief Łączy obszar zawierający pole @p x i @p y z obszarem
 * zawierającym pole @p x2 i @p y2. 
 * @param[in] g       – wskaźnik na strukturę przechowującą dane,
 * @param[in] x       - pierwsza współrzedna pierwszego pola,
 * @param[in] y       - druga współrzedna pierwszego pola,
 * @param[in] x2      - pierwsza współrzedna drugiego pola,
 * @param[in] y2       - pierwsza współrzedna drugiego pola.
 @return Zwraca 1 jeśli można połączyć obszary lub zero jeśli nie można
 * albo są już połączone. 
 */
static bool check_link(gamma_t *g, uint32_t x, uint32_t y,
                   uint32_t x2, uint32_t y2) {
   // Sprawdzamy, czy pola należą do tego samego gracza oraz czy nie były
   // już połączone.
   return (g->board[y][x] == g->board[y2][x2] &&
           find(numer(g, x, y), g->find_union) != find(numer(g, x2, y2),
                      g->find_union));
}

/** @brief Łączy jeśli to możliwe obszar zawierający pole @p x i @p y 
 * z sąsiednimi polami.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new. 
 */
static void link_areas(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
   uint32_t n = g->width; 
   uint32_t m = g->height;
   
   // Jeśli to możliwe łączymy pole o wsp. x i y z sąsiednimi polami. 
   // Jest to możliwe jeśli pola nie były połączone i należą do tego
   // samego gracza. Następnie aktualizujemy liczbe obszarów. 
   if ((x + 1 <= n - 1) && check_link(g, x, y, x + 1, y)) {
      g->arrays[player].num_of_areas--; 
      funion(g->find_union, numer(g, x, y), numer(g, x + 1, y));
   }
   if (x > 0 && check_link(g, x, y, x - 1, y)) {
      g->arrays[player].num_of_areas--; 
      funion(g->find_union, numer(g, x, y), numer(g, x - 1, y));
   } 
   if (y > 0 && check_link(g, x, y, x, y - 1)) {
      g->arrays[player].num_of_areas--; 
      funion(g->find_union, numer(g, x, y), numer(g, x, y - 1));
   }
   if ((y + 1 <= m - 1) && check_link(g, x, y, x, y + 1)) {
      g->arrays[player].num_of_areas--; 
      funion(g->find_union, numer(g, x, y), numer(g, x, y + 1));
   }
   
}

/** @brief Liczy ilość pól sąsiadujących z polem o współrzędnych @p x i @p y 
 * należących do gracza @p player. 
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new. 
 * @return Zwraca ilość pól sąsiadujących z polem o współrzędnych @p x i @p y 
 * należących do gracza @p player. 
 */
static uint32_t num_neighbour_fields(gamma_t *g, uint32_t player, uint32_t x,
                            uint32_t y) {
   // Sprawdzamy, czy współrzędne są prawidłowe i czy pole o wsp. x i y 
   // nie jest wolne.
   if (!check_x_y(g, x, y) || g->board[y][x] != 0)
      return 0;
      
   uint32_t count = 0; 
   uint32_t n = g->width, m = g->height; 

   // Zwiększamy wartość licznika count jeśli pole x i y sąsiaduje z polem
   // należącym do gracza player. 
   if ((x + 1 <= n - 1) && g->board[y][x + 1] == player)
      count++; 
   if (x > 0 && g->board[y][x - 1] == player)
      count++; 
   if (y > 0 && g->board[y - 1][x] == player)
      count++;
   if ((y + 1 <= m - 1) && g->board[y + 1][x] == player)
      count++; 
      
   return count; 
}

/** @brief Sprawdza, czy istnieją wolne pola sąsiadujące z polem o
 * współrzędnych @p x i @p y i należacym do gracza @p player i nie sąsiadujące
 * z żadnym inny polem gracza @p player. Jeśli takie istnieją to zwiększamy
 * wartość tablicy neighbourf_fields dla gracza @p player.  
 * należy do gracza @p player. 
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new. 
 */
static void increase_neighbour_fields(gamma_t *g, uint32_t player,
                                      uint32_t x, uint32_t y) {
   // Sprawdzamy, czy współrzędne są prawidłowe. 
   if (!check_x_y(g, x, y))
      return;
   
   // Aktualizujemy liczbę wolnych pól sąsiadujących z polem x i y, po wykonaniu
   // nowego ruchu.  
   if (num_neighbour_fields(g, player, x + 1, y) == 1)
      g->arrays[player].neighbour_fields++; 
   if (num_neighbour_fields(g, player, x - 1, y) == 1)
      g->arrays[player].neighbour_fields++;
   if (num_neighbour_fields(g, player, x, y + 1) == 1)
      g->arrays[player].neighbour_fields++;
   if (num_neighbour_fields(g, player, x, y - 1) == 1)
      g->arrays[player].neighbour_fields++;
}

/** @brief Liczy ilość pól sąsiadujących z polem o współrzędnych @p x i @p y 
 * należących do gracza @p player. 
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new. 
 * @return Zwraca ilość pól sąsiadujących z polem o współrzędnych @p x i @p y 
 * należących do gracza @p player albo -1 jeśli parametry funkcji są niepoprawne
 * lub, pole o współrzędnych @p x i @p y nie jest wolne. 
 */
static int32_t golden_neighbour_fields(gamma_t *g, uint32_t player, uint32_t x,
                                       uint32_t y) {
   // Sprawdzamy poprawność podanych współrzędnych oraz czy pole o wsp. y i x
   // nie jest wolne. 
   if (!check_x_y(g, x, y) || g->board[y][x] != 0)
      return -1;
   
   int32_t count = 0; 
   uint32_t n = g->width, m = g->height; 
      
   // Zwiększamy wartość licznika count jeśli pole x i y sąsiaduje z polem
   // należącym do gracza player. 
   if ((x + 1 <= n - 1) && g->board[y][x + 1] == player)
      count++; 
   if (x > 0 && g->board[y][x - 1] == player)
      count++; 
   if (y > 0 && g->board[y - 1][x] == player)
      count++;
   if ((y + 1 <= m - 1) && g->board[y + 1][x] == player)
      count++; 
   
   return count; 
}

/** @brief Funkcja zmniejsza ilość wolnych pól sąsiadujących z obszarami
 * zajętymi przez gracza @p player, po wykonaniu złotego ruchu i 
 * zmianie przez pola o współrzędnych @p x i @p y właściciela. 
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new. 
 */
static void golden_decrease(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
   // Sprawdzamy poprawność podanych współrzędnych. 
   if (!check_x_y(g, x, y))
      return;
   
   // Odejmujemy liczbę wolnych sąsiadujących pól z obszarem zajmowanym
   // przez gracza player, jeśli golden move zmienił wystarczająco strukturę
   // planszy. 
   if (golden_neighbour_fields(g, player, x + 1, y) == 0)
      g->arrays[player].neighbour_fields--; 
   if (golden_neighbour_fields(g, player, x - 1, y) == 0)
      g->arrays[player].neighbour_fields--;
   if (golden_neighbour_fields(g, player, x, y + 1) == 0)
      g->arrays[player].neighbour_fields--;
   if (golden_neighbour_fields(g, player, x, y - 1) == 0)
      g->arrays[player].neighbour_fields--;
}

/** @brief Funkcja sprawdzająca czy wartość parametru @p searched jest różna
 * od czterech kolejnych parametrów.
 * @param[in] searched  - numer szukanej liczby,
 * @param[in] player1  – numer gracza nr. 1, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] player2  – numer gracza nr. 2, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] player3  – numer gracza nr. 3, liczba dodatnia niewiększa o wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] player4  – numer gracza nr. 4, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Zwraca 1 jeśli wartość parametru @p searched jest rożna od czterech
 * kolejnych parametrów.  
 */
static bool diff(uint32_t searched, uint32_t player1, uint32_t player2,
              uint32_t player3, uint32_t player4) {
   return (searched != player1 && searched != player2 &&
           searched != player3 && searched != player4);
}

/** @brief Po zajęciu pola o współrzędnych @p x i @p y przez, któregoś z 
 * graczy funkcja aktualizuje (zmniejsza) liczbę wolnych pól sąsiadujących
 * z zajętym polem. 
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new. 
 */
static void neighbour_decrease(gamma_t *g, uint32_t x, uint32_t y) {
   // Sprawdzamy poprawność współrzędnych. 
   if (!check_x_y(g, x, y))
      return;
   
   uint32_t n = g->width; 
   uint32_t m = g->height;
   // Zmienne będą służyły do zapamiętania numerów graczy z sąsiednich pól
   // pola o wsp. x i y. 
   uint32_t player1, player2, player3; 
   player1 = player2 = player3 = 0; 
      
   // Pole x i y zostało przekształcone na pole gracza player z wolnego pola.
   // Dlatego musimy zaaktualizować liczbę wolnych pól sąsiadujących dla
   // graczy będących właścicielami pól sąsiadujących. 
   if ((x + 1 <= n - 1) &&
        diff(g->board[y][x + 1], 0, player1, player2, player3)) {
      player1 = g->board[y][x + 1]; 
      g->arrays[player1].neighbour_fields--;
   }
   if (x > 0 && diff(g->board[y][x - 1], 0, player1, player2, player3)) {
      player2 = g->board[y][x - 1]; 
      g->arrays[player2].neighbour_fields--;
   } 
   if (y > 0 && diff(g->board[y - 1][x], 0, player1, player2, player3)) {
      player3 = g->board[y - 1][x]; 
      g->arrays[player3].neighbour_fields--;;
   }
   if ((y + 1 <= m - 1) &&
        diff(g->board[y + 1][x], 0, player1, player2, player3)) {
      g->arrays[g->board[y + 1][x]].neighbour_fields--;
   }
}

bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
   if (g == NULL)
      return false;
   // Sprawdzamy poprawność parametrów i czy pole x i y nie jest wolne. 
   if (check_x_y(g, x, y) && check_player(g, player) && g->board[y][x] == 0) {
   // Sprawdzamy, czy diff nie przekroczy maksymalne ilości pól po zajęciu pola.
      if (g->arrays[player].num_of_areas == g->areas 
          && !have_neighbour(g, player, x, y))
         return false;
      
      // Ruch może zostać wykonany więc aktualizujemy plansze. 
      g->arrays[player].num_of_fields++;
      g->num_of_busy_fields++;
      g->board[y][x] = player;
      
      // Dodajemy graczowi player nowe wolne pola sąsiadujące z polem x i y. 
      increase_neighbour_fields(g, player, x, y); 
      // Odejmujemy graczom będącym właścicielami pól sąsiadujących z x i y
      // liczbę wolnych pól sąsiadujących. 
      neighbour_decrease(g, x, y); 
      
      // Zwiększamy liczbę zajętych obszarów przez gracza player
      // lub łączymy w większy obszar i odejmujemy. 
      if (!have_neighbour(g, player, x, y)) {
         g->arrays[player].num_of_areas++;
      }
      else {
         g->arrays[player].num_of_areas++;
         link_areas(g, player, x, y); 
      }
      
      // Ruch został wykonany.
      return true; 
   }
   else // Ruch nie został wykonany. 
      return false; 
}


/** @brief Sprawdza, czy gracz może wykonać złoty ruch.
 * Sprawdza, czy gracz @p player jeszcze nie wykonał w tej rozgrywce złotego
 * ruchu i jest przynajmniej jedno pole zajęte przez innego gracza.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu i jest przynajmniej jedno pole zajęte przez innnego gracza. 
 * Zwraca false w przeciwnym przypadku.
 */
static bool gamma_golden_possible_old(gamma_t *g, uint32_t player) {
    if (g == NULL)
      return false;
    // Sprawdzamy, czy golden move dla gracza player jest możliwy. 
    return (check_player(g, player) && g->arrays[player].golden_move == 0 
            && g->num_of_busy_fields - g->arrays[player].num_of_fields > 0);
 }
 

/** @brief Przechodzimy algorytmem dfs po spójnym fragmencie planszy należącym
 * do gracza @p player.  
 * Funkcja przechodzi algorytmem dfs po spójnym fragmencie planszy należaym 
 * do gracza player, jednocześnie aktualizując odwiedzone pola.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new. 
 * @param[in] rep_number - numer gracza, który jest reprezentantem pola, po którym
 *                   aktualnie przechodzimy algorytmem dfs.
 */
static void dfs(gamma_t *g, uint32_t player, uint32_t x, uint32_t y,
                uint32_t rep_number) {
   // Zaznaczamy, że wierzchołek został odwiedzony. 
   g->visited[numer(g, x, y)] = counter; 
   // Ustawiamy nowego reprezentanta.
   change_rep(g->find_union, numer(g, x, y), rep_number); 
   // Zwiększamy wielkość spójnego obszaru.
   increase_rank(g->find_union, rep_number); 
   
   // Jeśli nie odwiedziliśmy sąsiedniego wierzchołka oraz sąsiednie pole jest
   // prawidłowe, to uruchamiamy kolejne przejście dfs. 
   if (check_x_y(g, x - 1, y) && g->visited[numer(g, x-1, y)] != counter &&
       g->board[y][x - 1] == player) { 
      dfs(g, player, x - 1, y, rep_number); 
   }
   if (check_x_y(g, x, y - 1) && g->visited[numer(g, x, y - 1)] != counter &&
       g->board[y - 1][x] == player) {
      dfs(g, player, x, y - 1, rep_number); 
   }
   if (check_x_y(g, x + 1, y) && g->visited[numer(g, x+1, y)] != counter &&
       g->board[y][x + 1] == player) {
      dfs(g, player, x + 1, y, rep_number);    
   }
   if (check_x_y(g, x, y + 1) && g->visited[numer(g, x, y + 1)] != counter &&
       g->board[y + 1][x] == player) {
      dfs(g, player, x, y + 1, rep_number);
   }
   
}

/** @brief Funkcja sprawdzająca czy wartość parametru @p a jest różna
 * od czterech kolejnych parametrów.
 * @param[in] a   - liczba porównywana z innymi,
 * @param[in] l1  – liczba nr. 1,
 * @param[in] l2  – liczba nr. 2,
 * @param[in] l3  – liczba nr. 3,
 * @param[in] l4  – liczba nr. 4.
 * @return Zwraca 1 jeśli wartość parametru @p a jest rożna od czterech
 * kolejnych parametrów.  
 */
static bool different_num(uint32_t a, uint32_t l1, uint32_t l2, uint32_t l3,
                  uint32_t l4) {
   return (a != l1 && a != l2 && a != l3 && a != l4);
}


/** @brief Funkcja rozpoczyna w danym polu algorytm dfs. 
 * Jeśli pole o wsp. @p x i @p y jest zajmowane przez gracza @p player2, to
 * rozpoczynamy w tym polu algorytm dfs, po wszystkich spójych polach należaych
 * do gracza @p player. Przyjmujemy, że pole, w którym zaczeliśmy staje się 
 * reprezentantem wszystkich odwiedzonych pól. 
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player2  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new. 
 * @return Zwracama 1 jeśli pole o wsp. @p x i @p y należy do gracza @p player2
 * lub 0 w przeciwnym przypadku.
 */
static bool start_dfs(gamma_t *g, uint32_t player2, uint32_t x, uint32_t y) {
   if (g->board[y][x] == player2) {  
      counter++; 
      
      uint64_t num = numer(g, x, y);
      change_rep(g->find_union, num, num);
      change_rank(g->find_union, num, 1);
      dfs(g, player2, x, y, num); 
      return true; 
   }
   else {
      return false;
   }
}

/** @brief  Przywraca plansze do pierwotnego stanu po tym jak na polu @p x i @p y
 * nie mógł zostać wykonany złoty ruch. 
 * Przywraca pierwotny stan planszy po rozłączeniu obszarów powstałych przez
 * zmiane właściciela pola o wsp. @p x i @p y. Zmienia także właściciela tego
 * pola na właściciela sprzed złotego ruchu. Aktualizuje liczbę obszarów
 * powstałych po złączeniu pól gracza @p player z polem @p x i @p y. 
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player2  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new,
 * @param[in] new_areas - ilość nowopowstałych obszarów, po wyłączeniu pola 
 *                   @p x i @p y z pól gracza nr. @p player. 
 */
static void golden_move_failed(gamma_t *g, uint32_t player2, uint32_t x,
                               uint32_t y, uint32_t new_areas) {
   g->board[y][x] = player2; 
   change_rank(g->find_union, numer(g, x, y), 1);
   change_rep(g->find_union, numer(g, x, y), numer(g, x, y));
   link_areas(g, player2, x, y);
   g->arrays[player2].num_of_areas += new_areas;
}

/** @brief Funkcja zwracająca true jeśli gracz @p player może wykonać złotych 
 * ruch na polu o numerze @p x i @p y.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza wykonującego złoty ruch, liczba dodatnia
 *                      niewiększa od wartości @p players z funkcji 
 *                      @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Zwraca true jeśli na polu @p x i @p y gracz @p player może wykonać
 *         złoty ruch.
 */
static bool gamma_golden_possible_x_y(gamma_t *g, uint32_t player,
                                      uint32_t x, uint32_t y) {
   if (check_player(g, player) && check_x_y (g, x, y) && 
       gamma_golden_possible_old(g, player)) {
      // Sprawdzamy, czy pole nie należy do graczy i czy nie jest wolne.
      if (g->board[y][x] == player || g->board[y][x] == 0)
         return false;
      // Ustawiam player2 jako poprzedniego właściciela pola x i y. 
      uint32_t player2 = g->board[y][x];
      // c1, c2, c3, c4 - numery globalnego licznika dla nowych wywołań dfs.
      // new_areas - ilość nowych obszarów gracza player2 powstałych po
      // zmianie właściciela pola o wsp. x i y. 
      uint32_t c1 = 1, c2 = 1, c3 = 1, c4 = 1, new_areas = 0; 
      g->board[y][x] = 0; 
      
      if (check_x_y(g, x - 1, y) && start_dfs(g, player2, x - 1, y)) {
         new_areas++; 
         c1 = counter; 
      }
      if (check_x_y(g, x, y - 1) 
          && different_num(g->visited[numer(g, x, y - 1)], c1, c2, c3, c4)
          && start_dfs(g, player2, x, y - 1)) {
         new_areas++;
         c2 = counter; 
      }
      if (check_x_y(g, x + 1, y) 
          && different_num(g->visited[numer(g, x + 1, y)], c1, c2, c3, c4)
          && start_dfs(g, player2, x + 1, y)) {
         new_areas++;
         c3 = counter; 
      }
      if (check_x_y(g, x, y+1) 
          && different_num(g->visited[numer(g, x, y+1)], c1, c2, c3, c4) 
          && start_dfs(g, player2, x, y + 1)) {
         new_areas++;
      }
      // Sprawdzamy, czy golden move może być wykonany i przywracamy
      // poprzedni stan planszy. 
      if (g->arrays[player2].num_of_areas + new_areas - 1 > g->areas ||
         (!have_neighbour(g, player, x, y) && 
          g->arrays[player].num_of_areas + 1 > g->areas)) {
         golden_move_failed(g, player2, x, y, new_areas);
         return false; 
      }
      else {
         golden_move_failed(g, player2, x, y, new_areas);
         return true;
      }
   }
   else {
      return false;
   }
}
 
bool gamma_golden_possible(gamma_t *g, uint32_t player) {
      if (g == NULL)
      return false;
   // Sprawdzamy, czy golden move dla gracza player jest możliwy. 
    if (check_player(g, player) && g->arrays[player].golden_move == 0 
            && g->num_of_busy_fields - g->arrays[player].num_of_fields > 0) {
      for (uint32_t i = 0; i < g->height; i++) {
         for (uint32_t j = 0; j < g->width; j++) {
            if (gamma_golden_possible_x_y(g, player, j, i)) {
               return true;
            }
         }
      }
      return false;
   }
   else {
      return false;
   }
}

/** @brief  Aktualizuje plansze gdy diff @p player może wykonać złoty ruch
 * na polu @p x i @p y. 
 * Zmienia stan planszy po wykonaniu przez gracza @p player złotego ruch na 
 * polu @p x i @p y. Aktualizuje stan planszy dla gracza @p player2. 
 * Zaznacza wykonanie złotego ruchu przez gracza @p player. 
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player2  – numer gracza wykonującego, który był właścicielem pola
 *                       @p x i @p y przed złotym ruchem, liczba dodatnia
 *                       niewiększa od wartości @p players z funkcji 
 *                       @ref gamma_new,
 * @param[in] player  – numer gracza wykonującego złoty ruch, liczba dodatnia
 *                      niewiększa od wartości @p players z funkcji 
 *                      @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new,
 * @param[in] new_areas - ilość nowopowstałych obszarów, po wyłączeniu pola 
 *                        @p x i @p y z pól gracza nr. @p player2. 
 */
static void golden_move_ok(gamma_t *g, uint32_t player2, uint32_t player, 
                uint32_t x, uint32_t y, uint32_t new_areas) {
   // Odejmujemy liczbę wolnych pól sąsiadujących z polem o wsp. x i y
   // dla gracza, którego pole zostało zajęte przez golden move. 
   golden_decrease(g, player2, x, y);
   // Zwiększamy liczbę pól zajętych przez gracza wykonującego golden
   // move i zmniejszamy dla poprzedniego właściciela pola x i y.
   g->arrays[player].num_of_fields++;
   g->arrays[player2].num_of_fields--; 
   
   // Aktualizujemy liczbę obszarów dla gracza player2. 
   if (!have_neighbour(g, player2, x, y))
      g->arrays[player2].num_of_areas--;
   else 
      g->arrays[player2].num_of_areas += (new_areas - 1);
   
   // Aktualizujemy plansze, numer reprezentanta oraz wielkość 
   // poszczególnych obszarów. 
   g->board[y][x] = player;
   change_rank(g->find_union, numer(g, x, y), 1);
   change_rep(g->find_union, numer(g, x, y), numer(g, x, y));
   
   // Dodajemy liczbę nowych wolnych pól dla gracza x i y sąsiadujących z x i y.
   increase_neighbour_fields(g, player, x, y);
   
   // Aktualizujemy liczbę obszarów dla gracza player. 
   if (!have_neighbour(g, player, x, y)) {
      g->arrays[player].num_of_areas++;
   }
   else {
      g->arrays[player].num_of_areas++;
      link_areas(g, player, x, y); 
   }
   
   g->arrays[player].golden_move = 1;                            
}

bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
   if (g == NULL)
      return false;
   // Sprawdzamy, czy złoty ruch jest możliwy i czy parametry są poprawne.
   if (check_player(g, player) && check_x_y (g, x, y) && 
       gamma_golden_possible_old(g, player)) {
      // Sprawdzamy, czy pole nie należy do graczy i czy nie jest wolne.
      if (g->board[y][x] == player || g->board[y][x] == 0)
         return false;
      
      // Ustawiam player2 jako poprzedniego właściciela pola x i y. 
      uint32_t player2 = g->board[y][x];
      // c1, c2, c3, c4 - numery globalnego licznika dla nowych wywołań dfs.
      // new_areas - ilość nowych obszarów gracza player2 powstałych po
      // zmianie właściciela pola o wsp. x i y. 
      uint32_t c1 = 1, c2 = 1, c3 = 1, c4 = 1, new_areas = 0; 
      g->board[y][x] = 0; 
      
      if (check_x_y(g, x - 1, y) && start_dfs(g, player2, x - 1, y)) {
         new_areas++; 
         c1 = counter; 
      }
      if (check_x_y(g, x, y - 1) 
          && different_num(g->visited[numer(g, x, y - 1)], c1, c2, c3, c4)
          && start_dfs(g, player2, x, y - 1)) {
         new_areas++;
         c2 = counter; 
      }
      if (check_x_y(g, x + 1, y) 
          && different_num(g->visited[numer(g, x + 1, y)], c1, c2, c3, c4)
          && start_dfs(g, player2, x + 1, y)) {
         new_areas++;
         c3 = counter; 
      }
      if (check_x_y(g, x, y+1) 
          && different_num(g->visited[numer(g, x, y+1)], c1, c2, c3, c4) 
          && start_dfs(g, player2, x, y + 1)) {
          new_areas++;
      }
      // Sprawdzamy, czy golden move może być wykonany, jeśli nie to przywracamy
      // poprzedni stan planszy. 
      if (g->arrays[player2].num_of_areas + new_areas - 1 > g->areas ||
         (!have_neighbour(g, player, x, y) && 
          g->arrays[player].num_of_areas + 1 > g->areas)) {
            golden_move_failed(g, player2, x, y, new_areas);
            return false; 
      }
      
      golden_move_ok(g, player2, player, x, y, new_areas);
      return true; 
   }
   return false;
}

uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
   if (g == NULL)
      return false;
   if (check_player(g, player))
       return g->arrays[player].num_of_fields;
   else 
       return 0;
}

uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
   if (g == NULL || player == 0 || player > g->players)
      return false;
   if (check_player(g, player) && g->arrays[player].num_of_areas == g->areas)
      return g->arrays[player].neighbour_fields;
   else
      return (uint64_t)(g->height) * (uint64_t)(g->width) 
              - (uint64_t)g->num_of_busy_fields;
}

char* gamma_board(gamma_t *g) {
   if (g == NULL)
      return false;
    // Tablica, w której będą zamieniane liczby na znaki. 
   char digits[UINT32_MAX_NUM_OF_DIGITS];
   // Zmienna przechowywująca długość wszystkich cyfr plus ilość znkaów
   // nowej lini, będzie ona służyła jaka rozmiar zwracanej tablicy. 
   long sum = 0; 
    
   for (uint32_t i = 0; i < g->height; i++) {
      for (uint32_t j = 0; j < g->width; j++) {
         // Kopiujemy liczbę do tablicy znaków. 
         int len = sprintf(digits, "%" PRIu32, g->board[i][j]); 
         // Aktualizujemy wielkość zwracanej tablicy. 
         sum += len;
         if (len > 1)
            sum += 2;
      }      
   }
   sum += g->height; 
   
   char *char_board = (char *)calloc(sum + 1, sizeof(char));
   if (check_alloc(char_board))
      return NULL;
   // Zmienna pomocnicza potrzebna do kopiowania liczb znak 
   // po znaku do zwracanej tablicy znaków.
   uint64_t k = 0;
   for (long i = -1 + g->height; i >= 0; i--) {
      for (long j = 0; j < g->width; j++) {
         if (g->board[i][j] == 0) {
            char_board[k] = '.'; k++;
         }
         else {
            int len = sprintf(digits, "%" PRIu32, g->board[i][j]); 
            if (len > 1) {
               char_board[k] = ' '; k++; 
            }
            for (long s = 0; s < len; s++) {
               char_board[k] = digits[s]; k++;
            }
            if (len > 1) {
               char_board[k] = ' '; k++; 
            }
         }
      }
      char_board[k] = '\n'; k++;
   }
   char_board[k] = '\0';
   return char_board;
}

uint64_t gamma_all_free_fields(gamma_t *g) {
   return ((uint64_t)(g->height)) * ((uint64_t)(g->width)) -
           (uint64_t)g->num_of_busy_fields;
}

uint32_t gamma_give_player(gamma_t *g, uint32_t x, uint32_t y) {
   return g->board[y][x];
}
