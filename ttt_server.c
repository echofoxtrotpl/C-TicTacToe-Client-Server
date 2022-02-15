#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define WIN 4
#define LOSS -4
#define DRAW -2

int evaluate(char board[3][3])
{
  for (int x = 0; x < 3; x++)
  {
    // check row
    if ((board[x][0] != ' ') && (board[x][0] == board[x][1]) && (board[x][0] == board[x][2]))
      return (board[x][0] == 'x' ? WIN : LOSS);

    // check col
    if ((board[0][x] != ' ') && (board[0][x] == board[1][x]) && (board[0][x] == board[2][x]))
      return (board[0][x] == 'x' ? WIN : LOSS);
  };

  // check '\' direction
  if ((board[0][0] != ' ') && (board[0][0] == board[1][1]) && (board[0][0] == board[2][2]))
    return (board[0][0] == 'x' ? WIN : LOSS);

  // check '/' direction
  if ((board[2][0] != ' ') && (board[2][0] == board[1][1]) && (board[1][1] == board[0][2]))
    return (board[2][0] == 'x' ? WIN : LOSS);

  return DRAW;
}

int minimax(char board[3][3], int depth, char player)
{
  int result = evaluate(board);

  // possible end of recursion
  if (result == WIN)
    return WIN;
  if (result == LOSS)
    return LOSS;
  if (depth == 0)
    return 0;

  if (player == 'x')
  {
    int max_val = -5;
    int value;
    int no_move = 1;

    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        if (board[i][j] == ' ')
        {
          board[i][j] = 'x';
          value = minimax(board, depth - 1, 'o');
          board[i][j] = ' ';
          if (value > max_val)
            max_val = value;
          no_move = 0;
        }
      }
    }

    if (no_move)
      return 0;

    return max_val;
  }
  else
  {
    int min_val = 5;
    int value;
    int no_move = 1;
    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        if (board[i][j] == ' ')
        {
          board[i][j] = 'o';
          value = minimax(board, depth - 1, 'x');
          board[i][j] = ' ';
          if (value < min_val)
            min_val = value;
          no_move = 0;
        }
      }
    }

    if (no_move)
      return 0;

    return min_val;
  }
}

void move(char board[3][3], int depth, char player, int *best_i, int *best_j)
{
  int value;
  int max_val = -5;

  if (player == 'x')
  {
    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        if (board[i][j] == ' ')
        {
          board[i][j] = 'x';
          value = minimax(board, depth - 1, 'o');
          board[i][j] = ' ';
          if (value > max_val)
          {
            max_val = value;
            *best_i = i;
            *best_j = j;
          }
        }
      }
    }
  }
}

int main(void)
{
  int status, socket1, socket2;
  struct sockaddr_in srv, cli;
  int buf[2];
  int i = 0;
  char board[3][3] = {"   ", "   ", "   "};
  int result;
  int best_i, best_j;

  socket1 = socket(AF_INET, SOCK_STREAM, 0);
  if (socket1 == -1)
  {
    printf("Socket error!\n");
    return 0;
  }

  srv.sin_family = AF_INET;
  srv.sin_port = htons(9000);
  srv.sin_addr.s_addr = inet_addr("127.0.0.1");

  status = bind(socket1, (struct sockaddr *)&srv, sizeof srv);
  if (status == -1)
  {
    printf("Bind error!\n");
    return 0;
  }

  status = listen(socket1, 10);
  if (status == -1)
  {
    printf("Listen error\n");
    return 0;
  }

  int length = sizeof cli;
  socket2 = accept(socket1, (struct sockaddr *)&cli, (socklen_t *)&length);
  if (socket2 == -1)
  {
    printf("Accept error!\n");
    return 0;
  }

  // game start
  while (i < 5)
  {
    // computer move, 'x' is computer
    move(board, 9, 'x', &best_i, &best_j);

    board[best_i][best_j] = 'x';

    // send updated board to client
    write(socket2, board, sizeof board);

    // evaluate board
    result = evaluate(board);
    buf[0] = result;
    write(socket2, buf, sizeof buf);
    if (result == WIN || result == LOSS)
      break;

    // i == 4 is last move
    if (i == 4 && result == DRAW)
      break;

    // read client move and perform it
    status = read(socket2, buf, sizeof buf);
    board[buf[0]][buf[1]] = 'o';

    // send updated board to client
    write(socket2, board, sizeof board);

    // evaluate board
    result = evaluate(board);
    buf[0] = result;
    write(socket2, buf, sizeof buf);

    if (result == WIN || result == LOSS)
      break;
    if (i == 4 && result == DRAW)
      break;

    ++i;
  }
  close(socket1);
  close(socket2);

  return 0;
}
