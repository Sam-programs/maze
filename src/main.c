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
};

#define ITER_DIR(name) for (int name = 1; name < 1 << 4; name <<= 1)

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

// Searches for 1 more step--adding a one block or backtracking one step--in the
// maze.
// `w`,`h` are the width and height of the maze respectively.
// `grid` is an array of size w * h. `hist` should also be an array of size w *
// h.
// `hist`,  `histi`, and `last_pos` are used to keep information between
// function calls.
//
// The maze is constructed in `grid` with each block being a bitmask
// of the directions of the blocks it's connected to.
void maze_dfs_once(Direction *grid, Direction *hist, int *histi, int *last_pos,
                   int w, int h) {
    int grid_end = w * h - 1;
    Direction available_dirs[10] = {0};
    int available_amount = 0;
    int new_pos;
    ITER_DIR(dir) {
        new_pos = *last_pos + dir2diff(dir, w);
        int new_x = new_pos / w;
        int new_y = new_pos % w;
        int last_x = *last_pos / w;
        int last_y = *last_pos % w;
        // The check below was added to stop the maze generator from teleporting
        // to other sides when touching an edge (e.g. right to left)
        if (!(new_x != last_x && new_y != last_y)) {
            if (new_pos >= 0 && new_pos <= grid_end && grid[new_pos] == 0) {
                available_dirs[available_amount++] = dir;
            }
        }
    }
    if (available_amount == 0) {
        if (*histi <= 0) {
            memset(grid, 0, w * h);
            *last_pos = 0;
            histi = 0;
            // `hist` is reset when `histi` is 0.
            return;
        }
        grid[*last_pos] |= none;
        *last_pos -= dir2diff(hist[--(*histi)], w);
        return;
    }
    Direction dir = available_dirs[rand() % available_amount];
    hist[(*histi)++] = dir;
    grid[*last_pos] |= dir;
    *last_pos += dir2diff(dir, w);
}

void render_maze(Direction *grid, int w, int h, int last_pos) {
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
            if (grid[i] & left) {
                render_x = x_off + (x * 2 - 2) * block_size;
                render_y = y_off + (y * 2) * block_size;
                DrawRectangle(render_x, render_y, block_size * 2, block_size,
                              YELLOW);
            }
            if (grid[i] & right) {
                render_x = x_off + (x * 2 + 1) * block_size;
                render_y = y_off + (y * 2) * block_size;
                DrawRectangle(render_x, render_y, block_size * 2, block_size,
                              YELLOW);
            }
            if (grid[i] & up) {
                render_x = x_off + (x * 2) * block_size;
                render_y = y_off + (y * 2 - 2) * block_size;
                DrawRectangle(render_x, render_y, block_size, block_size * 2,
                              YELLOW);
            }
            if (grid[i] & down) {
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
    srand(time(NULL));
    int histi = 0;
    int last_pos = 0;
    SetTraceLogLevel(LOG_NONE);
    InitWindow(0, 0, "Maze");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLUE);
        maze_dfs_once(maze, hist, &histi, &last_pos, w, h);
        usleep(1000 * 100);
        render_maze(maze, w, h, last_pos);
        EndDrawing();
    }
    free(maze);
}
