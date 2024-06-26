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

#define COL_A 1 // forse non necessarie 
#define COL_B 2
#define COL_C 4
#define COL_D 8
#define COL_E 16
#define COL_F 32
#define COL_G 64
#define COL_H 128

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
extern struct position auxLastFreedCell;

// location
extern unsigned char whiteEnPassant;
extern unsigned char blackEnPassant;

// time
extern int wEnPassantMove;
extern int bEnPassantMove;

extern type_pezzo Board[8][8];
extern int turn;
extern unsigned int castle_privileges;
extern int game_status;
extern int numMoves;

/// @brief Inserisce nella lista l_head la posizione p in coda (da modificare, meglio inserimento in testa)
/// @param head puntatre alla lista nella quale inserire la posizione p
/// @param p posizione da inserire
/// @return ritorna l'elemento in testa della lista
struct position_list* insert(struct position_list *l_head, struct position p); //TODO metti aggiungi in testa

/// @brief Estrae dalla testa della lista head la prima posizione
/// @return posizione in testa alla lista
struct position pop();

/// @brief Rimuove tutte le posizioni presenti nella lista head
void flush_checkmate_aux_list();

/// @brief Valuta se la mossa descritta dai parametri è legale, e nel caso la compie sulla scacchiera. 
///        Ha l'effetto collaterale di aggiornare le variabili globali per le informazioni aggiuntive sulla scacchiera 
/// @param riga_i riga iniziale del pezzo da muovere
/// @param colonna_i colonna iniziale del pezzo da muovere
/// @param riga_f riga finale del pezzo da muovere
/// @param colonna_f colonna finale del pezzo da muovere
/// @return 1 se la mossa era legale ed è stata effettuata, 0 altrimenti
int move(int riga_i, int colonna_i, int riga_f, int colonna_f); // ritorna 1 se la mossa è legale, 0 altrimenti

/// @brief Stampa a video lo stato attuale della scacchiera
void print_chessboard(); //usa libreria da definirsi 

/// @brief Valuta se la posizione inExamPiece è sotto scacco da almeno un pezzo avversario.
/// @param inExamPiece posizione del pezzo da valutare
/// @param color colore del pezzo da valutare
/// @param ow se vale OVER_WRITE inserisce tutti i pezzi che vedono la posizione inExamPiece nella lista head
/// @return 1 se la casa è vista almeno da una pedina di colore !color, 0 altrimenti
int isItCheck(struct position inExamPiece, int color, int ow);

/// @brief valuta se la partita è finita per scacco matto
/// @return 1 se è scacco matto, 0 altrimenti
int checkMate(); // ritorna 1 se l'utima mossa ha generato matto, 0 se la partita può continuare

#endif