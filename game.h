#ifndef BOARD_HEADER
#define BOARD_HEADER
#include "board.h"
#include <stdio.h>
#include <malloc.h>


int colore_pezzo(type_pezzo p);
int isWhitePiece(type_pezzo p);
int isBlackPiece(type_pezzo p);

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

/// @brief controlla se c'è stata una promozione
/// @param color turno per cui controllare la promozione pendente
/// @return la colonna del pedone promosso se c'è stata una promozione, 0 altrimenti
int promotion(int color);

#endif