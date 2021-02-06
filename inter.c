/** @file
 * Implementacja interfejsu funkcji odpowiedzialnej za start trybu interaktywnego.
 *
 * @author Jakub Bedełek <jb417705@students.mimuw.edu.pl>
 * @copyright Jakub Bedełek
 * @date 10.05.2020
 */
 
#include "inter.h"
#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

// * źródło: https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux
/**
 * Zmienne potrzebne do prawdłowej obsługi terminala.
 */
static struct termios old, newp;

/**
 * Liczba znaków skłądająca się na strzałkę.
 */
#define ARROW_KEYS 3


/**
 * Struktura odpowiedzialne za przechowywanie danych w trybie interaktywnym.
 */
typedef struct inter_s inter_t; 

/**
 * Struktura odpowiedzialne za przechowywanie danych w trybie interaktywnym.
 */
struct inter_s {
    uint32_t x;                      /**< Współrzędna kursora. */
    uint32_t y;                      /**< Współrzędna kursora. */
    uint32_t COL;                    /**< Szerokość planszy. */
    uint32_t ROW;                    /**< Wysokość planszy. */
    int length;                 /**< Ilość cyfry liczby players. */
    int modulo;                 /**< Ilość cyfr liczby players + 1. */
    bool moreThan10;            /**< True jeśli liczba graczy >= 10. */
    bool C_arrow;               /**< True jeśli została użyta strzałka z 'C'. */
    int count;                  /**< Liczby ostatnie 3 znaki. */
    int numbers[ARROW_KEYS];    /**< Zapamiętuje ostatnie 3 wpisane znaki. */
    uint32_t players;           /**< Liczba graczy. */
    uint32_t PLAYER;            /**< Numer aktualnie wypisywanego gracza. */
};

/**
 * Objekt odpowiedzialny za przechowywanie danych w trybie interaktywnym.
 */
static inter_t inter;

// * źródło: https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux
/** @brief Initialize new terminal i/o settings.
 * @param[in] echo - echo mode.
 */
static void initTermios(int echo) 
{
   int result1 = tcgetattr(0, &old); /* grab old terminal i/o settings */
   newp = old; /* make new settings same as old settings */
   newp.c_lflag &= ~ICANON; /* disable buffered i/o */
   newp.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
   int result2 = tcsetattr(0, TCSANOW, &newp); /* use these new terminal i/o settings now */

   if (result1 == -1 || result2 == -1) {
      exit(EXIT_FAILURE);
   }
}

// * źródło: https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux
/** @brief Restore old terminal i/o settings.
 */
static void resetTermios(void) 
{
   int result = tcsetattr(0, TCSANOW, &old);
   if (result == -1) {
      exit(EXIT_FAILURE);
   }
}

/** @brief Read 1 character without echo.
 * @return Return read character
 * źródło: https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux
 */
static char getch(void) {
   return getchar();
}


/** @brief Zmienia pozycję kursora na @p row i @p col.
 * @param[in] row - numer wiersza,
 * @param[in] col - numer kolumny.
 */
static void moveTo(int row, int col) {
   printf("\x1b[%d;%df", row, col);
}

/** @brief Sprawdzamy, czy dana litera ma szansę być starzłką.
 * @param[in] litera - kod ASCII litery.
 * @return Zwraca true jeśli dana litera ma szansę być strzałką.
 */
static bool check_arrow_key(int litera) {
    // Ascii codes of some letters.
    int esc = (int)'\e';
    int left_quad_bracket = (int)'[';
    int A = (int)'A';
    int D = (int)'D';
    
    return litera == esc || litera == left_quad_bracket || 
           (litera <= D && litera >= A);
}

/** @brief Ustawiamy kolor textu na zielony. */
static void green_text2() {
     printf("\x1b[32;1m");
}

/** @brief Przywracamy domyślne ustawienia terminala. */
static void clear_settings() {
     printf("\x1b[0;1m");
}

/** @brief Czyścimy linię ze znaków. */
static void clear_line() {
     printf("\x1b[2K");
}

/** @brief Czyścimy konsole ze znaków. */
static void clear_console() {
     printf("\x1b[2J");
}

/** @brief Czyści wyświetlanie w negatywie poprzedniego pola.
 *  @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry.
 */
static void clear_colors(gamma_t *g) {
   if (inter.moreThan10) {
       uint32_t gracz;
        if (inter.x % inter.modulo == 0) {
            gracz = gamma_give_player(g, (inter.x - 1)/inter.modulo, inter.y - 1);
        }
        
        clear_settings();
        if (inter.x % inter.modulo == 0) {
            int dlugosc2 = snprintf(NULL, 0, "%" PRIu32, gracz);
            // Wymazywanie wcześniejszych liczb spacjami.
            for (int k = 0; k <= dlugosc2; k++) {
                moveTo(inter.y, inter.x - dlugosc2 + k);
                printf(" "); 
            }   
            moveTo(inter.y, inter.x - dlugosc2 + 1);
            if (gracz != 0) {
            printf("%" PRIu32, gracz);
            }
            else {
            green_text2();
            printf(".");
         }
            moveTo(inter.y, inter.x); 
             
        }
    }
}

/** @brief Ustawiamy tło tekstu na biały. */
static void white_background() {
   printf("\x1b[47;1m");
}

/** @brief Ustawiamy kolor tekstu na czerny. */
static void black_text() {
   printf("\x1b[30;1m");
}

/** @brief Pokazuje kursor. */
static void show_cursor() {
   printf("\033[?25h");
}

/** @brief Chowa kursor. */
static void hide_cursor() {
   printf("\033[?25l");
}

/** @brief Wyświetla w negatywie aktualne pole.
 *  @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry.
 */
static void show_colors(gamma_t *g) {
    if (inter.moreThan10) {
       uint32_t gracz;
       if (inter.x % inter.modulo == 0) {
          gracz = gamma_give_player(g, (inter.x - 1)/inter.modulo, inter.y - 1);
       }
      
       clear_settings();
       if (inter.x % inter.modulo == 0) {
          int dlugosc2 = snprintf(NULL, 0, "%" PRIu32, gracz);
          // Wymazywanie wcześniejszych liczb spacjami.
          for (int k = 0; k <= dlugosc2; k++) {
              moveTo(inter.y, inter.x - dlugosc2 + k);
              printf(" "); 
          }   
          moveTo(inter.y, inter.x - dlugosc2 + 1);
          
          white_background();
          black_text();
          
          if (gracz != 0) {
             printf("%" PRIu32, gracz);
          }
          else {
             printf(".");
          }
          moveTo(inter.y, inter.x);   
      }
   }
}


/** @brief Sprawdza ostatni kod strzałki i jeśli jest on prawidłowy to zmienia 
 * współrzędne kursora.
 * @param[in] last - ostatni kod.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry.
 */
static void move(int last, gamma_t *g) {
    if (last == (int)'A') {
       if (inter.y > 1) {
          if (inter.moreThan10)
             clear_colors(g);
          inter.y--;
          if (inter.moreThan10)
             show_colors(g);
       }
    }
    else if ((char)last == (int)'B') {
       if (inter.y < inter.ROW) {
          if (inter.moreThan10) 
             clear_colors(g);
          inter.y++;
          if (inter.moreThan10)
             show_colors(g);
       }
    }
    else if ((char)last == (int)'C') {
        if (inter.moreThan10) {
            if (inter.x < inter.modulo * inter.COL) {
            clear_colors(g);
                inter.x += inter.modulo;
                show_colors(g);
            }
        }
        else if (inter.x < inter.COL) {
            inter.x++;
        }
    }
    else if ((char)last == (int)'D') {
        if (inter.moreThan10) {
            if (inter.x * inter.COL > inter.modulo * inter.COL) {
            clear_colors(g);
                inter.x -= inter.modulo;
                show_colors(g);
            }
        }
        else if (inter.x > 1) {
            inter.x--;
        }
    }
}

/** @brief Wypisuje tablicę na wyjście.
 */
static void show_board() {
    for (uint32_t i = 0; i < inter.ROW; i++) {
        for (uint32_t j = 0; j < inter.COL; j++) {
            if (inter.moreThan10) {
                for (int j = 0; j < inter.length; j++) {
                    printf(" ");
                }
            }    
            printf(".");
        }
        printf("\n");
    }
}

/** @brief Przywraca kursor do lewego górnego rogu.
 */
static void go_to_begin() {
    moveTo(1, 1);
}

/** @brief Inicjalizuję danego do rozgrywki w trybie interaktywnym.
 * @param[in] width - szerokość planszy,
 * @param[in] height - wysokośc planszy,
 * @param[in] fplayers - liczba graczy.
 */
static void init_data_interactive(uint32_t width, uint32_t height,
                                  uint32_t fplayers) {
    initTermios(0);
    inter.PLAYER = 1;
    inter.count = 0;
    inter.ROW = height; 
    inter.COL = width;    
    inter.x = 1; 
    inter.y = height;
    inter.C_arrow = true;
    inter.moreThan10 = false;
    inter.players    = fplayers;

    inter.length = snprintf(NULL, 0, "%" PRIu32, inter.players);
    inter.modulo = inter.length + 1; 
    
    if (inter.players >= 10) {
        inter.moreThan10 = true; 
        inter.x = inter.modulo;
    }
    
    for (int i = 0; i < ARROW_KEYS; i++) {
        inter.numbers[i] = 0;
    }
}

/** @brief Wypisuje początkowe dane.
 */
static void show_begin() {
    go_to_begin();
    show_board();
    clear_line();
    clear_settings();
}

/** @brief Aktualizuje pozycję kursora.
 */
static void updateCursor() {
    moveTo(inter.y, inter.x);
}

/** @brief Czyta strzałki i wykonuje ruch.
 */
static void read_arrow(gamma_t *g) {
   int esc = (int)'\e';
    if (inter.count == 2 && inter.numbers[0] == esc &&
        inter.numbers[1] == (int)'[') {
        move(inter.numbers[2], g);
        inter.C_arrow = false;
    }    
    else if (inter.count == 1 && inter.numbers[2] == esc &&
             inter.numbers[0] == (int)'[') {
        move(inter.numbers[1], g);
        inter.C_arrow = false;
    }
    else if (inter.count == 0 && inter.numbers[1] == esc &&
             inter.numbers[2] == (int)'[') {
        move(inter.numbers[0], g);
        inter.C_arrow = false;
    }
    else {
       inter.C_arrow = true;
    }
    updateCursor();
}

/** @brief Czyta znak spacji i wykonuje ruch. 
 * @param[in] g - struktura przchowywująca stan gry.
 */
static void read_space(gamma_t *g) {
    if (inter.moreThan10) {
        if (inter.x % inter.modulo == 0 &&
            gamma_move(g, inter.PLAYER, (inter.x - 1)/inter.modulo, inter.y - 1)) {
            int dlugosc = snprintf(NULL, 0, "%" PRIu32, inter.PLAYER);
            moveTo(inter.y, inter.x - dlugosc + 1);
            printf("%" PRIu32, inter.PLAYER);
            moveTo(inter.y, inter.x); 

            inter.PLAYER %= inter.players;
            inter.PLAYER++;                    
        }
    }
    else if (gamma_move(g, inter.PLAYER, inter.x - 1, inter.y - 1)) {
        moveTo(inter.y, inter.x);
        printf("%" PRIu32, inter.PLAYER); 
        moveTo(inter.y, inter.x);
    
        inter.PLAYER %= inter.players;
        inter.PLAYER++;
    }
}

/** @brief Czyta znak złotego ruchu i wykonuje ruch.
 * @param[in] g - struktura przechowywująca stan gry.
 */
static void read_gold(gamma_t *g) {
    if (inter.moreThan10) {
        uint32_t gracz;
        if (inter.x % inter.modulo == 0) {
            gracz = gamma_give_player(g, (inter.x - 1)/inter.modulo, inter.y - 1);
        }
        
        if (inter.x % inter.modulo == 0 &&
            gamma_golden_move(g, inter.PLAYER, (inter.x - 1)/inter.modulo,
                              inter.y - 1)) {
            int dlugosc2 = snprintf(NULL, 0, "%" PRIu32, gracz);
            int dlugosc = snprintf(NULL, 0, "%" PRIu32, inter.PLAYER);
            // Wymazywanie wcześniejszych liczb spacjami.
            for (int k = 0; k <= dlugosc2; k++) {
                moveTo(inter.y, inter.x - dlugosc2 + k);
                printf(" "); 
            }
            moveTo(inter.y, inter.x - dlugosc + 1);
            black_text();
            printf("%" PRIu32, inter.PLAYER);
            moveTo(inter.y, inter.x); 

            inter.PLAYER %= inter.players;
            inter.PLAYER++;                    
        }
    }
    else if (gamma_golden_move(g, inter.PLAYER, inter.x - 1, inter.y - 1)) {
        moveTo(inter.y, inter.x);
        printf("%" PRIu32, inter.PLAYER);
        moveTo(inter.y, inter.x);
        
        inter.PLAYER %= inter.players;
        inter.PLAYER++;
    }
}

/** @brief Pomija jednego gracza.
 */
static void skip_one_player() {
    inter.PLAYER %= inter.players;
    inter.PLAYER++;    
}

/** @brief Pomija wszystkich graczy, którzy nie mogą wykonać ruchu.
 * @param[in] g - struktura przechowywująca stan gry.
 * @return Zwraca true jeśli wszyscy gracze zostali pominięci lub false w
 * przeciwnym przypadku.
 */
static bool skip_players(gamma_t *g) {
    uint32_t count2 = 1; 
        
    while (count2 <= inter.players && gamma_free_fields(g, count2) == 0
           && gamma_golden_possible(g, count2) == false) {
        count2++; 
    }
    
    uint32_t player_num = inter.PLAYER;
    
    while (player_num <= inter.players && 
           gamma_free_fields(g, player_num) == 0 &&
           gamma_golden_possible(g, player_num) == false) {
        player_num++;
    }
    
    if (player_num == inter.players + 1) {
        player_num = 1;
        
        while (player_num <= inter.PLAYER &&
               gamma_free_fields(g, player_num) == 0 
               && gamma_golden_possible(g, player_num) == false) {
            player_num++;
        }
    }
    
    inter.PLAYER = player_num;
    
    inter.count++; 
    inter.count %= 3;
    
    if (count2 == inter.players + 1) {
        return true;
    }

    return false;
}

/** @brief Pokazuje dane następnego gracza.
 * @param[in] g - struktura przechowywująca stan gry.
 */
static void show_next_player(gamma_t *g) {
    moveTo(inter.ROW + 1, 1);
    clear_settings();
    clear_line();
    printf("Player %" PRIu32, inter.PLAYER);
    moveTo(inter.ROW + 2, 1);
    clear_line();
    printf("Busy Fields %" PRIu64, gamma_busy_fields(g, inter.PLAYER));
    moveTo(inter.ROW + 3, 1);
    clear_line();
    printf("Board free fields %" PRIu64, gamma_all_free_fields(g));
    moveTo(inter.ROW + 4, 1);
    clear_line();
    if (gamma_golden_possible(g, inter.PLAYER)) {
       printf("Golden move YES");
    }
    else {
       printf("Golden move NO");
    }
    updateCursor();
}

/** @brief Pokazuje wyniki gry.
 * @param[in] g - struktura przechowywująca stan gry. 
 */
static void show_results(gamma_t *g) {
   clear_settings();
   // Liczba linijek wypisywana dla każdego gracza po zakończonej grze.
   uint32_t number_of_lines = 4;
   if (number_of_lines > inter.players) {
      for (uint32_t i = 1; i <= number_of_lines; i++) {
         moveTo(inter.ROW + i, 1);
         clear_line();
      }   
   }
   
    for (uint32_t i = 1; i <= inter.players; i++) {
        moveTo(inter.ROW + i, 1); 
        clear_line();
        printf("Player %d has taken %" PRIu64 " fields.\n", i, gamma_busy_fields(g, i));
    }
}

/** @brief Sprawdza, czy rozmiar terminala jest prawidłowy.
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia,
 * @param[in] players – liczba graczy, liczba dodatnia.
 * @return Zwraca @p true jeśli rozmiar terminala jest prawidłowy i @p false
 * w przeciwnym przypadku.
 */
static bool terminal_size_ok(uint32_t width, uint32_t height, uint32_t players) {
    // Liczba linii potrzebna do opisu gracza.
    int player_description = 4;
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    
    // a - liczba cyfr liczby players.
    int a = players;
    int i = 0;
    
    while (a > 0) {
       a = a / 10;   
       i++;  
    }
    
    if (i > 1) {
       i++;
    }
    
    if (ws.ws_row < height + player_description || width * i > ws.ws_col) {
       go_to_begin();
       printf("Terminal is to small for this game.\n");
       return false;
    }
   return true;
}

void start_interactive(gamma_t *g, uint32_t width, 
                       uint32_t height, uint32_t players) {
    clear_console();
    if (!terminal_size_ok(width, height, players)) {
        return;
    }
    
    green_text2();
    init_data_interactive(width, height, players);
    if (inter.moreThan10)
        hide_cursor();
    
    show_begin();
    show_next_player(g);
    updateCursor();
    
    if (inter.moreThan10)
        show_colors(g);
    
    char c;
    clear_settings();
    while ((c = getch()) != '\4') {  /// \4 - znak końca wczytywania tekstu.
        updateCursor();
        int int_c = (int)c;
        inter.numbers[inter.count] = int_c;    
        
        if (check_arrow_key(int_c)) {
            read_arrow(g);
        }
        else if (c == ' ') {
            read_space(g);
            show_colors(g);
        }
        else if (c == 'G' || c == 'g') {
            read_gold(g);
            show_colors(g);
        }
        
        if ((c == 'C' && inter.C_arrow) || c == 'c') 
            skip_one_player();
        if (skip_players(g))
            break;
            
        updateCursor();
        show_next_player(g);
    }
    clear_colors(g);
    show_results(g);
    show_cursor();
    resetTermios();
}
