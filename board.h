#include <stdio.h>

#ifndef BOARD_HEADER
#define BOARD_HEADER

#define WHITE 0
#define BLACK 1

#define EMPTY ' '

#define W_KING 'K'
#define W_QUEEN 'Q'
#define W_ROOK 'R'
#define W_BISHOP 'B'
#define W_KNIGHT 'N'
#define W_PAWN 'P'

#define B_KING 'k'
#define B_QUEEN 'q'
#define B_ROOK 'r'
#define B_BISHOP 'b'
#define B_KNIGHT 'n'
#define B_PAWN 'p'

#define WHITE_CASTLE_PRIVILEGE 1
#define BLACK_CASTLE_PRIVILEGE 2

#define CHECK_MATE 1
#define DRAW -1
#define NOT_FINISHED 0
typedef char type_pezzo;

struct position
{
    int r;
    int c;
};

extern struct wKingPosition;
extern struct bKingPosition;
extern struct lastPieceMoved;

extern type_pezzo Board[8][8];
extern int turn;
extern unsigned int castle_privileges;
extern int game_status;

int move(int riga_i, int colonna_i, int riga_f, int colonna_f); // ritorna 1 se la mossa è legale, 0 altrimenti
void print_chessboard(); //usa libreria da definirsi 
int isItCheck(int color);
int checkMate(); // ritorna 1 se l'utima mossa ha generato matto, 0 se la partita può continuare

#endif
