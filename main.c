#include "board.h"
#include <stdlib.h>

int main()
{
    char mov[4];
    do {
        system("clear");
        print_chessboard();
        // "1 <= row <= 8, a <= col <= h"
        int count = 0; char c;
        while (count < 4 && (c = getchar()) != '\n')
        {
            mov[count++] = c;
        }
        mov[0] -= 'a';
        mov[1] -= '1';
        mov[2] -= 'a';
        mov[3] -= '1';
        if(move(mov[1], mov[0], mov[3], mov[2]))
        {
            if(turn == WHITE)
            {
                if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE)){
                    printf("Scacco!\n");
                    system("sleep 1");
                    game_status = checkMate();
                }
            }
            else
            {
                if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                {
                    printf("Scacco!\n");
                    system("sleep 1");
                    game_status = checkMate();
                }
            }
            if(game_status == CHECK_MATE)
            {
                system("clear");
                print_chessboard();
                printf("Scacco Matto!\n");
                break;
            }
            if(turn == WHITE)
                printf("Sta al bianco: ");
            else
                printf("Sta al nero: ");
                    turn = !turn; 
                }
        system("clear");
    }while(1);
    return 0;
}

