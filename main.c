#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define xm 10
#define ym 20

struct current
{
    int block;
    int xP;
    int yP;

    char data[4][4];
    int w;
    int h;

    char field[ym][xm];
    int lost;
};

struct blox
{
    char data[4][4];
    int w;
    int h;
};

struct blox blocks[] =
{
    {{"XX",
      "XX"},
    2, 2},

    {{"####"},
    4, 1},

    {{"0  ",
      "000"},
    3, 2},

    {{"  $",
      "$$$"},
    3, 2},

    {{"?? ",
      " ??"},
    3, 2},

    {{" QQ",
      "QQ "},
    3, 2},

    {{" @ ",
      "@@@"},
    3, 2}
};

void clear_screen()
{
    printf("\e[1;1H\e[2J");
}

int borders(int newx, int newy, struct current *player)
{    
    // Top
    int flagT = 1;
    if (player->yP == 0) 
        for (size_t j = 0; j < player->w; j++)
            //if (player->field[0][j] != ' ')
                for (size_t i = 0; i < player->h; i++)
                if ((player->field[i][player->xP + j] != ' ') && (player->data[i][j] != ' '))
                    return 1;
    
    //Remove lines
    for (size_t i = 0; i < 20; i++)
    {
        int flagB = 1;
        for (size_t j = 0; j < 10; j++)
            if (player->field[i][j] == ' ')
                flagB = 0;
        
        if (flagB == 1)
            for (size_t k = i; k > 0; k--)
                for (size_t m = 0; m < 10; m++)
                    player->field[k][m] = player->field[k-1][m];    
    }

    // Formen
    int flagB = 0;

    for (size_t i = 0; i < player->h; i++)
        for (size_t j = 0; j < player->w; j++)
            if ((player->data[i][j] != ' ') && (player->field[player->yP + i + 1][player->xP + j] != ' '))
                flagB = 1;
    
    // Bottom
    if ((newy >= 21-player->h) || flagB)
    {
        for (size_t j = 0; j < player->h; j++)
            for (size_t i = 0; i < player->w; i++)
                if (player->data[j][i] != ' ')
                    player->field[j + player->yP][i + player->xP] = player->data[j][i];
        return -1;
    }

    // Left and Right
    else if (newx < 0)
        return 0;
    
    else if ((newx + player->w) >= 11)
        return 0;
    
    // Move
    else
    {
        player->xP = newx;
        player->yP = newy;
    }

    return 0;
}

void print_screen(struct current *player)
{
    char tmpfield[ym][xm];

    for (size_t b = 0; b < 20; b++)
        for (size_t a = 0; a < 10; a++)
            tmpfield[b][a] = player->field[b][a];

    for (size_t j = 0; j < player->h; j++)
        for (size_t i = 0; i < player->w; i++)
            if (player->data[j][i] != ' ')
                tmpfield[j + player->yP][i + player->xP] = player->data[j][i];

    clear_screen();
    for (size_t a = 0; a < 23; a++)
        printf("-");

    for (size_t b = 0; b < 20; b++)
    {
        printf("\n");
        printf("| ");
        for (size_t a = 0; a < 10; a++)
            printf("%c ", tmpfield[b][a]);
        printf("|");
    }

    printf("\n");
    for (size_t a = 0; a < 23; a++)
        printf("-");
    printf("\n");   
}

void turn(int dir, struct current *player)
{
    int w = player->w;
    int h = player->h;

    int new[4][4];

    if (dir == 1)
        for (size_t i = 0; i < w; i++)
            for (size_t j = 0, k = (h-1); j < h; j++, k--)
                new[i][k] = player->data[j][i];
    
    else if (dir == -1)
        for (size_t i = 0, k = (w-1); i < w; i++, k--)
            for (size_t j = 0; j < h; j++)
                new[k][j] = player->data[j][i];
    
    player->w = h;
    player->h = w;
    
    for (size_t i = 0; i < 4; i++)
        for (size_t j = 0; j < 4; j++)
            player->data[j][i] = new[j][i];
}

int main()
{
    // Game Settings
    struct current player;
    player.block = -1;
    player.lost = 0;
    for (size_t i = 0; i < 20; i++)
        for (size_t j = 0; j < 10; j++)
            player.field[i][j] = ' ';
    int ret = 0;
    int ms = 0;

    // random
    srand(time(NULL));
    rand();

    // IO settings
    int c;
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO | VEOF);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0)|O_NONBLOCK);

    // Speed Settings
    struct timespec last_t, current_t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &last_t);

    while (player.lost == 0)
    {
        if (ret == 1)
            return 0;

        else if (player.block == -1 || ret == -1)
        {
            ret = 0;
            player.block = rand() % 7;
            player.xP = 4;
            player.yP = 0;

            player.h = blocks[player.block].h;
            player.w = blocks[player.block].w;

            for (size_t i = 0; i < player.w; i++)
                for (size_t j = 0; j < player.h; j++)
                    player.data[j][i] = blocks[player.block].data[j][i];
        }
        
        clock_gettime(CLOCK_MONOTONIC_RAW, &current_t);
        long int delta_us = (current_t.tv_sec - last_t.tv_sec) * 1000000 + (current_t.tv_nsec - last_t.tv_nsec) / 1000;
        if (delta_us > 1000000/10)
        {
            ret = borders(player.xP, player.yP, &player);
            print_screen(&player);
            last_t = current_t;
            ms++;
        }
        if (ms >= 3)
        {
            ret = borders(player.xP, (player.yP + 1), &player);
            ms = 0;
        }

        while ((c = getchar()) > 0)
        {
            switch (c)
            {
                case 't':
                    player.lost = 1;
                    break;
                
                case 'a':
                    ret = borders((player.xP - 1), player.yP, &player);
                    break;

                case 'd':
                    ret = borders((player.xP + 1), player.yP, &player);
                    break;

                case 's':
                    ret = borders(player.xP, (player.yP + 1), &player);
                    break;

                case 'k':
                    turn(-1, &player);
                    break;
                
                case 'l':
                    turn(1, &player);
                    break;

                default:
                    break;
            }
        }
    }

    // Revoke IO Settings
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    return 0;
}