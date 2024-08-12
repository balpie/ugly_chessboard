#ifndef BOARD_H
#define BOARD_H

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

#define COL_A 1 // forse non necessarie 
#define COL_B 2
#define COL_C 4
#define COL_D 8
#define COL_E 16
#define COL_F 32
#define COL_G 64
#define COL_H 128

#define WHITE_CASTLE_SHORT_PRIVILEGE 1
#define WHITE_CASTLE_LONG_PRIVILEGE 2
#define BLACK_CASTLE_SHORT_PRIVILEGE 4
#define BLACK_CASTLE_LONG_PRIVILEGE 8

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

struct move{ // necessaria per valutare la patta per stallo
    struct position init_p;
    struct position fin_p;
};

struct move_list{ // per memorizzare tutte le mosse possibili di un determinato colore
    struct move m;
    struct move_list* next;
};

struct board_list{ //per la patta per ripetizione
    type_pezzo board[8][8];
    struct board_list *next;

};

extern struct move_list* legalMoves; //mosse legali per un colore
extern struct position_list* attackers; //lista dei pezzi che mettono sotto scacco un determinato pezzo
extern struct position_list* wPieces;
extern struct position_list* bPieces;
extern struct board_list* boardList;
extern struct move_list* moves; // mosse legali per il colore del turno attuale
extern struct position wKingPosition;
extern struct position bKingPosition;
extern struct position lastPieceMoved;
extern struct position lastFreedCell;
extern struct position auxLastFreedCell;

// location
extern unsigned char whiteEnPassant;
extern unsigned char blackEnPassant;

// time
extern int wEnPassantMove;
extern int bEnPassantMove;

extern type_pezzo Board[8][8];
extern int turn;
extern unsigned short int castle_privileges; // maschera
extern int game_status;
extern int numMoves;
extern int unActiveMoves; // se = 100 patta per stallo (1 equivale a mezza mossa)

#endif