
#include <stdlib.h>    // For rand()
#include <time.h>      // For time()

#include "globals.h"
#include "misc.h"
#include "sudoku.h"

// ---------------------------------------------------------
// Local helpers for fast generation

// Fisher–Yates shuffle for integer arrays
static void shuffle_array(int* array, int length) {
    for (int i = length - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

// Find next empty cell (UNK). Returns linear index or -1 if none
static int find_empty_cell(int* arr) {
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            if (arr[COLS * y + x] == UNK)
                return COLS * y + x;
    return -1;
}

// Recursive randomized backtracking fill for a full valid grid
static int backtrack_fill(int* arr) {
    int idx = find_empty_cell(arr);
    if (idx == -1)
        return 1; // filled

    int numbers[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    shuffle_array(numbers, 9);

    for (int i = 0; i < 9; i++) {
        int n = numbers[i];
        if (valid_pos(arr, idx, n)) {
            arr[idx] = n;
            if (backtrack_fill(arr))
                return 1;
            arr[idx] = UNK;
        }
    }

    return 0;
}

/*
do {
    filled = 0;
    while (filled is less than difficulty)
        if (random num at random pos is valid)
            sudoku[random_pos] = random_num;
            filled++;
} while (generated sudoku cant be solved)

Where difficulty is the parameter and indicates the number of cells to be filled.
*/
void generate_sudoku(int difficulty) {
    // 1) Build a full valid solved grid via randomized backtracking
    init_grid(solved);
    if (!backtrack_fill(&solved[0][0])) {
        // Extremely unlikely; reset and try once more
        init_grid(solved);
        (void)backtrack_fill(&solved[0][0]);
    }

    // 2) Start visible grid from the solved grid
    copy_grid(&solved[0][0], &grid[0][0]);

    // 3) Remove cells to match requested number of clues (difficulty)
    int positions[ROWS * COLS];
    for (int i = 0; i < ROWS * COLS; i++)
        positions[i] = i;
    shuffle_array(positions, ROWS * COLS);

    int cells_to_remove = ROWS * COLS - difficulty;
    for (int i = 0; i < cells_to_remove; i++) {
        int idx = positions[i];
        int y = idx / COLS;
        int x = idx % COLS;
        grid[y][x] = UNK;
    }

    // The new sudoku has not been altered for now
    altered_sudoku = 0;
}

// Same as src/solver/sudoku.c
int valid_pos(int* arr, int idx, int num) {
    // Get x and y pos in the arr from idx
    int yp, xp;
    idx2yx(idx, &yp, &xp);

    // Check current row
    for (int x = 0; x < COLS; x++)
        if (arr[COLS * yp + x] == num)
            return 0;

    // Check current col
    for (int y = 0; y < ROWS; y++)
        if (arr[COLS * y + xp] == num)
            return 0;

    // Idx of cells corresponding to each box by y and x of the box
    static const int boxes[COLS][ROWS][BOXSZ] = {
        // First row of boxes (y1)
        {
          { 0, 1, 2, 9, 10, 11, 18, 19, 20 },     // y0x0
          { 3, 4, 5, 12, 13, 14, 21, 22, 23 },    // y0x1
          { 6, 7, 8, 15, 16, 17, 24, 25, 26 }     // y0x2
        },
        // Middle row of boxes (y2)
        {
          { 27, 28, 29, 36, 37, 38, 45, 46, 47 },    // y1x0
          { 30, 31, 32, 39, 40, 41, 48, 49, 50 },    // y1x1
          { 33, 34, 35, 42, 43, 44, 51, 52, 53 }     // y1x2
        },
        // Bottom row of boxes
        {
          { 54, 55, 56, 63, 64, 65, 72, 73, 74 },    // y2x0
          { 57, 58, 59, 66, 67, 68, 75, 76, 77 },    // y2x1
          { 60, 61, 62, 69, 70, 71, 78, 79, 80 }     // y2x2
        }
    };

    // Check current box
    for (int box_i = 0; box_i < BOXSZ; box_i++)
        /*
         * Divide x and y by 3 to get the box position (top-left, bottom-middle,
         * ...), use them to specify which box we want to check. We are iterating the
         * items inside that box to check if the number we want to check (num) is any
         * of the indexes that form that box.
         */
        if (arr[boxes[yp / 3][xp / 3][box_i]] == num)
            return 0;

    return 1;
}

/* idx2yx: converts idx to 1d array to x and y from 2d array */
void idx2yx(int idx, int* y, int* x) {
    *y = idx / COLS;
    *x = idx - *y * COLS;
}

/*
 * solve: Will try to solve input into output, return 1 if success, 0 if failure.
 * See src/solver/sudoku.c for comments.
 */
int solve(int* input, int* output) {
    int ucells[ROWS * COLS] = { 0 };
    int ucells_last         = 0;

    // First, copy input to output, then use output just like we used arr in
    // src/solver/sudoku.c
    copy_grid(input, output);

    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            if (output[COLS * y + x] == UNK)
                ucells[ucells_last++] = COLS * y + x;

    int found_valid = 0;

    for (int i = 0; i < ucells_last; i++) {
        found_valid = 0;

        int first_check = (output[ucells[i]] == UNK) ? 1 : output[ucells[i]];
        for (int n = first_check; n <= 9; n++) {
            if (valid_pos(output, ucells[i], n)) {
                found_valid = n;
                break;
            }
        }

        if (found_valid > 0) {
            output[ucells[i]] = found_valid;
        } else {
            // Can't find a valid number for the first unknown cell
            if (i < 1)
                return 0;

            output[ucells[i]] = UNK;

            i -= 2;
        }
    }

    return 1;
}

