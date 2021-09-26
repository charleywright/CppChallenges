// Generates a minesweeper style board using width height and number of bombs. Random song: https://abstract.land/tidal/redirect/73349211

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#define WIDTH 30
#define HEIGHT 16
#define BOMBS 99

bool inBounds(short x, short y)
{
  return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

void InitializeBoard(short (&board)[HEIGHT][WIDTH])
{
  for (short i = 0; i < HEIGHT; i++)
    for (short j = 0; j < WIDTH; j++)
      board[i][j] = 0;
}

void GenerateBoard(short (&board)[HEIGHT][WIDTH])
{
  srand((unsigned)time(0));
  for (short i = 0; i < BOMBS; i++)
  {
    bool valid = false;
    short x, y;
    while (!valid)
    {
      x = rand() % WIDTH;
      y = rand() % HEIGHT;
      if (board[y][x] == 0)
        valid = true;
    }
    board[y][x] = -1;
  }

  for (short y = 0; y < HEIGHT; y++)
  {
    for (short x = 0; x < WIDTH; x++)
    {
      if (board[y][x] == -1)
        continue;

      if (inBounds(x - 1, y - 1) && board[y - 1][x - 1] == -1)
        board[y][x]++;
      if (inBounds(x, y - 1) && board[y - 1][x] == -1)
        board[y][x]++;
      if (inBounds(x + 1, y - 1) && board[y - 1][x + 1] == -1)
        board[y][x]++;
      if (inBounds(x - 1, y) && board[y][x - 1] == -1)
        board[y][x]++;

      if (inBounds(x + 1, y) && board[y][x + 1] == -1)
        board[y][x]++;
      if (inBounds(x - 1, y + 1) && board[y + 1][x - 1] == -1)
        board[y][x]++;
      if (inBounds(x, y + 1) && board[y + 1][x] == -1)
        board[y][x]++;
      if (inBounds(x + 1, y + 1) && board[y + 1][x + 1] == -1)
        board[y][x]++;
    }
  }
}

void ShowBoard(short (&board)[HEIGHT][WIDTH])
{
  for (short i = 0; i < HEIGHT; i++)
  {
    for (short j = 0; j < WIDTH; j++)
    {
      switch (board[i][j])
      {
      case -1:
        printf("X ");
        break;
      default:
        printf("%d ", board[i][j]);
      }
    }
    std::cout << std::endl;
  }
}

int main()
{
  short board[HEIGHT][WIDTH];
  InitializeBoard(board);
  GenerateBoard(board);
  ShowBoard(board);
  return 0;
}