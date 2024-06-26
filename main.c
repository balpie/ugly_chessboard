#include "board.h"
#include "input-output.h"
#include <stdlib.h>

void flushStdin() // fai header e source file per io
{
    scanf("%*[^\n]");
    scanf("%*c");
}

int main()
{
    char mov[4];
    do {
        system("clear");
        print_chessboard();
        if(turn == WHITE)
            printf("Sta al bianco: ");
        else
            printf("Sta al nero: ");
        // "1 <= row <= 8, a <= col <= h"
        int count = 0; char c;
        while (count < 4 && (c = getchar()) != '\n')
        {
            mov[count++] = c;
        }
        flushStdin();
        mov[0] -= 'a';
        mov[1] -= '1';
        mov[2] -= 'a';
        mov[3] -= '1';
        if(move(mov[1], mov[0], mov[3], mov[2]))
        {
            int promoted = promotion(turn);
            if(turn == WHITE)
            {
                if(promoted)
                {
                    type_pezzo newPiece;
                    newPiece = inputPiece(WHITE);
                    Board[7][promoted] = newPiece;
                }
                if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE)){
                    printf("Scacco!\n");
                    system("sleep 1");
                    game_status = checkMate();
                }
            }
            else
            {
                if(promoted)
                {
                    type_pezzo newPiece;
                    newPiece = inputPiece(BLACK);
                    Board[0][promoted] = newPiece;
                }
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
            turn = !turn; 
            system("clear");
            if(bEnPassantMove != -1)
            {
                if(bEnPassantMove == numMoves + 1)
                {
                    blackEnPassant = 0;
                    bEnPassantMove = -1;
                }
            }
            if(wEnPassantMove != -1)
            {
                if(bEnPassantMove == numMoves + 1)
                {
                    whiteEnPassant = 0;
                    wEnPassantMove = -1;
                }
            }
            numMoves++;
        }
    }while(1);
    return 0;
}

