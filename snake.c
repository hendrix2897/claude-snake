#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 23
#define MAX_LENGTH 100

// Snake structure
typedef struct {
    int x[MAX_LENGTH];
    int y[MAX_LENGTH];
    int length;
    int direction;
} Snake;

// Directions
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

// Game state
typedef struct {
    Snake snake;
    int food_x;
    int food_y;
    int score;
    int game_over;
    int paused;
    time_t start_time;
    time_t pause_start_time;
    time_t total_pause_time;
} Game;

// Function prototypes
void init_game(Game *game);
void draw_game(Game *game);
void move_snake(Game *game);
void generate_food(Game *game);
void handle_input(Game *game);
void draw_pause_menu(Game *game);

// Initialize the game
void init_game(Game *game) {
    // Initialize snake
    game->snake.length = 1;
    game->snake.x[0] = WIDTH / 2;
    game->snake.y[0] = HEIGHT / 2;
    game->snake.direction = RIGHT;
    
    // Initialize game state
    game->score = 0;
    game->game_over = 0;
    game->paused = 0;
    game->start_time = time(NULL);
    game->pause_start_time = 0;
    game->total_pause_time = 0;
    
    // Seed random number generator
    srand(time(NULL));
    
    // Generate initial food
    generate_food(game);
}

// Generate food at random location
void generate_food(Game *game) {
    while (1) {
        game->food_x = rand() % (WIDTH - 2) + 1;
        game->food_y = rand() % (HEIGHT - 2) + 1;
        
        // Ensure food doesn't spawn on snake
        int collision = 0;
        for (int i = 0; i < game->snake.length; i++) {
            if (game->food_x == game->snake.x[i] && 
                game->food_y == game->snake.y[i]) {
                collision = 1;
                break;
            }
        }
        
        if (!collision) break;
    }
}

// Wrap-around logic for snake movement
void wrap_around(int *x, int *y) {
    if (*x <= 0) *x = WIDTH - 2;
    if (*x >= WIDTH - 1) *x = 1;
    if (*y <= 0) *y = HEIGHT - 2;
    if (*y >= HEIGHT - 1) *y = 1;
}

// Move the snake
void move_snake(Game *game) {
    // Move body segments
    for (int i = game->snake.length - 1; i > 0; i--) {
        game->snake.x[i] = game->snake.x[i-1];
        game->snake.y[i] = game->snake.y[i-1];
    }
    
    // Move head based on direction
    switch (game->snake.direction) {
        case UP:    game->snake.y[0]--; break;
        case DOWN:  game->snake.y[0]++; break;
        case LEFT:  game->snake.x[0]--; break;
        case RIGHT: game->snake.x[0]++; break;
    }
    
    // Wrap around walls
    wrap_around(&game->snake.x[0], &game->snake.y[0]);
    
    // Check for food eating
    if (game->snake.x[0] == game->food_x && 
        game->snake.y[0] == game->food_y) {
        // Grow snake
        if (game->snake.length < MAX_LENGTH) {
            game->snake.length++;
        }
        
        // Increase score
        game->score++;
        
        // Generate new food
        generate_food(game);
    }
}

// Draw the game
void draw_game(Game *game) {
    clear();
    
    // Draw border
    for (int x = 0; x < WIDTH; x++) {
        mvprintw(0, x, "#");
        mvprintw(HEIGHT-1, x, "#");
    }
    for (int y = 0; y < HEIGHT; y++) {
        mvprintw(y, 0, "#");
        mvprintw(y, WIDTH-1, "#");
    }
    
    // Draw snake
    for (int i = 0; i < game->snake.length; i++) {
        mvprintw(game->snake.y[i], game->snake.x[i], 
                 i == 0 ? "O" : "o");
    }
    
    // Draw food
    mvprintw(game->food_y, game->food_x, "*");
    
    // Draw score and play time
    time_t current_time = time(NULL);
    time_t play_time = current_time - game->start_time - game->total_pause_time;
    mvprintw(HEIGHT, 0, "Score: %d | Time: %ld seconds", game->score, play_time);
    
    refresh();
}

// Draw pause menu
void draw_pause_menu(Game *game) {
    time_t current_time = time(NULL);
    time_t play_time = current_time - game->start_time - game->total_pause_time;
    
    // Create a window for the pause menu
    WINDOW *pause_win = newwin(10, 40, HEIGHT/2 - 5, WIDTH/2 - 20);
    box(pause_win, 0, 0);
    
    // Pause menu title
    mvwprintw(pause_win, 1, 2, "GAME PAUSED");
    
    // Display current score
    mvwprintw(pause_win, 3, 2, "Current Score: %d", game->score);
    
    // Display play time
    mvwprintw(pause_win, 4, 2, "Play Time: %ld seconds", play_time);
    
    // Instruction to unpause
    mvwprintw(pause_win, 6, 2, "Press P to unpause");
    
    // Refresh the pause window
    wrefresh(pause_win);
}

// Handle self-collision
int check_self_collision(Game *game) {
    // Self collision
    for (int i = 1; i < game->snake.length; i++) {
        if (game->snake.x[0] == game->snake.x[i] && 
            game->snake.y[0] == game->snake.y[i]) {
            return 1;
        }
    }
    
    return 0;
}

// Handle user input
void handle_input(Game *game) {
    int ch = getch();
    
    switch (ch) {
        case 'p':
        case 'P':
            game->paused = !game->paused;
            if (game->paused) {
                // Record pause start time
                game->pause_start_time = time(NULL);
            } else {
                // Add paused duration to total pause time
                game->total_pause_time += time(NULL) - game->pause_start_time;
            }
            break;
        
        // Only process movement if not paused
        case KEY_UP:
            if (!game->paused && game->snake.direction != DOWN)
                game->snake.direction = UP;
            break;
        case KEY_DOWN:
            if (!game->paused && game->snake.direction != UP)
                game->snake.direction = DOWN;
            break;
        case KEY_LEFT:
            if (!game->paused && game->snake.direction != RIGHT)
                game->snake.direction = LEFT;
            break;
        case KEY_RIGHT:
            if (!game->paused && game->snake.direction != LEFT)
                game->snake.direction = RIGHT;
            break;
        case 'q':
        case 'Q':
            game->game_over = 1;
            break;
    }
}

// Main game loop
int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    
    // Initialize game
    Game game;
    init_game(&game);
    
    // Game loop
    while (!game.game_over) {
        // Handle input
        handle_input(&game);
        
        // Only update game if not paused
        if (!game.paused) {
            // Move snake
            move_snake(&game);
            
            // Check for self-collision
            if (check_self_collision(&game)) {
                game.game_over = 1;
            }
        }
        
        // Draw game
        draw_game(&game);
        
        // Draw pause menu if paused
        if (game.paused) {
            draw_pause_menu(&game);
        }
        
        // Delay to control game speed
        usleep(100000);  // 100ms delay
    }
    
    // Game over screen
    clear();
    mvprintw(HEIGHT/2, WIDTH/2 - 5, "Game Over!");
    mvprintw(HEIGHT/2 + 1, WIDTH/2 - 6, "Score: %d", game.score);
    refresh();
    
    // Wait for a key press
    nodelay(stdscr, FALSE);
    getch();
    
    // Clean up ncurses
    endwin();
    
    return 0;
}
