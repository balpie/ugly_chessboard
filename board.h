#include <stdio.h>
#include <malloc.h>

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

#define OVER_WRITE 1 // per la lista dei pezzi che mettono sotto scaccco
#define NOT_OVER_WRITE 0

typedef char type_pezzo;

struct position
{
    int r;
    int c;
};

struct position_list{
    struct position init_p;
    struct position_list *next;
};

extern struct position_list* head; //lista dei pezzi che mettono sotto scacco un determinato pezzo
extern struct position wKingPosition;
extern struct position bKingPosition;
extern struct position lastPieceMoved;
extern struct position lastFreedCell;


extern type_pezzo Board[8][8];
extern int turn;
extern unsigned int castle_privileges;
extern int game_status;

struct position_list* insert(struct position_list *head, struct position p); //TODO metti aggiungi in testa
struct position pop();
void flush_checkmate_aux_list();
int move(int riga_i, int colonna_i, int riga_f, int colonna_f); // ritorna 1 se la mossa è legale, 0 altrimenti
void print_chessboard(); //usa libreria da definirsi 
int isItCheck(struct position inExamPiece, int color, int ow);
int checkMate(); // ritorna 1 se l'utima mossa ha generato matto, 0 se la partita può continuare

#endif
