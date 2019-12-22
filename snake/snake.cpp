#include <iostream>
#include <../include/SDL2/SDL.h>
#include <../include/SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <random>
#include <ctime>

using namespace std;

#define fps 30

#define win_width 600
#define win_height 700
#define win_diff win_height-win_width

#define board_width 19  // amount of square on the window
#define square_size 28
#define square_gap 1
#define floor_init_pos 24

#define move_frequency 4

#define winning_condition square_size * square_size - 2

#define snake_init_pos floor_init_pos + (board_width / 2) * (square_size + square_gap) // middle

mt19937_64 mt_rand(time(nullptr));  // random seed


class Apple{
private:
    SDL_Rect apple{};
public:
    Apple(){
        apple.x = floor_init_pos + (int)(mt_rand() % board_width) * (square_size + square_gap) + 3;
        apple.y = floor_init_pos + win_diff + (int)(mt_rand() % board_width) * (square_size + square_gap) + 3;
        apple.w = square_size - 6;
        apple.h = square_size - 6;
    }

    void ResetState(){
        apple.x = floor_init_pos + (int)(mt_rand() % board_width) * (square_size + square_gap) + 3;
        apple.y = floor_init_pos + win_diff + (int)(mt_rand() % board_width) * (square_size + square_gap) + 3;
        apple.w = square_size - 6;
        apple.h = square_size - 6;
    }

    SDL_Rect* GetAppleAddress(){
        return &apple;
    }

    int getX(){
        return apple.x - 3;
    }
    int getY(){
        return apple.y - 3;
    }

};

class Snake{
private:
    /*
     *      ^           0
     *  <       >   3       1
     *      v           2
     */
    vector<SDL_Rect> snake;
    int face = 0;
    bool add = false;   // check if snake eat an apple to add one more block
    SDL_Rect new_rect{};    // for temporary add snake block
    int SnakeLength = 2;   // local var for adding new block

public:
    Snake(){
        snake.reserve(board_width * board_width);

        new_rect.x = snake_init_pos, new_rect.y = snake_init_pos + win_diff;
        new_rect.w = square_size, new_rect.h = square_size;
        snake.push_back(new_rect);

        new_rect.x = snake_init_pos, new_rect.y = snake_init_pos + win_diff + square_size + square_gap;
        new_rect.w = square_size, new_rect.h = square_size;
        snake.push_back(new_rect);
    }

    // -1 for left turn; +1 for right turn
    void turn(int dir){
        switch (dir){
            case -1:
                --face;
                if (face < 0) face = 3;
                break;
            case 1:
                ++face;
                if (face > 3) face = 0;
                break;
            default:
                break;
        }
    }

    void move(){
        if (add) {
            new_rect.x = (snake.end() - 1)->x, new_rect.y = (snake.end() - 1)->y;
            new_rect.w = square_size, new_rect.h = square_size;
            snake.push_back(new_rect);
            ++SnakeLength;
            add = false;
        }

        for (int i = SnakeLength-1; i > 0; --i) {
            snake[i].x = snake[i-1].x;
            snake[i].y = snake[i-1].y;
        }

        if (face == 0) snake[0].y -= square_size + square_gap;
        else if (face == 1) snake[0].x += square_size + square_gap;
        else if (face == 2) snake[0].y += square_size + square_gap;
        else if (face == 3) snake[0].x -= square_size + square_gap;
    }

    bool IsLose(){
        SnakeLength = snake.size();
        if (snake[0].x < floor_init_pos || snake[0].x > floor_init_pos + (board_width-1) * (square_size + square_gap)
           || snake[0].y < floor_init_pos + win_diff ||
           snake[0].y > floor_init_pos + win_diff + (board_width-1) * (square_size + square_gap))
            return true;
        for (int i = 1; i < SnakeLength; ++i) {
            if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
                return true;
        }
        return false;
    }

    void ResetState(){
        snake.clear();

        face = 0;
        new_rect.x = snake_init_pos, new_rect.y = snake_init_pos + win_diff;
        new_rect.w = square_size, new_rect.h = square_size;
        snake.push_back(new_rect);

        new_rect.x = snake_init_pos, new_rect.y = snake_init_pos + win_diff + square_size + square_gap;
        new_rect.w = square_size, new_rect.h = square_size;
        snake.push_back(new_rect);
    }

    void Add(){
        add = true;
    }

    vector<SDL_Rect> getSnake(){
        return snake;
    }

    bool operator == (Apple &apple){
        return (snake[0].x == apple.getX() && snake[0].y == apple.getY());
    }

    // Snake not in Apple
    bool operator != (Apple &apple){
        int x = apple.getX(), y = apple.getY();
        for (SDL_Rect rect : snake){
            if (rect.x == x && rect.y == y)
                return true;    // Snake in Apple
        }
        return false;
    }

};

void CreateNewGame(Snake &snake, Apple &apple){
    snake.ResetState();
    apple.ResetState();
}

bool SnakeAppleCollision(Snake &snake, Apple &apple){
    if (snake == apple) {
        snake.Add();
        do{
            apple.ResetState();
        } while (snake != apple);

        return true;
    }
    return false;
}



void InitEverything(SDL_Window *&win, SDL_Renderer *&renderer, TTF_Font *&font){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error(SDL_Init): %s\n", SDL_GetError());
    }

    win = SDL_CreateWindow("Welcome to Snake Game!",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            win_width, win_height, 0);

    if (win == nullptr)
        printf("Error(SDL_CreateWindow) : %s\n", SDL_GetError());

    if (TTF_Init() < 0)
        printf("Error(TTF_Init): %s\n", TTF_GetError());

    // init renderer
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
        printf("Error(SDL_Renderer): %s\n", SDL_GetError());
    // init font
    font = TTF_OpenFont("../asset/times.ttf", 50);
    if (!font)
        printf("Error(TTF_Font): %s\n", TTF_GetError());
}

void QuitWindow(SDL_Window *&win, SDL_Renderer *&renderer, TTF_Font *&font, SDL_Texture *&texture){
//    SDL_Delay(3000); // delay 3s
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    TTF_Quit();
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void FPS_Control(Uint32 starting_tick){
    if (1000/fps > SDL_GetTicks()-starting_tick)
        SDL_Delay(1000/fps - SDL_GetTicks() + starting_tick);
}

bool UpdateScreen(SDL_Renderer *&renderer, TTF_Font *&font, vector<SDL_Rect> &floors,
                  Snake &snake, Apple &apple, string &msg, SDL_Color msgColor,
                  SDL_Surface *&textSurface, SDL_Texture *&ScoreMsg, SDL_Rect &ScoreRect){
    // background
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderClear(renderer);
    // floors
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 250);
    for (SDL_Rect rect : floors)
        SDL_RenderFillRect(renderer, &rect);
    // snake
    SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
    for (SDL_Rect rect : snake.getSnake())
        SDL_RenderFillRect(renderer, &rect);
    // apple
    SDL_SetRenderDrawColor(renderer, 250, 5, 5, 255);
    SDL_RenderFillRect(renderer, apple.GetAppleAddress());
    // score
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderFillRect(renderer, &ScoreRect);

    textSurface = TTF_RenderText_Solid(font, msg.c_str(), msgColor);
    if (!textSurface) {
        printf("Error(SDL_Surface): %s ", SDL_GetError());
    }

    ScoreMsg = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!ScoreMsg) {
        printf("Error(SDL_Texture): %s ", SDL_GetError());
        return false;
    }

    SDL_FreeSurface(textSurface);

    SDL_QueryTexture(ScoreMsg, nullptr, nullptr, &ScoreRect.w, &ScoreRect.h);

    SDL_RenderCopy(renderer, ScoreMsg, nullptr, &ScoreRect);


    SDL_RenderPresent(renderer);
    return true;
}

int main(int argc, char* argv[]) {
    // init win
    SDL_Window *win = nullptr;
    SDL_Renderer *renderer = nullptr;
    TTF_Font *font = nullptr;
    SDL_Color scoreBoxColor = {144, 77, 25, 255};

    InitEverything(win, renderer, font);


    // init floors
    vector <SDL_Rect> floors;    // (10, 10) -> (790,790)
    floors.reserve(board_width * board_width);
    SDL_Rect floor;
    for (int i = 0; i < board_width*board_width; ++i){
        floor.x = floor_init_pos + (i % board_width) * (square_size + square_gap);
        floor.y = floor_init_pos + win_diff + (i / board_width) * (square_size + square_gap);
        floor.w = square_size, floor.h = square_size;
        floors.push_back(floor);
    }
    // init snake
    Snake snake;
    // init apple
    Apple apple;

    // init score box
    Uint32 score = 0;
    string scoreStr = "Your score: " + to_string(score);

    SDL_Rect ScoreRect{80, 50, 50, 50};

    SDL_Surface *message = nullptr;
    SDL_Texture *ScoreMsg = nullptr;


    // main loop
    Uint32 starting_tick;
    Uint8 CountMove = 0, CountTurn = 0;
    SDL_Event events;
    bool run = true;
    main_loop : while (run){
        starting_tick = SDL_GetTicks();

        // get user inputs
        while (SDL_PollEvent(&events)){
            if (events.type == SDL_QUIT){
                run = false;
                QuitWindow(win, renderer, font, ScoreMsg);
                return -99;
            }

            switch (events.type){
                case SDL_KEYDOWN:
                    switch (events.key.keysym.sym){
                        case SDLK_LEFT:
                            if (CountTurn > move_frequency) {
                                snake.turn(-1);
                                CountTurn = 0;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (CountTurn > move_frequency) {
                                snake.turn(1);
                                CountTurn = 0;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
//                case SDL_KEYUP:
//                    break;
                default:
                    break;
            }
        }


        if (CountMove > move_frequency){
            snake.move();
            CountMove = 0;
            (SnakeAppleCollision(snake, apple))?++score:0;
            scoreStr = "Your score: " + to_string(score);
        }

        // update background and limit fps
        ++CountMove, ++CountTurn;
        if (snake.IsLose() || score == winning_condition){
//            CreateNewGame(snake, apple);
//            score = 0;
            printf("Your final score: %d\n", score);
            run = false;
            QuitWindow(win, renderer, font, ScoreMsg);
            return score;
        }
        run = UpdateScreen(renderer, font, floors, snake, apple,
                scoreStr, scoreBoxColor, message, ScoreMsg, ScoreRect);
        FPS_Control(starting_tick);
    }

    return 0;
}
