#include "game.h"
#include "input-output.h"
#include <stdlib.h>

int main()
{
    struct position mov[2];
    searchInsert(&bPieces, BLACK);
    searchInsert(&wPieces, WHITE);
    generateMoves(&legalMoves, WHITE); // genero le mosse iniziali
    do {
        searchInsert(&bPieces, BLACK); // ottimizza rimuovendo quando stampi
        searchInsert(&wPieces, WHITE);
        system("clear");
        print_chessboard();
        if(turn == WHITE)
            printf("White turn\n-& ");
        else
            printf("Black turn\n-& ");
        // "1 <= row <= 8, a <= col <= h"
        inputMove(mov);
        if(move(mov[0].r, mov[0].c, mov[1].r, mov[1].c))
        {
            int promoted = promotion(turn);
            flush_move_list(&legalMoves);
            if(turn == WHITE) // gestione promozione
            {
                if(promoted)
                {
                    type_pezzo newPiece;
                    while(!isWhitePiece(newPiece = inputPiece())){
                        printf("Colore sbagliato!\n-& ");
                    }
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
                    while(!isBlackPiece(newPiece = inputPiece())){
                        printf("Colore sbagliato!\n-& ");
                    }
                    Board[0][promoted] = newPiece;
                }
                if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                {
                    printf("Scacco!\n");
                    system("sleep 1");
                    game_status = checkMate();
                }
            }
            int gameEnded = generateMoves(&legalMoves, !turn); //genera le mosse possibili del colore successivo
            if(game_status == CHECK_MATE)
            {
                system("clear");
                print_chessboard();
                printf("Scacco Matto!\n");
                break;
            }
            if(gameEnded == 0) 
            {
                system("clear");
                print_chessboard();
                printf("Patta per stallo!\n");
                break;
            }
            if(unActiveMoves >= 100)
            {
                system("clear");
                print_chessboard();
                printf("Patta per la regola delle 50 mosse!\n");
                break;
            }
            if(isItThreefoldRepetition())
            {
                system("clear");
                print_chessboard();
                printf("Patta per ripetizione!\n");
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

