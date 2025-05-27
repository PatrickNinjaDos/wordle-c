#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "raylib.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 900
#define MAX_WORD_LENGTH 10
#define MAX_ATTEMPTS 6
#define CELL_SIZE 60
#define CELL_SPACING 10
#define KEYBOARD_KEY_SIZE 40
#define KEYBOARD_SPACING 5

typedef enum {
    LETTER_UNKNOWN = 0,
    LETTER_CORRECT,
    LETTER_PRESENT,
    LETTER_ABSENT
} LetterState;

typedef struct {
    char letter;
    LetterState state;
} Cell;

typedef struct {
    char key_char;
    LetterState key_state;
    Rectangle key_rect;
} GameKeyboardKey;

typedef struct {
    Cell grid[MAX_ATTEMPTS][MAX_WORD_LENGTH];
    int current_row;
    int current_col;
    char target_word[MAX_WORD_LENGTH + 1];
    int word_length;
    bool game_over;
    bool won;
    char current_guess[MAX_WORD_LENGTH + 1];
    GameKeyboardKey keyboard[26];
    int attempts_left;
    int difficulty;
} GameState;

// functiile folosite
char* get_random_word(const char *filename, int difficulty);
void init_game(GameState *game, const char *filename);
void draw_game(GameState *game);
void draw_grid(GameState *game);
void draw_keyboard(GameState *game);
void handle_input(GameState *game);
void submit_guess(GameState *game);
void update_keyboard_state(GameState *game, const char *guess, const LetterState *states);
LetterState* evaluate_guess(const char *guess, const char *target);
Color get_cell_color(LetterState state);
Color get_text_color(LetterState state);
void init_keyboard(GameState *game);
int get_difficulty_selection();

char* get_random_word(const char *filename, int difficulty) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }
    
    char **valid_words = NULL;
    int valid_count = 0;
    char word[45];
    
    while (fgets(word, sizeof(word), file) != NULL) {
        size_t len = strlen(word);
        if (len > 0 && word[len - 1] == '\n') {
            word[len - 1] = '\0';
            len--;
        }
        
        bool is_valid = false;
        
        switch (difficulty) {
            case 1: 
                if (len >= 2 && len <= 4) is_valid = true;
                break;
            case 2: 
                if (len == 5) is_valid = true;
                break;
            case 3: 
                if (len >= 5 && len <= MAX_WORD_LENGTH) is_valid = true;
                break;
        }
        
        if (is_valid) {
            valid_count++;
            valid_words = realloc(valid_words, valid_count * sizeof(char*));
            valid_words[valid_count - 1] = malloc((len + 1) * sizeof(char));
            strcpy(valid_words[valid_count - 1], word);
        }
    }
    
    fclose(file);
    
    if (valid_count == 0) {
        printf("No valid words found for the selected difficulty.\n");
        return NULL;
    }
    
    int random_index = rand() % valid_count;
    char *selected_word = strdup(valid_words[random_index]);
    
    // trecere la litere mari
    for (int i = 0; selected_word[i]; i++) {
        selected_word[i] = toupper(selected_word[i]);
    }
    
    for (int i = 0; i < valid_count; i++) {
        free(valid_words[i]);
    }
    free(valid_words);
    
    return selected_word;
}

void init_game(GameState *game, const char *filename) { //initializare gridului de joc
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        for (int j = 0; j < MAX_WORD_LENGTH; j++) {
            game->grid[i][j].letter = ' ';
            game->grid[i][j].state = LETTER_UNKNOWN;
        }
    }
    
    game->current_row = 0;
    game->current_col = 0;
    game->game_over = false;
    game->won = false;
    memset(game->current_guess, 0, sizeof(game->current_guess));
    
    char *word = get_random_word(filename, game->difficulty);
    if (word) {
        strcpy(game->target_word, word);
        game->word_length = strlen(word);
        free(word);
    } else {
        strcpy(game->target_word, "HELLO");
        game->word_length = 5;
    }
    
    game->attempts_left = MAX_ATTEMPTS;
    
    init_keyboard(game);
}

void init_keyboard(GameState *game) {
    const char* qwerty_layout[3][10] = {
        {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
        {"A", "S", "D", "F", "G", "H", "J", "K", "L", ""},
        {"Z", "X", "C", "V", "B", "N", "M", "", "", ""}
    };
    
    int key_index = 0;
    float start_y = SCREEN_HEIGHT - 200;
    
    for (int row = 0; row < 3; row++) {
        int keys_in_row = 0;
        
        // numara cate litere
        for (int col = 0; col < 10 && qwerty_layout[row][col][0] != '\0'; col++) {
            keys_in_row++;
        }
        
        float start_x = (SCREEN_WIDTH - (keys_in_row * KEYBOARD_KEY_SIZE + (keys_in_row - 1) * KEYBOARD_SPACING)) / 2.0f;
        
        for (int col = 0; col < keys_in_row; col++) {
            if (key_index < 26) {
                game->keyboard[key_index].key_char = qwerty_layout[row][col][0];
                game->keyboard[key_index].key_state = LETTER_UNKNOWN;
                game->keyboard[key_index].key_rect = (Rectangle){
                    start_x + col * (KEYBOARD_KEY_SIZE + KEYBOARD_SPACING),
                    start_y + row * (KEYBOARD_KEY_SIZE + KEYBOARD_SPACING),
                    KEYBOARD_KEY_SIZE,
                    KEYBOARD_KEY_SIZE
                };
                key_index++;
            }
        }
    }
}

Color get_cell_color(LetterState state) {
    switch (state) {
        case LETTER_CORRECT: return (Color){83, 141, 78, 255};   // verde
        case LETTER_PRESENT: return (Color){181, 159, 59, 255};  // galben
        case LETTER_ABSENT: return (Color){58, 58, 60, 255};     // negru
        default: return (Color){129, 131, 132, 255};             // gri
    }
}

Color get_text_color(LetterState state) {
    if (state == LETTER_UNKNOWN) {
        return BLACK;
    }
    return WHITE;
}

LetterState* evaluate_guess(const char *guess, const char *target) {
    static LetterState states[MAX_WORD_LENGTH];
    int target_len = strlen(target);
    
    // pune toate ca lipsa
    for (int i = 0; i < target_len; i++) {
        states[i] = LETTER_ABSENT;
    }
    
    // numara cate litere are cuvantul
    int target_count[26] = {0};
    for (int i = 0; i < target_len; i++) {
        if (target[i] >= 'A' && target[i] <= 'Z') {
            target_count[target[i] - 'A']++;
        }
    }
    
    // marcheaza pozitile corecte
    for (int i = 0; i < target_len; i++) {
        if (guess[i] == target[i]) {
            states[i] = LETTER_CORRECT;
            if (target[i] >= 'A' && target[i] <= 'Z') {
                target_count[target[i] - 'A']--;
            }
        }
    }
    
    // marcheaza literele prezente dar nu pe pozitii corecte
    for (int i = 0; i < target_len; i++) {
        if (states[i] == LETTER_ABSENT && guess[i] >= 'A' && guess[i] <= 'Z') {
            if (target_count[guess[i] - 'A'] > 0) {
                states[i] = LETTER_PRESENT;
                target_count[guess[i] - 'A']--;
            }
        }
    }
    
    return states;
}

void update_keyboard_state(GameState *game, const char *guess, const LetterState *states) {
    for (int i = 0; i < game->word_length; i++) {
        char letter = guess[i];
        
        // pentru fiecare cheie din tastatura
        for (int j = 0; j < 26; j++) {
            if (game->keyboard[j].key_char == letter) {
                // updateaza doar daca se gasesc noutati
                if (states[i] == LETTER_CORRECT || 
                    (states[i] == LETTER_PRESENT && game->keyboard[j].key_state != LETTER_CORRECT) ||
                    (states[i] == LETTER_ABSENT && game->keyboard[j].key_state == LETTER_UNKNOWN)) {
                    game->keyboard[j].key_state = states[i];
                }
                break;
            }
        }
    }
}

void submit_guess(GameState *game) {
    if (game->current_col != game->word_length) return;
    
    // pune cuvantul introdus in grid
    for (int i = 0; i < game->word_length; i++) {
        game->grid[game->current_row][i].letter = game->current_guess[i];
    }
    
    // valideaza cuvantul introdus
    LetterState *states = evaluate_guess(game->current_guess, game->target_word);
    
    // updateaza gridul 
    for (int i = 0; i < game->word_length; i++) {
        game->grid[game->current_row][i].state = states[i];
    }
    
    // updateaza tastatura
    update_keyboard_state(game, game->current_guess, states);
    
    // verifica daca este ghicit cuvantul initial
    if (strcmp(game->current_guess, game->target_word) == 0) {
        game->won = true;
        game->game_over = true;
    }
    
    // muta la urmatorul rand
    game->current_row++;
    game->current_col = 0;
    memset(game->current_guess, 0, sizeof(game->current_guess));
    game->attempts_left--;
    
    // verifica daca mai sunt vieti
    if (game->current_row >= MAX_ATTEMPTS && !game->won) {
        game->game_over = true;
    }
}

// tastele fizice -> front end

void handle_input(GameState *game) {
    if (game->game_over) {
        if (IsKeyPressed(KEY_R)) {
            init_game(game, "words.txt");
        }
        return;
    }
    
    // letter input
    for (int key_code = KEY_A; key_code <= KEY_Z; key_code++) {
        if (IsKeyPressed(key_code) && game->current_col < game->word_length) {
            char letter = 'A' + (key_code - KEY_A);
            game->current_guess[game->current_col] = letter;
            game->grid[game->current_row][game->current_col].letter = letter;
            game->current_col++;
            break;
        }
    }
    
    // backspace
    if (IsKeyPressed(KEY_BACKSPACE) && game->current_col > 0) {
        game->current_col--;
        game->current_guess[game->current_col] = '\0';
        game->grid[game->current_row][game->current_col].letter = ' ';
    }
    
    // enter
    if (IsKeyPressed(KEY_ENTER)) {
        submit_guess(game);
    }
    
    // mouse click stanga
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        
        for (int i = 0; i < 26; i++) {
            if (CheckCollisionPointRec(mousePos, game->keyboard[i].key_rect)) {
                if (game->current_col < game->word_length) {
                    char letter = game->keyboard[i].key_char;
                    game->current_guess[game->current_col] = letter;
                    game->grid[game->current_row][game->current_col].letter = letter;
                    game->current_col++;
                }
                break;
            }
        }
    }
}

//desenat de fundal si litere
void draw_grid(GameState *game) {
    float start_x = (SCREEN_WIDTH - (game->word_length * CELL_SIZE + (game->word_length - 1) * CELL_SPACING)) / 2.0f;
    float start_y = 100;
    
    for (int row = 0; row < MAX_ATTEMPTS; row++) {
        for (int col = 0; col < game->word_length; col++) {
            float x = start_x + col * (CELL_SIZE + CELL_SPACING);
            float y = start_y + row * (CELL_SIZE + CELL_SPACING);
            
            Rectangle rect = {x, y, CELL_SIZE, CELL_SIZE};
            
            // fundal
            Color bg_color = get_cell_color(game->grid[row][col].state);
            DrawRectangleRec(rect, bg_color);
            DrawRectangleLinesEx(rect, 2, BLACK);
            
            // litera
            if (game->grid[row][col].letter != ' ') {
                char letter_str[2] = {game->grid[row][col].letter, '\0'};
                Color text_color = get_text_color(game->grid[row][col].state);
                
                Vector2 text_size = MeasureTextEx(GetFontDefault(), letter_str, 32, 0);
                float text_x = x + (CELL_SIZE - text_size.x) / 2;
                float text_y = y + (CELL_SIZE - text_size.y) / 2;
                
                DrawText(letter_str, (int)text_x, (int)text_y, 32, text_color);
            }
        }
    }
}

//desenat de fundal de litere
void draw_keyboard(GameState *game) {
    for (int i = 0; i < 26; i++) {
        Rectangle rect = game->keyboard[i].key_rect;
        
        Color bg_color = get_cell_color(game->keyboard[i].key_state);
        DrawRectangleRec(rect, bg_color);
        DrawRectangleLinesEx(rect, 1, BLACK);
        
        char key_str[2] = {game->keyboard[i].key_char, '\0'};
        Color text_color = get_text_color(game->keyboard[i].key_state);
        
        Vector2 text_size = MeasureTextEx(GetFontDefault(), key_str, 20, 0);
        float text_x = rect.x + (rect.width - text_size.x) / 2;
        float text_y = rect.y + (rect.height - text_size.y) / 2;
        
        DrawText(key_str, (int)text_x, (int)text_y, 20, text_color);
    }
    
    DrawText("ENTER", 50, SCREEN_HEIGHT - 150, 16, BLACK);
    DrawText("BACKSPACE", 650, SCREEN_HEIGHT - 150, 16, BLACK);
}

//desenat -> main
void draw_game(GameState *game) {
    BeginDrawing();
    ClearBackground(WHITE);
    
    // titlu
    DrawText("WORDLE", SCREEN_WIDTH/2 - 60, 20, 40, BLACK);
    
    // dificultate
    const char* diff_text = (game->difficulty == 1) ? "USOR (2-4 litere)" :
                           (game->difficulty == 2) ? "MEDIU (5 litere)" : "GREU (5+ litere)";
    DrawText(diff_text, SCREEN_WIDTH/2 - 80, 60, 16, GRAY);
    
    // grid
    draw_grid(game);
    
    // keyboard
    draw_keyboard(game);
    
    // vieti
    if (game->game_over) {
        if (game->won) {
            DrawText("FELICITARI! AI CASTIGAT!", SCREEN_WIDTH/2 - 150, 580, 24, GREEN);
        } else {
            char lose_text[100];
            snprintf(lose_text, sizeof(lose_text), "AI PIERDUT! Cuvantul era: %s", game->target_word);
            DrawText(lose_text, SCREEN_WIDTH/2 - 200, 580, 20, RED);
        }
        DrawText("Apasa R pentru a juca din nou", SCREEN_WIDTH/2 - 120, 650, 16, BLUE);
    } else {
        char attempts_text[50];
        snprintf(attempts_text, sizeof(attempts_text), "Incercari ramase: %d", game->attempts_left);
        DrawText(attempts_text, SCREEN_WIDTH/2 - 80, 550, 16, BLACK);
    }
    
    // instructiuni
    DrawText("Foloseste tastatura pentru a scrie", 20, SCREEN_HEIGHT - 40, 14, GRAY);
    DrawText("ENTER pentru a trimite, BACKSPACE pentru a sterge", 20, SCREEN_HEIGHT - 20, 14, GRAY);
    
    EndDrawing();
}

int get_difficulty_selection() {
    InitWindow(400, 300, "Selecteaza Dificultatea");
    SetTargetFPS(60);
    
    int selected_difficulty = 2;
    bool selection_made = false;
    
    while (!WindowShouldClose() && !selection_made) {
        if (IsKeyPressed(KEY_ONE)) {
            selected_difficulty = 1;
            selection_made = true;
        } else if (IsKeyPressed(KEY_TWO)) {
            selected_difficulty = 2;
            selection_made = true;
        } else if (IsKeyPressed(KEY_THREE)) {
            selected_difficulty = 3;
            selection_made = true;
        }
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            if (mousePos.y >= 80 && mousePos.y < 120) {
                selected_difficulty = 1;
                selection_made = true;
            } else if (mousePos.y >= 120 && mousePos.y < 160) {
                selected_difficulty = 2;
                selection_made = true;
            } else if (mousePos.y >= 160 && mousePos.y < 200) {
                selected_difficulty = 3;
                selection_made = true;
            }
        }
        
        BeginDrawing();
        ClearBackground(WHITE);
        
        DrawText("Selecteaza Dificultatea", 50, 20, 24, BLACK);
        
        DrawText("1. USOR (2-4 litere, incercari infinite)", 20, 80, 16, BLUE);
        DrawText("2. MEDIU (5 litere, 6 incercari)", 20, 120, 16, BLUE);
        DrawText("3. GREU (5+ litere, 6 incercari)", 20, 160, 16, BLUE);
        
        DrawText("Apasa 1, 2 sau 3 pentru a alege", 50, 220, 16, GRAY);
        DrawText("sau da click pe optiunea dorita", 50, 240, 16, GRAY);
        
        EndDrawing();
    }
    
    CloseWindow();
    return selection_made ? selected_difficulty : 2;
}

int main(void) {
    srand(time(NULL));
    
    // selectare de dificultate
    int difficulty = get_difficulty_selection();
    
    // initilizarea ferestrei de joc
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wordle Game");
    SetTargetFPS(60);
    
    GameState game = {0};
    game.difficulty = difficulty;
    init_game(&game, "words.txt");
    
    while (!WindowShouldClose()) {
        handle_input(&game);
        draw_game(&game);
    }
    
    CloseWindow();
    return 0;
}