// Sudoku puzzle verifier and solver

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    int row;
    int col;
    int** grid;  // Add grid parameter
    int * result;
} parameters;

int PSIZE = 0;
int PSIZE_SQR = 0;


void * validateWorker(void *args) {
    
    parameters * para = (parameters *)args;

    int flag[PSIZE];

    for(int i = 0; i < PSIZE; i++)
    {
      flag[i] = 0;
    }
    int row, col;
    row = para->row;
    col = para->col;
    int** grid = para->grid;  // Retrieve the grid from parameters
    int * result = para->result; 

    //validate row
    if(row != -1 && col == -1){ 
        // printf("\n");
        int rindex = row -1;
        for(int j = 1; j <= PSIZE; j ++){
            // printf("%d ", grid[row][j]);
            if(grid[row][j] == 0)
            {
              result[rindex] = 2;
              break;
            }
            flag[grid[row][j] - 1] = 1;
        }

        if(result[rindex] != 2)
        {
          int k;
          for(k = 0; k < PSIZE; k ++){
            // printf("%d ", flag[k]);
            if(flag[k] != 1){
                break;
            }
          }
          //printf("k = %d\n", k);
          
          if(k >= PSIZE){
            result[rindex] = 1;
            //printf("row:%d  %d\n", row, result[rindex]);
          }
        }
    }else if(row == -1 && col != -1){ //validate col
        int rindex = PSIZE + col -1;
        for(int i = 1; i <= PSIZE; i ++)
        {
          if(grid[i][col] == 0)
          {
            result[rindex] = 2;
            break;
          }
          flag[grid[i][col] - 1] = 1;
        }

        if(result[rindex] != 2)
        {
          int k;
          for(k = 0; k < PSIZE; k ++)
          {
            if(flag[k] != 1)
            {
              break;
             }
          }

           if(k == PSIZE){
            result[rindex] = 1;
            //printf("col:%d  %d\n", col, result[rindex]);
          }
        }
    }else if(row != -1 && col != -1){ //grids

        int rindex = PSIZE * 2 + row -1 + (col -1) / PSIZE_SQR;
        //printf("validadte subgrid tindex:%d, startrow:%d, startcol:%d\n", rindex, row, col);
        for(int i = row; i < row + PSIZE_SQR; i ++)
            for(int j = col; j < col + PSIZE_SQR; j ++)
            {
              if(grid[i][j] == 0)
              {
                result[rindex] = 2;
                break;
              }
              flag[grid[i][j] - 1] = 1;
            }
        
        if(result[rindex] != 2)
        {
           int k;
           
           for(k = 0; k < PSIZE; k ++)
           {
            if(flag[k] != 1)
                break;
           }
           
           if(k == PSIZE){
            result[rindex] = 1;
            //printf("subgrid rindex:%d, result:%d\n", rindex, result[rindex]);
          }
        }
    }else{
        printf("Not validate parameters\n");
        exit(0);
    }

    return NULL;
}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  // YOUR CODE GOES HERE and in HELPER FUNCTIONS
  *valid = true;
  *complete = true;
  
  int threadsNum = psize * 3;
  PSIZE = psize;
  PSIZE_SQR = (int)sqrt(psize);

  
  //0 for invalid, 1 for valid, 2 for incmplete
  int results[threadsNum];
  
  for (int i = 0; i < threadsNum; i++) {
    results[i] = 0;
  }
  pthread_t tids[threadsNum];

  int i, j;
  
  //check rows
  for(i = 1; i <= psize; i ++){
    parameters * para = (parameters *)malloc(sizeof(parameters));
    // pthread_t tid;
    para->row = i;
    para->col = -1;
    para->grid = grid;  // Pass the grid to the thread
    para->result = results;
    pthread_create(&tids[i-1], NULL, &validateWorker, para);
  }

  //check colmuns
  for(j = 1; j <= psize; j ++){
    parameters * para = (parameters *)malloc(sizeof(parameters));
    // pthread_t tid;
    para->row = -1;
    para->col = j;
    para->grid = grid;  // Pass the grid to the thread
    para->result = results;
    pthread_create(&tids[psize + j -1], NULL, &validateWorker, para);
  }

  //check subgrid
  for(i = 1; i <= psize; i += PSIZE_SQR){
    for(j = 1; j <= psize; j += PSIZE_SQR){
      
      parameters * para = (parameters *)malloc(sizeof(parameters));
      // pthread_t tid;
      para->row = i;
      para->col = j;
      para->grid = grid;  // Pass the grid to the thread
      para->result = results;
      int tindex = psize * 2 + i -1 + (j -1) / PSIZE_SQR;
      //printf("subgrid tindex:%d, row:%d, col:%d\n", tindex, i, j);
      pthread_create(&tids[tindex], NULL, &validateWorker, para);
      }
   }

   for(i = 0; i < threadsNum; i ++){
        pthread_join(tids[i], NULL);
        //printf("results[%d] = %d\n", i, results[i]);
        
        if(results[i] == 2){
            *complete = false;
            *valid = false;
            break;
        }
        else if(results[i] == 0)
        {
          *valid = false;
          break;
        }
    }
}


// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;

  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");

  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
