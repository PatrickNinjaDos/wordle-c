#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MULTIPLAYER_LIVES 5
#define MULTIPLAYER_WORD_LEN 5

int count_words_in_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 0;
    }
    
    int count = 0;
    char word[45]; 
    
    while (fgets(word, sizeof(word), file) != NULL) {
        size_t len = strlen(word);
        if (len > 0 && word[len - 1] == '\n') {
            word[len - 1] = '\0';
        }
        
        if (strlen(word) > 0) {
            count++;
        }
    }
    
    fclose(file);
    return count;
}

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
                if (len >= 5) is_valid = true;
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
    
    for (int i = 0; i < valid_count; i++) {
        free(valid_words[i]);
    }
    free(valid_words);
    
    return selected_word;
}

bool verificare_castig(char *word, char *letter_display)
{
    if (strcmp(word, letter_display) == 0) return true;
    return false;
}

void clear_input_buffer() 
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int difficulty_selection()
{
    int difficulty=0, incercari=0;
    printf("Alegeti dificultatea:\n");
    printf("Cod | Dificultate | Incercari | Dimensiune\n");
    printf("(1) | Usor        | Infinit   | 2-4 litere\n");
    printf("(2) | Mediu       | 6         | 5 litere\n");
    printf("(3) | Greu        | 3         | 5-44 litere\n");
    start:
    scanf("%d", &difficulty);
    clear_input_buffer();
    switch (difficulty)
    {
        case 1:
            printf("Ati ales dificultatea usor.\n");
            incercari = 999;
            break;
        case 2:
            printf("Ati ales dificultatea mediu.\n");
            incercari = 6;
            break;
        case 3:
            printf("Ati ales dificultatea greu.\n");
            incercari = 3;
            break;
        default:
            printf("Dificultatea aleasa nu exista.\n");
            goto start;
    }
    return incercari;
}

char *read_word()
{
    printf("Introduceti un cuvant: ");
    // cel mai lung cuvant are 44 de caractere
    char *word = malloc(45 * sizeof(char));
    if (word == NULL) return NULL;

    fgets(word, 45, stdin);
    size_t len = strlen(word);

    //eliminam newline-ul de la final
    if (len > 0 && word[len-1] == '\n') word[len-1] = '\0';

    return word;
}

char *letter_compare(char *guess, char *word, char *letter_display)
{
    int i = 0;
    while (word[i] != '\0')
    {
        if (guess[i] == word[i]) letter_display[i] = guess[i];
        i++;
    }
    return letter_display;
}

void display_colored_guess(char *guess, char *word)
{
    int word_len = strlen(word);
    char **display_lines = malloc(word_len * sizeof(char*));
    
    bool *matched = calloc(word_len, sizeof(bool));
    
    for (int i = 0; i < word_len; i++) {
        if (guess[i] == word[i]) {
            matched[i] = true;
        }
    }
    
    for (int i = 0; i < word_len; i++) {
        display_lines[i] = malloc(20 * sizeof(char));
        
        if (guess[i] == word[i]) {
            sprintf(display_lines[i], "🟩 %c ", guess[i]);
        } else {
            bool found = false;
            for (int j = 0; j < word_len; j++) {
                if (!matched[j] && guess[i] == word[j]) {
                    sprintf(display_lines[i], "🟨 %c ", guess[i]);
                    matched[j] = true;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                sprintf(display_lines[i], "⬜ %c ", guess[i]);
            }
        }
    }
   
    for (int i = 0; i < word_len; i++) {
        printf("%s", display_lines[i]);
    }
    printf("\n");
    
    for (int i = 0; i < word_len; i++) {
        free(display_lines[i]);
    }
    free(display_lines);
    free(matched);
}

char *codify_result(char *guess, char *word) {
    int word_len = strlen(word);
    char *result = malloc((word_len + 1) * sizeof(char));
    
    if (result == NULL) return NULL;
    
    char *word_copy = strdup(word);
    if (word_copy == NULL) {
        free(result);
        return NULL;
    }
    
    for (int i = 0; i < word_len; i++) {
        if (guess[i] == word[i]) {
            result[i] = '2'; // verde
            word_copy[i] = '_'; 
        } else {
            result[i] = '0'; // gri
        }
    }
    
    for (int i = 0; i < word_len; i++) {
        if (result[i] == '0') { // pozitie libera
            for (int j = 0; j < word_len; j++) {
                if (guess[i] == word_copy[j]) {
                    result[i] = '1'; // galben
                    word_copy[j] = '_'; 
                    break;
                }
            }
        }
    }
    
    result[word_len] = '\0';
    free(word_copy);
    return result;
}

void display_result_from_code(char *guess, char *result_code) {
    int len = strlen(guess);
    char **display_lines = malloc(len * sizeof(char*));
    
    for (int i = 0; i < len; i++) {
        display_lines[i] = malloc(20 * sizeof(char));
        
        switch (result_code[i]) {
            case '2':
                sprintf(display_lines[i], "🟩 %c ", guess[i]);
                break;
            case '1': 
                sprintf(display_lines[i], "🟨 %c ", guess[i]);
                break;
            default:
                sprintf(display_lines[i], "⬜ %c ", guess[i]);
                break;
        }
    }
    
    for (int i = 0; i < len; i++) {
        printf("%s", display_lines[i]);
    }
    printf("\n");
    
    for (int i = 0; i < len; i++) {
        free(display_lines[i]);
    }
    free(display_lines);
}

void display_guess_history(char **guess_history, int num_guesses, char *word)
{
    printf("Istoric incercari:\n");
    
    for (int i = 0; i < num_guesses; i++) {
        display_colored_guess(guess_history[i], word);
    }
    
    printf("\n");
}

bool guess_the_word(char *word, char **letter_display, int *incercari, char ***guess_history, int *num_guesses)
{
    char *guess = read_word();
    if (guess == NULL || strlen(guess) != strlen(word)) 
    {
        printf("Cuvantul introdus nu este valid. Trebuie să aibă %lu litere.\n", strlen(word));
        free(guess);
        return false;
    }
    
    *letter_display = letter_compare(guess, word, *letter_display);
    
    // adaugam guessul in istoric
    (*num_guesses)++;
    *guess_history = realloc(*guess_history, (*num_guesses) * sizeof(char*));
    if (*guess_history == NULL)
    {
        free(guess);
        return false;
    }
    
    // alocam spatiu pt guess si il copiem
    (*guess_history)[(*num_guesses) - 1] = malloc((strlen(guess) + 1) * sizeof(char));
    if ((*guess_history)[(*num_guesses) - 1] == NULL)
    {
        free(guess);
        return false;
    }
    strcpy((*guess_history)[(*num_guesses) - 1], guess);
    
    // afisare
    display_guess_history(*guess_history, *num_guesses, word);

    if (strcmp(word, guess) == 0) 
    {
        free(guess);
        (*incercari)--;
        return true;
    }
    else if(guess != NULL)
    {
        free(guess);
        (*incercari)--;
        return false;
    }
}

// functia pentru a crea un socket de server
int create_server_socket() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    // fisier de socket descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // setam socketul sa reutilizez adresa si portul introdus
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // legam socketul de adresa si portul introdus
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // asteptam conectie
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d\n", PORT);
    return server_fd;
}

// creare de conexiune
int accept_client_connection(int server_fd) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int new_socket;
    
    printf("Așteptăm conectarea unui jucător...\n");
    
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Jucător conectat!\n");
    return new_socket;
}

// conectarea la server
int connect_to_server(const char *server_ip) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    // cream client socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // convertim IPv4 si IPv6 addresse din text in binary
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }
    
    // connectam la server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
    
    printf("Conectat la server!\n");
    return sock;
}

// functie pentru a rula server ca host
void run_server_game() {
    int server_fd = create_server_socket();
    int client_socket = accept_client_connection(server_fd);
    char buffer[BUFFER_SIZE] = {0};
    
    // introducere de cuvinte de ghicit de catre jucatori
    char *my_word = NULL;
    do {
        if (my_word != NULL) {
            free(my_word);
        }
        printf("Alege un cuvânt de %d litere pentru oponent: ", MULTIPLAYER_WORD_LEN);
        my_word = malloc((MULTIPLAYER_WORD_LEN + 1) * sizeof(char));
        fgets(my_word, MULTIPLAYER_WORD_LEN + 2, stdin);
        
        // scoate newline
        size_t len = strlen(my_word);
        if (len > 0 && my_word[len - 1] == '\n') {
            my_word[len - 1] = '\0';
            len--;
        }
        
        if (len != MULTIPLAYER_WORD_LEN) {
            printf("Cuvântul trebuie să aibă exact %d litere!\n", MULTIPLAYER_WORD_LEN);
        }
    } while (strlen(my_word) != MULTIPLAYER_WORD_LEN);
    
    // trimite cuvantul meu la client
    send(client_socket, my_word, strlen(my_word), 0);
    
    // citeste cuvantul primit
    read(client_socket, buffer, BUFFER_SIZE);
    char *opponent_word = strdup(buffer);
    printf("Ai primit un cuvânt de %d litere pentru a ghici.\n", MULTIPLAYER_WORD_LEN);
    
    // initializare
    int lives = MULTIPLAYER_LIVES;
    char *letter_display = malloc((MULTIPLAYER_WORD_LEN + 1) * sizeof(char));
    
    for (int i = 0; i < MULTIPLAYER_WORD_LEN; i++) {
        letter_display[i] = '_';
    }
    letter_display[MULTIPLAYER_WORD_LEN] = '\0';
    
    char **guess_history = NULL;
    int num_guesses = 0;
    bool game_over = false;
    bool i_won = false;
    bool opponent_won = false;
    
    memset(buffer, 0, BUFFER_SIZE);
    
    // Main game loop
    while (!game_over) {
        // tura
        if (lives > 0) {
            printf("\nEste rândul tău. Mai ai %d încercări.\n", lives);
            printf("Cuvântul are %d litere: ", MULTIPLAYER_WORD_LEN);
            for (int i = 0; i < MULTIPLAYER_WORD_LEN; i++) {
                printf("%c ", letter_display[i]);
            }
            printf("\n");
            
            // primeste cuvantul de ghicit
            char *guess = read_word();
            if (guess == NULL || strlen(guess) != MULTIPLAYER_WORD_LEN) {
                printf("Cuvantul introdus nu este valid. Trebuie să aibă %d litere.\n", MULTIPLAYER_WORD_LEN);
                if (guess != NULL) free(guess);
                continue;
            }
            
            // trimite guessul meu oponentului
            send(client_socket, guess, strlen(guess), 0);
            
            // primeste rezultatul de la oponent
            memset(buffer, 0, BUFFER_SIZE);
            read(client_socket, buffer, BUFFER_SIZE);
            
            // verifica daca am castigat
            if (strcmp(buffer, "WIN") == 0) {
                printf("Felicitări! Ai ghicit cuvântul: %s\n", guess);
                i_won = true;
                game_over = true;
            } else {
                // updateaza display_ul pentru progress
                letter_display = letter_compare(guess, opponent_word, letter_display);
                
                // updateaza istoricul de ghiciri
                num_guesses++;
                guess_history = realloc(guess_history, num_guesses * sizeof(char*));
                guess_history[num_guesses - 1] = strdup(guess);
                
                // afisare
                printf("Rezultat: ");
                display_result_from_code(guess, buffer);
                
                lives--;
                if (lives <= 0) {
                    printf("Ai rămas fără încercări! Cuvântul era: %s\n", opponent_word);
                }
            }
            
            free(guess);
        }
        
        // verifica tura oponentului
        if (!game_over) {
            printf("\nAșteptând ca oponentul să ghicească...\n");
            
            // primeste guessul oponentului
            memset(buffer, 0, BUFFER_SIZE);
            read(client_socket, buffer, BUFFER_SIZE);
            
            printf("Oponentul a ghicit: %s\n", buffer);
            
            // verificam daca oponentul a ghicit
            if (strcmp(buffer, my_word) == 0) {
                printf("Oponentul a ghicit cuvântul tău!\n");
                opponent_won = true;
                game_over = true;
                send(client_socket, "WIN", 3, 0);
            } else {
                // trimite inapoi guessul
                char *result = codify_result(buffer, my_word);
                send(client_socket, result, strlen(result), 0);
                free(result);
                
                // afiseaza rezultatul
                printf("Rezultatul oponentului: ");
                display_colored_guess(buffer, my_word);
            }
        }
        
        // verifica daca e over
        if (lives <= 0 || i_won || opponent_won) {
            game_over = true;
        }
    }
    
    // Game over
    if (i_won && !opponent_won) {
        printf("Felicitări! Ai câștigat jocul!\n");
    } else if (!i_won && opponent_won) {
        printf("Ai pierdut jocul. Oponentul a ghicit cuvântul tău.\n");
    } else if (i_won && opponent_won) {
        printf("Joc egal! Ambii jucători au ghicit cuvântul.\n");
    } else {
        printf("Joc terminat! Nimeni nu a ghicit cuvântul.\n");
    }
    
    // Clean
    free(my_word);
    free(opponent_word);
    free(letter_display);
    for (int i = 0; i < num_guesses; i++) {
        free(guess_history[i]);
    }
    free(guess_history);
    
    close(client_socket);
    close(server_fd);
}

// pentru partea de client
void run_client_game(const char *server_ip) {
    int sock = connect_to_server(server_ip);
    if (sock < 0) {
        return;
    }
    
    char buffer[BUFFER_SIZE] = {0};
    
    // cerem un cuvant de ghicit
    char *my_word = NULL;
    do {
        if (my_word != NULL) {
            free(my_word);
        }
        printf("Alege un cuvânt de %d litere pentru oponent: ", MULTIPLAYER_WORD_LEN);
        my_word = malloc((MULTIPLAYER_WORD_LEN + 1) * sizeof(char));
        fgets(my_word, MULTIPLAYER_WORD_LEN + 2, stdin);
        
        // scoate newline
        size_t len = strlen(my_word);
        if (len > 0 && my_word[len - 1] == '\n') {
            my_word[len - 1] = '\0';
            len--;
        }
        
        if (len != MULTIPLAYER_WORD_LEN) {
            printf("Cuvântul trebuie să aibă exact %d litere!\n", MULTIPLAYER_WORD_LEN);
        }
    } while (strlen(my_word) != MULTIPLAYER_WORD_LEN);
    
    // citeste cuvantul introdus de host
    read(sock, buffer, BUFFER_SIZE);
    char *opponent_word = strdup(buffer);
    printf("Ai primit un cuvânt de %d litere pentru a ghici.\n", MULTIPLAYER_WORD_LEN);
    
    // trimite cuvantul
    send(sock, my_word, strlen(my_word), 0);
    
    // initializare
    int lives = MULTIPLAYER_LIVES;
    char *letter_display = malloc((MULTIPLAYER_WORD_LEN + 1) * sizeof(char));
    
    for (int i = 0; i < MULTIPLAYER_WORD_LEN; i++) {
        letter_display[i] = '_';
    }
    letter_display[MULTIPLAYER_WORD_LEN] = '\0';
    
    char **guess_history = NULL;
    int num_guesses = 0;
    bool game_over = false;
    bool i_won = false;
    bool opponent_won = false;
    
    memset(buffer, 0, BUFFER_SIZE);
    
    // Main game loop
    while (!game_over) {
        // asteptare
        printf("\nAșteptând ca oponentul să ghicească...\n");
        
        // primeste ghicirea oponentului
        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        
        printf("Oponentul a ghicit: %s\n", buffer);
        
        // verifica daca oponentul a ghicit
        if (strcmp(buffer, my_word) == 0) {
            printf("Oponentul a ghicit cuvântul tău!\n");
            opponent_won = true;
            game_over = true;
            send(sock, "WIN", 3, 0);
        } else {
            // trimite inapoi rezultatul
            char *result = codify_result(buffer, my_word);
            send(sock, result, strlen(result), 0);
            free(result);
            
            // afiseaza
            printf("Rezultatul oponentului: ");
            display_colored_guess(buffer, my_word);
        }
        
        // randul clientului
        if (!game_over && lives > 0) {
            printf("\nEste rândul tău. Mai ai %d încercări.\n", lives);
            printf("Cuvântul are %d litere: ", MULTIPLAYER_WORD_LEN);
            for (int i = 0; i < MULTIPLAYER_WORD_LEN; i++) {
                printf("%c ", letter_display[i]);
            }
            printf("\n");
            
            // introdu guess
            char *guess = read_word();
            if (guess == NULL || strlen(guess) != MULTIPLAYER_WORD_LEN) {
                printf("Cuvantul introdus nu este valid. Trebuie să aibă %d litere.\n", MULTIPLAYER_WORD_LEN);
                if (guess != NULL) free(guess);
                continue;
            }
            
            // trimite la host
            send(sock, guess, strlen(guess), 0);
            
            // primeste de la host
            memset(buffer, 0, BUFFER_SIZE);
            read(sock, buffer, BUFFER_SIZE);
            
            // verifica daca am castigat
            if (strcmp(buffer, "WIN") == 0) {
                printf("Felicitări! Ai ghicit cuvântul: %s\n", guess);
                i_won = true;
                game_over = true;
            } else {
                // updateaza displayul
                letter_display = letter_compare(guess, opponent_word, letter_display);
                
                // updateaza guess history
                num_guesses++;
                guess_history = realloc(guess_history, num_guesses * sizeof(char*));
                guess_history[num_guesses - 1] = strdup(guess);
                
                // afiseaza
                printf("Rezultat: ");
                display_result_from_code(guess, buffer);
                
                lives--;
                if (lives <= 0) {
                    printf("Ai rămas fără încercări! Cuvântul era: %s\n", opponent_word);
                }
            }
            
            free(guess);
        }
        
        // verifica game over
        if (lives <= 0 || i_won || opponent_won) {
            game_over = true;
        }
    }
    
    // Game over
    if (i_won && !opponent_won) {
        printf("Felicitări! Ai câștigat jocul!\n");
    } else if (!i_won && opponent_won) {
        printf("Ai pierdut jocul. Oponentul a ghicit cuvântul tău.\n");
    } else if (i_won && opponent_won) {
        printf("Joc egal! Ambii jucători au ghicit cuvântul.\n");
    } else {
        printf("Joc terminat! Nimeni nu a ghicit cuvântul.\n");
    }
    
    // Clean
    free(my_word);
    free(opponent_word);
    free(letter_display);
    for (int i = 0; i < num_guesses; i++) {
        free(guess_history[i]);
    }
    free(guess_history);
    
    close(sock);
}

void play_singleplayer(const char *filename) {
    int difficulty = 0;
    int incercari = 0;
    
    printf("Alegeti dificultatea:\n");
    printf("Cod | Dificultate | Incercari | Dimensiune\n");
    printf("(1) | Usor        | Infinit   | 2-4 litere\n");
    printf("(2) | Mediu       | 6         | 5 litere\n");
    printf("(3) | Greu        | 3         | 5-44 litere\n");

start:
    scanf("%d", &difficulty);
    clear_input_buffer();
    
    switch (difficulty)
    {
        case 1:
            printf("Ati ales dificultatea usor.\n");
            incercari = 999;
            break;
        case 2:
            printf("Ati ales dificultatea mediu.\n");
            incercari = 6;
            break;
        case 3:
            printf("Ati ales dificultatea greu.\n");
            incercari = 3;
            break;
        default:
        printf("Dificultatea aleasa nu exista.\n");
        goto start;
}

char *guess_word = get_random_word(filename, difficulty);

if (guess_word == NULL) {
    printf("Nu s-a putut obține un cuvânt. Verificați fișierul %s.\n", filename);
    return;
}

int len = strlen(guess_word);
char *letter_display = malloc((len + 1) * sizeof(char));

for (int i = 0; i < len; i++) letter_display[i] = '_';
letter_display[len] = '\0';

printf("Cuvantul are %d litere.\n", len);

char **guess_history = NULL;
int num_guesses = 0;

while(guess_the_word(guess_word, &letter_display, &incercari, &guess_history, &num_guesses) == false && incercari > 0)
{
    printf("Mai ai %d incercari.\n", incercari);
}

if(verificare_castig(guess_word, letter_display) == true) printf("Felicitari, ai castigat!\n");
else printf("Ai pierdut! Cuvantul era: %s\n", guess_word);

free(letter_display);
free(guess_word);

for(int i = 0; i < num_guesses; i++)
{
    free(guess_history[i]);
}
free(guess_history);
}

int main(void)
{
printf("Program inițializat cu succes:\n");

// initializam randomizatorul
srand(time(NULL));

// alegem modul de joc
int game_mode = 0;
printf("Alege modul de joc:\n");
printf("(1) Singleplayer\n");
printf("(2) Multiplayer - Host\n");
printf("(3) Multiplayer - Client\n");

scanf("%d", &game_mode);
clear_input_buffer();

switch (game_mode) {
    case 1: {
        // singleplayer
        const char *filename = "words.txt";
        play_singleplayer(filename);
        break;
    }
    case 2: {
        // multiplayer host
        printf("Ai ales să fii host în modul multiplayer.\n");
        run_server_game();
        break;
    }
    case 3: {
        // multiplayer client
        printf("Ai ales să fii client în modul multiplayer.\n");
        char server_ip[16];
        printf("Introdu adresa IP a serverului: ");
        fgets(server_ip, 16, stdin);
        
        // scoate \n
        size_t len = strlen(server_ip);
        if (len > 0 && server_ip[len - 1] == '\n') {
            server_ip[len - 1] = '\0';
        }
        
        run_client_game(server_ip);
        break;
    }
    default:
        printf("Mod de joc invalid.\n");
        return 1;
}

return 0;
}
