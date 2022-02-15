#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define WIN 4
#define LOSS -4
#define DRAW -2

void print_board(char board[3][3])
{
  printf("  0 1 2\n");
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      if (j == 0)
        printf("%d ", i);
      printf("%c ", board[i][j]);
      if (j == 2)
        printf("\n");
    }
  }
  printf("\n");
}

int main(void)
{
  int status, socket1;
  struct sockaddr_in srv, cli;
  int buf[2];
  char board[3][3];

  // making connection to server
  socket1 = socket(AF_INET, SOCK_STREAM, 0);
  if (socket1 == -1)
  {
    printf("Socket error!\n");
    return 0;
  }

  srv.sin_family = AF_INET;
  srv.sin_port = htons(9000);
  srv.sin_addr.s_addr = inet_addr("127.0.0.1");

  status = connect(socket1, (struct sockaddr *)&srv, sizeof srv);
  if (status < 0)
  {
    printf("Connect error!\n");
    return 0;
  }

  int i = 0;

  while (i < 5)
  {
    // print updated board
    status = read(socket1, board, sizeof board);
    printf("\nOpponent move:\n\n");
    print_board(board);

    // check board evaluation
    read(socket1, buf, sizeof buf);
    if (buf[0] == WIN)
    {
      printf("X wins!\n");
      break;
    }
    if (buf[0] == LOSS)
    {
      printf("O wins!\n");
      break;
    }
    if (i == 4 && buf[0] == DRAW)
    {
      printf("Draw!\n");
      break;
    }

    // send move to server
    printf("Enter your move <row col>: ");
    scanf("%d %d", &buf[0], &buf[1]);
    write(socket1, buf, sizeof buf);

    // print updated board
    status = read(socket1, board, sizeof board);
    printf("\nYour move:\n\n");
    print_board(board);

    // check board evaluation
    read(socket1, buf, sizeof buf);
    if (buf[0] == WIN)
    {
      printf("X wins!\n");
      break;
    }
    if (buf[0] == LOSS)
    {
      printf("O wins!\n");
      break;
    }
    if (i == 4 && buf[0] == DRAW)
    {
      printf("Draw!\n");
      break;
    }

    ++i;
  }
  close(socket1);
  
  return 0;
}
