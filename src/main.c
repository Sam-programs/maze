// TODO: maze solver
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// bitmask for the location a block is pointing to
enum {
    left = 1 << 0,
    right = 1 << 1,
    up = 1 << 2,
    down = 1 << 3,
    none = 1 << 4,
    DIREND = 1 << 5,
};

#define ITER_DIR(name) for (Direction name = 1; name < DIREND; name <<= 1)

typedef int8_t Direction;

// Returns the distance of 1 step in the direction `dir` for a 1-dimensional
// array. `w` is the maze width.
Direction dir2diff(Direction dir, int w) {
    switch (dir) {
        case left:
            return -1;
        case right:
            return 1;
        case up:
            return -w;
        case down:
            return w;
        default:
            return 0;
    }
}

// Returns the opposite direction of `dir` (left -> right, up -> down)
Direction invertdir(Direction dir) {
    switch (dir) {
        case left:
            return right;
        case right:
            return left;
        case up:
            return down;
        case down:
            return up;
        default:
            return none;
    }
}

// Searches for 1 more step--adding a one block or backtracking one step--in the
// maze.
// `w`,`h` are the width and height of the maze respectively.
// `maze`, and `hist` must be arrays of size `w * h`. 
// `maze`, hist`, `histi`, and `last_pos` are used to keep information between
// function calls.
//
// The maze is constructed in `maze`. Each block is a bitmask
// of the directions of the blocks that it's pointing to.
void maze_dfs_once(Direction *maze, Direction *hist, int *histi, int *last_pos,
                   int w, int h) {
    Direction available_dirs[4] = {0};
    int available_amount = 0;
    ITER_DIR(dir) {
        if (dir == none) {
            continue;
        }
        // The check below was added to stop the maze generator from teleporting
        // to other sides when touching an edge
        if ((dir == left && *last_pos % w == 0) ||
            ((dir == right) && *last_pos % w == (w - 1)) ||
            ((dir == up) && *last_pos < w) ||
            ((dir == down) && *last_pos >= w * (h - 1))) {
            continue;
        }
        int new_pos = *last_pos + dir2diff(dir, w);
        if (maze[new_pos] == 0) {
            available_dirs[available_amount++] = dir;
        }
    }
    if (available_amount == 0) {
        if (*histi <= 0) {
            return;
        }
        maze[*last_pos] |= none;
        *last_pos -= dir2diff(hist[--(*histi)], w);
        return;
    }
    Direction dir = available_dirs[rand() % available_amount];
    hist[(*histi)++] = dir;
    maze[*last_pos] |= dir;
    *last_pos += dir2diff(dir, w);
}

// Moves 1 step in the maze, and deletes dead ends until it reaches the bottom right corner.
// `w`,`h` are the width and height of the maze respectively.
// `maze`, and `hist` must be arrays of size `w * h`. 
// `maze`, and `player_pos` are used to keep information between function calls
void maze_solve_once(Direction *maze, int *player_pos, int w, int h) {
    int maze_end = w * h - 1;
    Direction available_dirs[4] = {0};
    int available_amount = 0;
    // Reset the maze when the player solves it
    if (*player_pos == maze_end) {
        *player_pos = 0;
        memset(maze, 0, w * h * sizeof(Direction));
        return;
    }

    ITER_DIR(dir) {
        if (maze[*player_pos] & dir) {
            available_dirs[available_amount++] = dir;
        }
    }
    if (available_amount > 0) {
        Direction dir = available_dirs[rand() % available_amount];
        *player_pos += dir2diff(dir, w);
    } else {
        // Scan all the blocks around us. If any of them are pointing to us,
        // mark their direction as an available direction.
        ITER_DIR(dir) {
            if (dir == none) {
                continue;
            }
            if ((dir == left && *player_pos % w == 0) ||
                ((dir == right) && *player_pos % w == (w - 1)) ||
                ((dir == up) && *player_pos < w) ||
                ((dir == down) && *player_pos >= w * (h - 1))) {
                continue;
            }
            int new_pos = *player_pos + dir2diff(dir, w);

            if (invertdir(dir) & maze[new_pos]) {
                available_dirs[available_amount++] = dir;
            }
        }
        Direction dir = available_dirs[rand() % available_amount];
        *player_pos += dir2diff(dir, w);
        maze[*player_pos] &= ~invertdir(dir); // remove dead paths
    }
}

// `maze` must be an array of size `w * h`.
void render_maze(Direction *maze, int w, int h, int last_pos) {
    int i = 0;
    const int screen_w = GetScreenWidth();
    const int screen_h = GetScreenHeight();
    int x_off = 0;
    int y_off = 0;
    int block_size;
    int w_blocks = w * 2 - 1;
    int h_blocks = h * 2 - 1;
    if (screen_w / w_blocks < screen_h / h_blocks) {
        block_size = screen_w / w_blocks;
    } else {
        block_size = screen_h / h_blocks;
    }
    x_off = (screen_w - block_size * w_blocks) / 2;
    y_off = (screen_h - block_size * h_blocks) / 2;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int render_x;
            int render_y;
            if (maze[i] & left) {
                render_x = x_off + (x * 2 - 2) * block_size;
                render_y = y_off + (y * 2) * block_size;
                DrawRectangle(render_x, render_y, block_size * 2, block_size,
                              YELLOW);
            }
            if (maze[i] & right) {
                render_x = x_off + (x * 2 + 1) * block_size;
                render_y = y_off + (y * 2) * block_size;
                DrawRectangle(render_x, render_y, block_size * 2, block_size,
                              YELLOW);
            }
            if (maze[i] & up) {
                render_x = x_off + (x * 2) * block_size;
                render_y = y_off + (y * 2 - 2) * block_size;
                DrawRectangle(render_x, render_y, block_size, block_size * 2,
                              YELLOW);
            }
            if (maze[i] & down) {
                render_x = x_off + (x * 2) * block_size;
                render_y = y_off + (y * 2 + 1) * block_size;
                DrawRectangle(render_x, render_y, block_size, block_size * 2,
                              YELLOW);
            }
            i += 1;
        }
    }
    int last_x = last_pos % w;
    int last_y = last_pos / w;
    DrawRectangle(x_off + last_x * block_size * 2,
                  y_off + last_y * block_size * 2, block_size, block_size, RED);
}

int main(void) {
    int w = 40;
    int h = 30;
    Direction *maze = calloc(w * h, 1);
    Direction *hist = calloc(w * h, 1);
    int histi = 0;
    int last_pos = 0;
    int player_pos = 0;

    srand(time(NULL));

    SetTraceLogLevel(LOG_NONE);
    InitWindow(0, 0, "Maze");

    int is_solving = false;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLUE);
        if (!is_solving) {
            usleep(1000 * 100);
            maze_dfs_once(maze, hist, &histi, &last_pos, w, h);
            is_solving = last_pos == 0;
        } else {
            usleep(1000 * 100);
            maze_solve_once(maze, &player_pos, w, h);
            is_solving = player_pos != 0;
        }
        render_maze(maze, w, h, is_solving ? player_pos : last_pos);
        EndDrawing();
    }
    free(maze);
    free(hist);
}
