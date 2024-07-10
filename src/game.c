#include "game.h"
//scacchiera base
type_pezzo Board[8][8] = {
    {W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_BISHOP, W_KNIGHT, W_ROOK},
    {W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {B_PAWN, B_PAWN, B_PAWN, B_PAWN, B_PAWN, B_PAWN, B_PAWN, B_PAWN},
    {B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_BISHOP, B_KNIGHT, B_ROOK},
};


int numMoves = 0;
unsigned char whiteEnPassant = 0;
unsigned char blackEnPassant = 0;
int wEnPassantMove = -1;
int bEnPassantMove = -1;

int turn = WHITE;
unsigned short int castle_privileges = WHITE_CASTLE_SHORT_PRIVILEGE | WHITE_CASTLE_LONG_PRIVILEGE | BLACK_CASTLE_SHORT_PRIVILEGE | BLACK_CASTLE_LONG_PRIVILEGE;
int game_status = NOT_FINISHED;

struct position_list *attackers= NULL;
struct move_list *legalMoves = NULL;
struct position_list* wPieces = NULL;
struct position_list* bPieces = NULL;

struct position wKingPosition = {.r = 0, .c = 4};
struct position bKingPosition = {.r = 7, .c = 4};
struct position lastPieceMoved = {.r = -1, .c = -1};
struct position lastFreedCell= {.r = -1, .c = -1};
struct position auxLastFreedCell = {.r = -1, .c = -1};


int colore_pezzo(type_pezzo p)
{
    if (p > 'Z' && p <= 'z') 
        return BLACK;
    else if (p <= 'Z' && p >= 'A')
        return WHITE;
    else 
        return EMPTY;
}

void insert_position(struct position_list **l_head, struct position p)
{
    if(l_head == NULL)
    {
        (*l_head) = (struct position_list*)malloc(sizeof(struct position_list));
        (*l_head)->next = NULL;
        (*l_head)->init_p = p;
        return;
    }
    struct position_list* p0 = (struct position_list*)malloc(sizeof(struct position_list));
    p0->next = *l_head;
    p0->init_p = p;
    *l_head = p0;
}

struct position pop_position(struct position_list **head) //forse si può fare di meglio? 
{
    struct position ret_value;
    if(*head == NULL)
    {
        ret_value.r = -1, ret_value.c = -1; // ritorno posizione fittizia
        return ret_value;
    }
    //rimozione in testa
    struct position_list *p0 = (*head)->next;
    ret_value = (*head)->init_p;
    free(*head);
    (*head) = p0;
    return ret_value;
}

void insert_move(struct move_list **head, struct move m)
{
    if (*head == NULL)
    {
        *head = (struct move_list*)malloc(sizeof(struct move_list));
        (*head)->next = NULL;
        (*head)->m = m;
        return;
    }
    struct move_list* p0 = (struct move_list*)malloc(sizeof(struct move_list));
    p0->next = *head;
    p0->m = m;
    *head = p0;
}

struct move pop_move(struct move_list **head)
{
    struct move ret_value;
    if(*head == NULL)
    {
        ret_value.init_p.r = ret_value.init_p.c = ret_value.fin_p.r = ret_value.fin_p.c = -1;
        return ret_value;
    }
    //rimozione in testa
    struct move_list *p0 = (*head)->next;
    ret_value = (*head)->m;
    free(*head);
    (*head) = p0;
    return ret_value;
}

void searchInsert(struct position_list **head, int color)
{
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            if(colore_pezzo(Board[i][j]) == color)
            {
                struct position aux = {.r = i, .c = j};
                insert_position(head, aux);
            }
        }
    }
}

void flush_position_list(struct position_list **head)
{
    while((*head)!= NULL)
    {
        pop_position(head);
    }
}

void flush_move_list(struct move_list **head)
{
    while (*head != NULL)
    {
        pop_move(head);
    }
}

int abs(int x)
{
    return (x < 0)? -x : x;
}

int isWhitePiece(type_pezzo p)
{
    return p < 'Z' && p >= 'A';
}

int isBlackPiece(type_pezzo p)
{
    return p > 'Z';
}

int checkInBound(int r, int c)
{
    if (r < 0 || r >= 8 || c < 0 || c >= 8)
    {
        return 0;
    }
    return 1;
}

int isValidKnightMove(int r_i, int c_i, int r_f, int c_f)
{
    if ((r_i == r_f + 1) || (r_i == r_f - 1))
    {
        if ((c_i == c_f - 2) || (c_i == c_f + 2))
        {
            return 1;
        }
    }
    else if((c_i == c_f + 1) || (c_i == c_f - 1))
    {
        if ((r_i == r_f - 2) || (r_i == r_f + 2))
        {
            return 1;
        }
    }
    return 0;
}

int goodDiagonal(int r_i, int c_i, int r_f, int c_f)
{
    return (abs(r_f - r_i) == abs(c_f - c_i));
}

int freeDiagonal(int r_i, int c_i, int r_f, int c_f)
{
    int ret = 1;
    if(abs(r_i - r_f) == 1) return 1; // è sicuramente libera perchè sto andando nel posto adiacente
    // assegno + 1 o -1 a degli accumulatori che andrò a sommare a i e j, 
    // in modo da percorrere la diagonale nel verso giusto
    int acc_i, acc_j;
    if (r_f > r_i) acc_i = 1;
    else acc_i = -1;

    if (c_f > c_i) acc_j = 1;
    else acc_j = -1;

    int i = r_i + acc_i, j = c_i + acc_j; 
    while((i != r_f) && (j != c_f)) // controllo fino alla penultima cella della diagonale
    {
        if(Board[i][j] != EMPTY)
        {
            ret = 0;
            break;
        }
        i += acc_i;
        j += acc_j;
    }
    return ret;
}

int goodLine(int r_i, int c_i, int r_f, int c_f)
{
    return ((r_i == r_f) || (c_i == c_f));
}

int freeLine(int r_i, int c_i, int r_f, int c_f)
{
    int acc, i;
    if ((abs(r_i - r_f) <= 1) && (abs(c_i - c_f) <= 1))
    {
        return 1; //linea sicuramente libera, posto adiacente
    }
    if (r_i == r_f)
    {
        acc = (c_f > c_i)? 1 : -1;
        i = c_i + acc;
        while(i != c_f)
        {
            if (Board[r_i][i] != EMPTY) 
            {
                return 0;
            }
            i += acc;
        }
    }
    else // c_i == c_f
    {
        acc = (r_f > r_i)? 1 : -1;
        i = r_i + acc;
        while(i != r_f)
        {
            if (Board[i][c_i] != EMPTY) 
            {
                return 0;
            }
            i += acc;
        }
    }
    return 1;
}

int isValidBishopMove(struct position pi, struct  position pf)
{
    if(goodDiagonal(pi.r, pi.c, pf.r, pf.c))
    {
        if (freeDiagonal(pi.r, pi.c, pf.r, pf.c))
            return 1;
        return 0;
    }
    return 0;
}

int isValidRookMove(struct position pi, struct position pf)
{
    if(goodLine(pi.r, pi.c, pf.r, pf.c))
    {
        if (freeLine(pi.r, pi.c, pf.r, pf.c))
            return 1;
        return 0;
    }
    return 0;
}

int isValidPawnMove(struct position pi, struct position pf)
{
    if (((pi.r == pf.r + 1) && (colore_pezzo(Board[pi.r][pi.c]) == BLACK)) || ((pi.r == pf.r - 1) && (colore_pezzo(Board[pi.r][pi.c]) == WHITE)))
    {
        if(pi.c == pf.c)
        {
            if (Board[pf.r][pf.c] == EMPTY)
            {
                return 1;
            }
            else return 0;
        }
        else if((pi.c == pf.c + 1) || (pi.c == pf.c - 1)) // caso in cui si mangia
        {
            if (checkInBound(pf.r, pf.c))
            {
                if((colore_pezzo(Board[pi.r][pi.c]) != colore_pezzo(Board[pf.r][pf.c])) && (Board[pf.r][pf.c] != EMPTY))
                {
                    return 1;
                } 
                else if((1<<pi.c) & ((colore_pezzo(Board[pi.r][pi.c] == WHITE) ? whiteEnPassant: blackEnPassant)))
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
        }
    }
    else if (((pi.r == 1) && (pf.r == 3) && (colore_pezzo(Board[pi.r][pi.c]) == WHITE)) || ((pi.r == 6) && (pf.r == 4) && (colore_pezzo(Board[pi.r][pi.c]) == BLACK)))
        return 1; 
    return 0;
}

int isValidMove(struct position pi, struct position pf)
{
    switch(Board[pi.r][pi.c])
    {
        case W_PAWN:
        case B_PAWN:
            return isValidPawnMove(pi, pf);
        case W_KNIGHT:
        case B_KNIGHT:
            return isValidKnightMove(pi.r, pi.c, pf.r, pf.c);
        case W_BISHOP:
        case B_BISHOP:
            return isValidBishopMove(pi, pf);
        case W_ROOK:
        case B_ROOK:
            return isValidRookMove(pi, pf);
        case W_QUEEN:
        case B_QUEEN:
            return (isValidBishopMove(pi, pf) || isValidRookMove(pi, pf));
        default:
            return 0;
    }
}


int generateMoves(struct move_list **head, int color)
{
    int moves = 0;
    struct position_list *p = (color == WHITE)? wPieces : bPieces;
    while(p != NULL)
    {
        struct position inExamPiece = p->init_p;
        struct position aux = inExamPiece;;
        switch(Board[p->init_p.r][p->init_p.c])
        {
            case W_PAWN:
                aux.r++;
                if(isValidMove(p->init_p, aux))
                {// provo mossa di pedone "standard"
                    type_pezzo auxp = Board[aux.r][aux.c];
                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                    if(!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {// se non mi metto sotto scacco la mossa è legale
                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                        insert_move(&legalMoves, auxm);
                        moves++;
                        Board[aux.r][aux.c] = auxp; //sfaccio la mossa
                        Board[p->init_p.r][p->init_p.c] = W_PAWN;    
                    }
                    Board[aux.r][aux.c] = auxp;
                    Board[p->init_p.r][p->init_p.c] = W_PAWN;    
                }
                aux.c--;
                if(isValidMove(p->init_p, aux))
                {// en-passant
                    type_pezzo auxp = Board[aux.r][aux.c];
                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                    Board[aux.r-1][aux.c] = EMPTY; // pedone mangiato
                    if(!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {// se non mi metto sotto scacco la mossa è legale
                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                        insert_move(&legalMoves, auxm);
                        Board[aux.r][aux.c] = auxp; //sfaccio la mossa
                        Board[aux.r-1][aux.c] = B_PAWN; 
                        Board[p->init_p.r][p->init_p.c] = W_PAWN;    
                        moves++;
                    }
                    Board[aux.r][aux.c] = auxp;
                    Board[aux.r-1][aux.c] = B_PAWN; 
                    Board[p->init_p.r][p->init_p.c] = W_PAWN;    
                }
                aux.c+=2;
                if(isValidMove(p->init_p, aux))
                {// en-passant
                    type_pezzo auxp = Board[aux.r][aux.c];
                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                    Board[aux.r-1][aux.c] = EMPTY; // pedone mangiato
                    if(!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {// se non mi metto sotto scacco la mossa è legale
                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                        insert_move(&legalMoves, auxm);
                        Board[aux.r][aux.c] = auxp; //sfaccio la mossa
                        Board[aux.r-1][aux.c] = B_PAWN; 
                        Board[p->init_p.r][p->init_p.c] = W_PAWN;    
                        moves++;
                    }
                    Board[aux.r][aux.c] = auxp;
                    Board[aux.r-1][aux.c] = B_PAWN; 
                    Board[p->init_p.r][p->init_p.c] = W_PAWN;    
                }
                aux.c--;
                aux.r++; //spinta di 2 case
                if(isValidMove(p->init_p, aux))
                {
                    type_pezzo auxp = Board[aux.r][aux.c];
                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                    if(!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {// se non mi metto sotto scacco la mossa è legale
                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                        insert_move(&legalMoves, auxm);
                        Board[aux.r][aux.c] = auxp; //sfaccio la mossa
                        Board[p->init_p.r][p->init_p.c] = W_PAWN;    
                        moves++;
                    }
                    Board[aux.r][aux.c] = auxp;
                    Board[p->init_p.r][p->init_p.c] = W_PAWN;    
                }
                break;
            case B_PAWN:
                aux.r++;
                if(isValidMove(p->init_p, aux))
                {// provo mossa di pedone "standard"
                    type_pezzo auxp = Board[aux.r][aux.c];
                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                    if(!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {// se non mi metto sotto scacco la mossa è legale
                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                        insert_move(&legalMoves, auxm);
                        Board[aux.r][aux.c] = auxp; //sfaccio la mossa
                        Board[p->init_p.r][p->init_p.c] = B_PAWN;    
                        moves++;
                    }
                    Board[aux.r][aux.c] = auxp;
                    Board[p->init_p.r][p->init_p.c] = B_PAWN;    
                }
                aux.c--;
                if(isValidMove(p->init_p, aux))
                {// en-passant
                    type_pezzo auxp = Board[aux.r][aux.c];
                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                    Board[aux.r-1][aux.c] = EMPTY; // pedone mangiato
                    if(!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {// se non mi metto sotto scacco la mossa è legale
                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                        insert_move(&legalMoves, auxm);
                        Board[aux.r][aux.c] = auxp; //sfaccio la mossa
                        Board[p->init_p.r][p->init_p.c] = B_PAWN;    
                        Board[aux.r-1][aux.c] = W_PAWN; 
                        moves++;
                    }
                    Board[aux.r][aux.c] = auxp;
                    Board[aux.r-1][aux.c] = W_PAWN; 
                    Board[p->init_p.r][p->init_p.c] = B_PAWN;    
                }
                aux.c+=2;
                if(isValidMove(p->init_p, aux))
                {// en-passant
                    type_pezzo auxp = Board[aux.r][aux.c];
                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                    Board[aux.r-1][aux.c] = EMPTY; // pedone mangiato
                    if(!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {// se non mi metto sotto scacco la mossa è legale
                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                        insert_move(&legalMoves, auxm);
                        Board[aux.r][aux.c] = auxp; //sfaccio la mossa
                        Board[p->init_p.r][p->init_p.c] = B_PAWN;    
                        Board[aux.r-1][aux.c] = W_PAWN; 
                        moves++;
                    }
                    Board[aux.r][aux.c] = auxp;
                    Board[aux.r-1][aux.c] = W_PAWN; 
                    Board[p->init_p.r][p->init_p.c] = B_PAWN;    
                }
                aux.c--;
                aux.r++; //spinta di 2 case
                if(isValidMove(p->init_p, aux))
                {
                    type_pezzo auxp = Board[aux.r][aux.c];
                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                    if(!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {// se non mi metto sotto scacco la mossa è legale
                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                        insert_move(&legalMoves, auxm);
                        Board[aux.r][aux.c] = auxp; //sfaccio la mossa
                        Board[p->init_p.r][p->init_p.c] = B_PAWN;    
                        moves++;
                    }
                    Board[aux.r][aux.c] = auxp;
                    Board[p->init_p.r][p->init_p.c] = B_PAWN;    
                }
                break;
            case W_BISHOP: 
                for(int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        aux = inExamPiece;
                        if((i != 0) && (j != 0))
                        {
                            aux.r += i; // proseguo sulla diagonale
                            aux.c += j;
                            while(checkInBound(aux.r, aux.c) && colore_pezzo(Board[inExamPiece.r][inExamPiece.c]) != colore_pezzo(Board[aux.r][aux.c])) 
                            {
                                if(isValidMove(inExamPiece, aux))
                                {
                                    type_pezzo auxp = Board[aux.r][aux.c];
                                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                                    if(!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                                    {// se non mi metto sotto scacco la mossa è legale
                                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                                        insert_move(&legalMoves, auxm);
                                        moves++;
                                    }
                                    Board[aux.r][aux.c] = auxp;
                                    Board[p->init_p.r][p->init_p.c] = W_BISHOP;    
                                }
                                aux.r += i; // proseguo sulla diagonale
                                aux.c += j;
                            }
                        }
                    }
                }
                break;
            case B_BISHOP: 
                for(int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        aux = inExamPiece;
                        if((i != 0) && (j != 0))
                        {
                            aux.r += i; // proseguo sulla diagonale
                            aux.c += j;
                            while(checkInBound(aux.r, aux.c) && colore_pezzo(Board[inExamPiece.r][inExamPiece.c]) != colore_pezzo(Board[aux.r][aux.c])) 
                            {
                                if(isValidMove(inExamPiece, aux))
                                {
                                    type_pezzo auxp = Board[aux.r][aux.c];
                                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                                    if(!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                                    {// se non mi metto sotto scacco la mossa è legale
                                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                                        insert_move(&legalMoves, auxm);
                                        moves++;
                                    }
                                    Board[aux.r][aux.c] = auxp;
                                    Board[p->init_p.r][p->init_p.c] = B_BISHOP;    
                                }
                                aux.r += i; // proseguo sulla diagonale
                                aux.c += j;
                            }
                        }
                    }
                }
                break;
            case W_ROOK: 
                for(int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        aux = inExamPiece;
                        if((i == 0) || (j == 0))
                        {
                            aux.r += i; // proseguo sulla diagonale
                            aux.c += j;
                            while(checkInBound(aux.r, aux.c) && colore_pezzo(Board[inExamPiece.r][inExamPiece.c]) != colore_pezzo(Board[aux.r][aux.c])) 
                            {
                                if(isValidMove(inExamPiece, aux))
                                {
                                    type_pezzo auxp = Board[aux.r][aux.c];
                                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                                    if(!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                                    {// se non mi metto sotto scacco la mossa è legale
                                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                                        insert_move(&legalMoves, auxm);
                                        moves++;
                                    }
                                    Board[aux.r][aux.c] = auxp;
                                    Board[p->init_p.r][p->init_p.c] = W_ROOK;    
                                }
                                aux.r += i; // proseguo sulla diagonale
                                aux.c += j;
                            }
                        }
                    }
                }
                break;
            case B_ROOK: 
                for(int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        aux = inExamPiece;
                        if((i == 0) || (j == 0))
                        {
                            aux.r += i; // proseguo sulla diagonale
                            aux.c += j;
                            while(checkInBound(aux.r, aux.c) && colore_pezzo(Board[inExamPiece.r][inExamPiece.c]) != colore_pezzo(Board[aux.r][aux.c])) 
                            {
                                if(isValidMove(inExamPiece, aux))
                                {
                                    type_pezzo auxp = Board[aux.r][aux.c];
                                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                                    if(!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                                    {// se non mi metto sotto scacco la mossa è legale
                                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                                        insert_move(&legalMoves, auxm);
                                        moves++;
                                    }
                                    Board[aux.r][aux.c] = auxp;
                                    Board[p->init_p.r][p->init_p.c] = B_ROOK;    
                                }
                                aux.r += i; // proseguo sulla diagonale
                                aux.c += j;
                            }
                        }
                    }
                }
                break;
            case W_QUEEN: 
                for(int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        aux = inExamPiece;
                        if(!((i == 0) && (j == 0)))
                        {
                            aux.r += i; // proseguo sulla diagonale
                            aux.c += j;
                            while(checkInBound(aux.r, aux.c) && colore_pezzo(Board[inExamPiece.r][inExamPiece.c]) != colore_pezzo(Board[aux.r][aux.c])) 
                            {
                                if(isValidMove(inExamPiece, aux))
                                {
                                    type_pezzo auxp = Board[aux.r][aux.c];
                                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                                    if(!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                                    {// se non mi metto sotto scacco la mossa è legale
                                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                                        insert_move(&legalMoves, auxm);
                                        moves++;
                                    }
                                    Board[aux.r][aux.c] = auxp;
                                    Board[p->init_p.r][p->init_p.c] = W_QUEEN;    
                                }
                                aux.r += i; // proseguo sulla diagonale
                                aux.c += j;
                            }
                        }
                    }
                }
                break;
            case B_QUEEN: 
                for(int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        aux = inExamPiece;
                        if(!((i == 0) && (j == 0)))
                        {
                            aux.r += i; // proseguo sulla diagonale
                            aux.c += j;
                            while(checkInBound(aux.r, aux.c) && colore_pezzo(Board[inExamPiece.r][inExamPiece.c]) != colore_pezzo(Board[aux.r][aux.c])) 
                            {
                                if(isValidMove(inExamPiece, aux))
                                {
                                    type_pezzo auxp = Board[aux.r][aux.c];
                                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                                    if(!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                                    {// se non mi metto sotto scacco la mossa è legale
                                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                                        insert_move(&legalMoves, auxm);
                                        moves++;
                                    }
                                    Board[aux.r][aux.c] = auxp;
                                    Board[p->init_p.r][p->init_p.c] = B_QUEEN;    
                                }
                                aux.r += i; // proseguo sulla diagonale
                                aux.c += j;
                            }
                        }
                    }
                }
                break;
            case W_KING: 
                for(int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        aux = inExamPiece;
                        if(!((i == 0) && (j == 0)))
                        {
                            aux.r += i; // proseguo sulla diagonale
                            aux.c += j;
                            if(checkInBound(aux.r, aux.c) && colore_pezzo(Board[inExamPiece.r][inExamPiece.c]) != colore_pezzo(Board[aux.r][aux.c])) 
                            {
                                if(isValidMove(inExamPiece, aux))
                                {
                                    type_pezzo auxp = Board[aux.r][aux.c];
                                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                                    if(!isItCheck(aux, WHITE, NOT_OVER_WRITE))
                                    {// se non mi metto sotto scacco la mossa è legale
                                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                                        insert_move(&legalMoves, auxm);
                                        moves++;
                                    }
                                    Board[aux.r][aux.c] = auxp;
                                    Board[p->init_p.r][p->init_p.c] = W_KING;    
                                }
                                aux.r += i; // proseguo sulla diagonale
                                aux.c += j;
                            }
                        }
                    }
                }
                break;
            case B_KING: 
                for(int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        aux = inExamPiece;
                        if(!((i == 0) && (j == 0)))
                        {
                            aux.r += i; // proseguo sulla diagonale
                            aux.c += j;
                            if(checkInBound(aux.r, aux.c) && colore_pezzo(Board[inExamPiece.r][inExamPiece.c]) != colore_pezzo(Board[aux.r][aux.c])) 
                            {
                                if(isValidMove(inExamPiece, aux))
                                {
                                    type_pezzo auxp = Board[aux.r][aux.c];
                                    Board[aux.r][aux.c] = Board[p->init_p.r][p->init_p.c];
                                    Board[p->init_p.r][p->init_p.c] = EMPTY;
                                    if(!isItCheck(aux, BLACK, NOT_OVER_WRITE))
                                    {// se non mi metto sotto scacco la mossa è legale
                                        struct move auxm = {.init_p = p->init_p, .fin_p = aux};
                                        insert_move(&legalMoves, auxm);
                                        moves++;
                                    }
                                    Board[aux.r][aux.c] = auxp;
                                    Board[p->init_p.r][p->init_p.c] = B_KING;    
                                }
                                aux.r += i; // proseguo sulla diagonale
                                aux.c += j;
                            }
                        }
                    }
                }
                break;
            
        }
        p = p->next;
    }
    return moves;
}

int move(int riga_i, int colonna_i, int riga_f, int colonna_f)
{
    if(!checkInBound(riga_i, colonna_i) || !checkInBound(riga_f, colonna_f)) 
    { //mossa sicuramente non valida
        return 0;
    }
    
    if (isWhitePiece(Board[riga_i][colonna_i]) && turn == BLACK)
    { //turno sbagliato
        return 0;
    }
    if (isBlackPiece(Board[riga_i][colonna_i]) && turn == WHITE)
    { //turno sbagliato
        return 0;
    }
    if((isWhitePiece(Board[riga_f][colonna_f]) && isWhitePiece(Board[riga_i][colonna_i])) || (isBlackPiece(Board[riga_f][colonna_f]) && isBlackPiece(Board[riga_i][colonna_i])))
        return 0; // se sto cercando di andare in una casa occupata da un pezzo dello stesso colore

    switch (Board[riga_i][colonna_i])
    {
    case W_PAWN:
        if (riga_i == riga_f - 1)
        {
            if((colonna_i == colonna_f + 1) || colonna_i == colonna_f - 1) // sto cercando di mangiare un pezzo
            {
                if(isBlackPiece(Board[riga_f][colonna_f]))
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = W_PAWN;
                    if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {
                        Board[riga_i][colonna_i] = W_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
                if((riga_i == 4) && (whiteEnPassant & (1<<colonna_i))) // en passant
                {
                    Board[lastPieceMoved.r][lastPieceMoved.c] = EMPTY;
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = W_PAWN;
                    if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {
                        Board[lastPieceMoved.r][lastPieceMoved.c] = B_PAWN;
                        Board[riga_i][colonna_i] = W_PAWN;
                        Board[riga_f][colonna_f] = EMPTY;
                        return 0;
                    }
                    auxLastFreedCell = lastPieceMoved; // pedone mangiato
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
                else 
                    return 0;
            }
            else if (colonna_i == colonna_f)
            {
                if(Board[riga_f][colonna_f] == EMPTY)
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = W_PAWN;
                    if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {
                        Board[riga_i][colonna_i] = W_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
                else 
                    return 0;
            }
            else 
                return 0;
        }
        else if (riga_i == riga_f - 2 )
        {
            if((colonna_f == colonna_i) && (riga_i == 1) && (Board[riga_i + 1][colonna_i] == EMPTY) && (Board[riga_i + 2][colonna_i] == EMPTY))
                { // Fixme: puoi sempre avanzare di 2 col pedone -> usa isValidPawnMove
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = W_PAWN;
                    if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {
                        Board[riga_i][colonna_i] = W_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    if(Board[riga_f][colonna_f + 1] == B_PAWN)
                    {
                        bEnPassantMove = numMoves;
                        blackEnPassant |= (1<<(colonna_f+1));
                    }
                    if(Board[riga_f][colonna_f - 1] == B_PAWN)
                    {
                        bEnPassantMove = numMoves;
                        blackEnPassant |= (1<<(colonna_f-1));
                    }
                    return 1;
                }
            else 
                return 0;
        }
        return 0;
        break;

    case B_PAWN:
        if (riga_i == riga_f + 1)
        {
            if((colonna_i == colonna_f + 1) || colonna_i == colonna_f - 1) // sto cercando di mangiare un pezzo
            {
                if(isWhitePiece(Board[riga_f][colonna_f]))
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = B_PAWN;
                    if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {
                        Board[riga_i][colonna_i] = B_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
                if((riga_i == 3) && (blackEnPassant & (1<<colonna_i))) // en passant
                {
                    Board[lastPieceMoved.r][lastPieceMoved.c] = EMPTY;
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = B_PAWN;
                    if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {
                        Board[lastPieceMoved.r][lastPieceMoved.c] = W_PAWN;
                        Board[riga_i][colonna_i] = B_PAWN;
                        Board[riga_f][colonna_f] = EMPTY;
                        return 0;
                    }
                    auxLastFreedCell = lastPieceMoved; // pedone mangiato
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
                else 
                    return 0;
            }
            else if (colonna_i == colonna_f)
            {
                if(Board[riga_f][colonna_f] == EMPTY)
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = B_PAWN;
                    if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {
                        Board[riga_i][colonna_i] = B_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
                else return 0;
            }
            else 
                return 0;
        }
        else if (riga_i == riga_f + 2 ) 
            if(colonna_f == colonna_i && riga_i == 6 && Board[riga_i - 1][colonna_i] == EMPTY && Board[riga_i - 2][colonna_i] == EMPTY)
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = B_PAWN;
                    if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {
                        Board[riga_i][colonna_i] = B_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    if(Board[riga_f][colonna_f + 1] == W_PAWN)
                    {
                        wEnPassantMove = numMoves;
                        whiteEnPassant |= (1<<(colonna_f+1));
                    }
                    if(Board[riga_f][colonna_f - 1] == W_PAWN)
                    {
                        wEnPassantMove = numMoves;
                        whiteEnPassant |= (1<<(colonna_f-1));
                    }
                    return 1;
                }
            else return 0;
        break;
        
    case W_KNIGHT: 
        if (isValidKnightMove(riga_i, colonna_i, riga_f, colonna_f))
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = W_KNIGHT;
            if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
            {
                Board[riga_i][colonna_i] = W_KNIGHT;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            lastFreedCell.r = riga_i; 
            lastFreedCell.c = colonna_i;
            lastPieceMoved.r = riga_f;
            lastPieceMoved.c = colonna_f;
            return 1;
        }
        else return 0;

    case B_KNIGHT:
        if (isValidKnightMove(riga_i, colonna_i, riga_f, colonna_f))
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = B_KNIGHT;
            if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
            {
                Board[riga_i][colonna_i] = B_KNIGHT;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            lastFreedCell.r = riga_i; 
            lastFreedCell.c = colonna_i;
            lastPieceMoved.r = riga_f;
            lastPieceMoved.c = colonna_f;
            return 1;
        }
        else return 0;
    case W_BISHOP:
        if (!goodDiagonal(riga_i, colonna_i, riga_f, colonna_f))
        {
            return 0; // mossa non valida
        }
        if (freeDiagonal(riga_i, colonna_i, riga_f, colonna_f))
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = W_BISHOP;
            if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
            {
                Board[riga_i][colonna_i] = W_BISHOP;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            lastFreedCell.r = riga_i; 
            lastFreedCell.c = colonna_i;
            lastPieceMoved.r = riga_f;
            lastPieceMoved.c = colonna_f;
            return 1;
        }
        return 0;
    case B_BISHOP:
        if (!goodDiagonal(riga_i, colonna_i, riga_f, colonna_f))
        {
            return 0; // mossa non valida
        }
        if (freeDiagonal(riga_i, colonna_i, riga_f, colonna_f))
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = B_BISHOP;
            if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
            {
                Board[riga_i][colonna_i] = B_BISHOP;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            lastFreedCell.r = riga_i; 
            lastFreedCell.c = colonna_i;
            lastPieceMoved.r = riga_f;
            lastPieceMoved.c = colonna_f;
            return 1;
        }
        return 0;
    case W_ROOK:
        if (!goodLine(riga_i, colonna_i, riga_f, colonna_f))
        {
            return 0;
        }
        if (freeLine(riga_i, colonna_i, riga_f, colonna_f))
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = W_ROOK;
            if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
            {
                Board[riga_i][colonna_i] = W_ROOK;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            lastFreedCell.r = riga_i; 
            lastFreedCell.c = colonna_i;
            lastPieceMoved.r = riga_f;
            lastPieceMoved.c = colonna_f;
            castle_privileges &= (~((colonna_i == 7)? WHITE_CASTLE_SHORT_PRIVILEGE : WHITE_CASTLE_LONG_PRIVILEGE));
            return 1;
        }
        return 0;
    case B_ROOK:
        if (!goodLine(riga_i, colonna_i, riga_f, colonna_f))
        {
            return 0;
        }
        if (freeLine(riga_i, colonna_i, riga_f, colonna_f))
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = B_ROOK;
            if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
            {
                Board[riga_i][colonna_i] = B_ROOK;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            lastFreedCell.r = riga_i; 
            lastFreedCell.c = colonna_i;
            lastPieceMoved.r = riga_f;
            lastPieceMoved.c = colonna_f;
            castle_privileges &= (~((colonna_i == 7)? BLACK_CASTLE_SHORT_PRIVILEGE : BLACK_CASTLE_LONG_PRIVILEGE));
            return 1;
        }
        return 0;
    case W_QUEEN:
        if (goodDiagonal(riga_i, colonna_i, riga_f, colonna_f))
        {
            if (freeDiagonal(riga_i, colonna_i, riga_f, colonna_f))
            {
                type_pezzo aux = Board[riga_f][colonna_f];
                Board[riga_i][colonna_i] = EMPTY;
                Board[riga_f][colonna_f] = W_QUEEN;
                if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                {
                    Board[riga_i][colonna_i] = W_QUEEN;
                    Board[riga_f][colonna_f] = aux;
                    return 0;
                }
                lastFreedCell.r = riga_i; 
                lastFreedCell.c = colonna_i;
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                return 1;
            }
        }
        else 
        {
            if (goodLine(riga_i, colonna_i, riga_f, colonna_f))
            {
                if (freeLine(riga_i, colonna_i, riga_f, colonna_f))
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = W_QUEEN;
                    if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {
                        Board[riga_i][colonna_i] = W_QUEEN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastFreedCell.r = riga_i; 
                    lastFreedCell.c = colonna_i;
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
            }
        }
        return 0;
    case B_QUEEN:
        if (goodDiagonal(riga_i, colonna_i, riga_f, colonna_f))
        {
            if (freeDiagonal(riga_i, colonna_i, riga_f, colonna_f))
            {
                type_pezzo aux = Board[riga_f][colonna_f];
                Board[riga_i][colonna_i] = EMPTY;
                Board[riga_f][colonna_f] = B_QUEEN;
                if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                {
                    Board[riga_i][colonna_i] = B_QUEEN;
                    Board[riga_f][colonna_f] = aux;
                    return 0;
                }
                lastFreedCell.r = riga_i; 
                lastFreedCell.c = colonna_i;
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                return 1;
            }
        }
        else if(goodLine(riga_i, colonna_i, riga_f, colonna_f))
        {
            if (freeLine(riga_i, colonna_i, riga_f, colonna_f))
            {
                type_pezzo aux = Board[riga_f][colonna_f];
                Board[riga_i][colonna_i] = EMPTY;
                Board[riga_f][colonna_f] = B_QUEEN;
                if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                {
                    Board[riga_i][colonna_i] = B_QUEEN;
                    Board[riga_f][colonna_f] = aux;
                    return 0;
                }
                lastFreedCell.r = riga_i; 
                lastFreedCell.c = colonna_i;
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                return 1;
            }
        }
        return 0;
    case W_KING:
        if((abs(riga_i - riga_f) <= 1) && abs(colonna_i - colonna_f) <= 1)
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = W_KING;
            if(isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
            {
                Board[riga_i][colonna_i] = W_KING;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            else
            {
                wKingPosition.r = riga_f;
                wKingPosition.c = colonna_f;
                lastFreedCell.r = riga_i; 
                lastFreedCell.c = colonna_i;
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                castle_privileges &= ~(WHITE_CASTLE_SHORT_PRIVILEGE | WHITE_CASTLE_LONG_PRIVILEGE); // unset black castle privilege
                return 1;
            }
        }
        else if((riga_f == 0) && (colonna_f == 6) && (castle_privileges & WHITE_CASTLE_SHORT_PRIVILEGE))
        {
            if(freeLine(riga_i, colonna_i, riga_f, colonna_f + 1)){
                int legalCastle = 1;
                for(struct position aux = wKingPosition; aux.c <= colonna_f; aux.c++)
                if(isItCheck(aux, WHITE, NOT_OVER_WRITE))
                {
                    legalCastle = 0;
                    break;
                }
                if(legalCastle)
                { // non aggiorno l'ultima cella liberata in quanto non serve
                    Board[0][7] = EMPTY;
                    Board[riga_i][colonna_i] = W_ROOK;
                    Board[riga_f][colonna_f] = W_KING;
                    wKingPosition.r = riga_f;
                    wKingPosition.c = colonna_f;
                    lastPieceMoved.r = riga_i; //nuova posizione della torre
                    lastPieceMoved.c = colonna_i;
                    castle_privileges &= ~(WHITE_CASTLE_SHORT_PRIVILEGE | WHITE_CASTLE_LONG_PRIVILEGE); // unset white castle privilege
                    return 1;
                }
            }
        }
        else if((riga_f == 0) && (colonna_f == 2) && (castle_privileges & WHITE_CASTLE_LONG_PRIVILEGE))
        {
            if(freeLine(riga_i, colonna_i, riga_f, colonna_f - 1)){
                int legalCastle = 1;
                for(struct position aux = wKingPosition; aux.c <= colonna_f; aux.c++)
                if(isItCheck(aux, WHITE, NOT_OVER_WRITE))
                {
                    legalCastle = 0;
                    break;
                }
                if(legalCastle)
                {
                    Board[0][0] = EMPTY;
                    Board[riga_i][colonna_i - 1] = W_ROOK;
                    Board[riga_f][colonna_f] = W_KING;
                    wKingPosition.r = riga_f;
                    wKingPosition.c = colonna_f;
                    lastPieceMoved.r = riga_i;
                    lastPieceMoved.c = colonna_i - 1;
                    castle_privileges &= ~(WHITE_CASTLE_SHORT_PRIVILEGE | WHITE_CASTLE_LONG_PRIVILEGE); // unset wite castle privilege
                    return 1;
                }
            }
            return 0;
        }
        return 0;
    case B_KING:
        if((abs(riga_i - riga_f) <= 1) && abs(colonna_i - colonna_f) <= 1)
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = B_KING;
            if(isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
            {
                Board[riga_i][colonna_i] = B_KING;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            else
            {
                lastFreedCell.r = riga_i; 
                lastFreedCell.c = colonna_i;
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                bKingPosition.r = riga_f;
                bKingPosition.c = colonna_f;
                castle_privileges &= ~(BLACK_CASTLE_SHORT_PRIVILEGE | BLACK_CASTLE_LONG_PRIVILEGE); // unset black castle privilege
                return 1;
            }
        }
        else if((riga_f == 7) && (colonna_f == 6) && (castle_privileges & BLACK_CASTLE_SHORT_PRIVILEGE))
        {
            if(freeLine(riga_i, colonna_i, riga_f, colonna_f + 1)){
                int legalCastle = 1;
                for(struct position aux = bKingPosition; aux.c <= colonna_f; aux.c++)
                if(isItCheck(aux, BLACK, NOT_OVER_WRITE))
                {
                    legalCastle = 0;
                    break;
                }
                if(legalCastle)
                {
                    Board[7][7] = EMPTY;
                    Board[riga_i][colonna_i] = B_ROOK;
                    Board[riga_f][colonna_f] = B_KING;
                    bKingPosition.r = riga_f;
                    bKingPosition.c = colonna_f;
                    lastPieceMoved.r = riga_i;
                    lastPieceMoved.c = colonna_i;
                    castle_privileges &= ~(BLACK_CASTLE_SHORT_PRIVILEGE | BLACK_CASTLE_LONG_PRIVILEGE); // unset black castle privilege
                    return 1;
                }
            }
        }
        else if((riga_f == 0) && (colonna_f == 2) && (castle_privileges & BLACK_CASTLE_LONG_PRIVILEGE))
        {
            if(freeLine(riga_i, colonna_i, riga_f, colonna_f - 1)){
                int legalCastle = 1;
                for(struct position aux = bKingPosition; aux.c <= colonna_f; aux.c++)
                if(isItCheck(aux, BLACK, NOT_OVER_WRITE))
                {
                    legalCastle = 0;
                    break;
                }
                if(legalCastle)
                {
                    Board[7][0] = EMPTY;
                    Board[riga_i][colonna_i - 1] = B_ROOK;
                    Board[riga_f][colonna_f] = B_KING;
                    bKingPosition.r = riga_f;
                    bKingPosition.c = colonna_f;
                    lastPieceMoved.r = riga_i;
                    lastPieceMoved.c = colonna_i - 1;
                    castle_privileges &= ~(BLACK_CASTLE_SHORT_PRIVILEGE | BLACK_CASTLE_LONG_PRIVILEGE); // unset black castle privilege
                    return 1;
                }
            }
        }
        return 0;
    default:
        break;
    }
    return 0;
}

void print_chessboard()
{
    printf("\n a  b  c  d  e  f  g  h\n");
    for(int i = 0; i < 8; i++)
    {
        printf("_________________________\n");
        for (int j = 0; j < 8; j++)
        {
            if (j == 0) putchar('|');
            putchar(Board[i][j]);
            putchar(' ');
            putchar('|');
        }
        printf("%d\n", i+1);
    }
    printf("_________________________\n");
}

int isItCheck(struct position inExamPiece, int color, int ow) 
// usa il metodo degli accumulatori per scorrere diagonali e linee !!!!!!!!!!!!!!!
{
    int r, c;
    r = inExamPiece.r;
    c = inExamPiece.c;
    int check = 0;
    if(!checkInBound(r, c))
    {
        return 0; // non può essere scacco fuori dalla scacchiera
    }
    //check cavalli, pensa a come migliorare
    if(color == WHITE)
    {
        if (checkInBound(r+1, c+2) && Board[r+1][c+2] == B_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r+1, .c = c+2};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r+1, c-2) && Board[r+1][c-2] == B_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r+1, .c = c-2};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r-1, c+2) && Board[r-1][c+2] == B_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r-1, .c = c+2};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r-1, c-2) && Board[r-1][c-2] == B_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r-1, .c = c-2};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r+2, c+1) && Board[r+2][c+1] == B_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r+2, .c = c+1};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r+2, c-1) && Board[r+2][c-1] == B_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r+2, .c = c-1};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r-2, c+1) && Board[r-2][c+1] == B_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r-2, .c = c+1};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r-2, c-1) && Board[r-2][c-1] == B_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r-2, .c = c-1};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
    }
    if(color == BLACK)
    {
        if (checkInBound(r+1, c+2) && Board[r+1][c+2] == W_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r+1, .c = c+2};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r+1, c-2) && Board[r+1][c-2] == W_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r+1, .c = c-2};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r-1, c+2) && Board[r-1][c+2] == W_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r-1, .c = c+2};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r-1, c-2) && Board[r-1][c-2] == W_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r-1, .c = c-2};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r+2, c+1) && Board[r+2][c+1] == W_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r+2, .c = c+1};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r+2, c-1) && Board[r+2][c-1] == W_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r+2, .c = c-1};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r-2, c+1) && Board[r-2][c+1] == W_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r-2, .c = c+1};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
        else if (checkInBound(r-2, c-1) && Board[r-2][c-1] == W_KNIGHT)
        {
            check = 1;
            if(ow)
            {
                struct position paux = {.r = r-2, .c = c-1};
                insert_position(&attackers, paux);
            }
            else
            {
                return check;
            }
        }
    }
    // check pedone basso sx
    if(((Board[r + 1][c - 1] == B_PAWN)) && color == WHITE) 
    {
        check = 1;
        if(ow)
        {
            struct position paux = {.r = r + 1, .c = c - 1};
            insert_position(&attackers, paux);
        }
        else
        {
            return check;
        }
    }
    // check pedone basso dx
    if((Board[r + 1][c + 1] == B_PAWN) && color == WHITE) //check pedone
    {
        check = 1;
        if(ow)
        {
            struct position paux = {.r = r + 1, .c = c + 1};
            insert_position(&attackers, paux);
        }
        else
        {
            return check;
        }
    }
    if((Board[r - 1][c - 1] == W_PAWN) && color == BLACK) // check pedone
    {
        check = 1;
        if(ow)
        {
            struct position paux = {.r = r - 1, .c = c - 1};
            insert_position(&attackers, paux);
        }
        else
        {
            return check;
        }
    }
    if((Board[r - 1][c + 1] == W_PAWN) && color == BLACK) // check pedone
    {
        check = 1;
        if(ow)
        {
            struct position paux = {.r = r - 1, .c = c + 1};
            insert_position(&attackers, paux);
        }
        else
        {
            return check;
        }
    }
    
    for(int acc_i = -1; acc_i <= 1; acc_i++)
    {
        for(int acc_j = -1; acc_j <= 1; acc_j++)
        {
            int i = r, j = c;
            if (!((acc_i == 0) && (acc_j == 0)))
            {
                i += acc_i; j += acc_j;
                while(checkInBound(i, j)) // qualche check tipo se non empty esci?!?!?!?
                {
                    if(((color == WHITE) && isWhitePiece(Board[i][j])) || ((color == BLACK) && isBlackPiece(Board[i][j])))
                    {
                        break; // incontrata casa dello stesso colore, non può farmi scacco
                    }
                    if((((Board[i][j] == W_ROOK) || (Board[i][j] == W_QUEEN) || (Board[i][j] == B_ROOK) || (Board[i][j] == B_QUEEN)) && goodLine(i, j, r, c)) || (((Board[i][j] == W_BISHOP) || (Board[i][j] == W_QUEEN) || (Board[i][j] == B_BISHOP) || (Board[i][j] == B_QUEEN)) && goodDiagonal(i, j, r, c)))
                    {
                        check = 1;
                        if(ow)
                        {
                            struct position paux = {.r = i, .c = j};
                            insert_position(&attackers, paux);
                            break;
                        }
                        else
                        {
                            return check;
                        }
                    }
                    i += acc_i; j += acc_j;
                }
            }
        }
    }
    return check;
}

int checkMate()
{
    if(turn == WHITE)
    {
        //vedo se il nero è sotto scacco matto
        int ret_value = CHECK_MATE; // invece di i, j usa una position
        for(int i = -1; i <= 1; i++) // pensa meglio a caso i = 0, j = 0
        {
            for (int j = -1; j < 1; j++)
            {
                if (!((i == 0) && (j == 0)))
                {
                    if(checkInBound(bKingPosition.r+i, bKingPosition.c+j)) 
                    {
                        if(!isBlackPiece(Board[bKingPosition.r + i][bKingPosition.c + j]))
                        {
                            type_pezzo aux = Board[bKingPosition.r + i][bKingPosition.c + j];
                            Board[bKingPosition.r + i][bKingPosition.c + j] = B_KING;
                            Board[bKingPosition.r][bKingPosition.c] = EMPTY;
                            struct position aux_k_position = {.r = bKingPosition.r + i, .c = bKingPosition.c + j};
                            if(!isItCheck(aux_k_position, BLACK, NOT_OVER_WRITE)) 
                            {
                                Board[bKingPosition.r + i][bKingPosition.c + j] = aux;
                                Board[bKingPosition.r][bKingPosition.c] = B_KING;
                                return NOT_FINISHED;
                            }
                            Board[bKingPosition.r + i][bKingPosition.c + j] = aux;
                            Board[bKingPosition.r][bKingPosition.c] = B_KING;
                        }
                    }
                }
            }
        } // fino a qui valutate le mosse del re per il nero
        struct position inExamSquare;
        if(isValidMove(lastPieceMoved, bKingPosition))
        {
            flush_position_list(&attackers); // libero la lista di pezzi
            isItCheck(lastPieceMoved, WHITE, OVER_WRITE);
            struct position saverPiece;
            while(attackers != NULL)
            {
                saverPiece = pop_position(&attackers);
                if(isValidMove(saverPiece, lastPieceMoved)) // forse superfluo ???
                {
                    type_pezzo aux;
                    aux = Board[lastPieceMoved.r][lastPieceMoved.c];
                    Board[lastPieceMoved.r][lastPieceMoved.c] = Board[saverPiece.r][saverPiece.c];
                    Board[saverPiece.r][saverPiece.c] = EMPTY;
                    if (!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {
                            Board[saverPiece.r][saverPiece.c] = Board[lastPieceMoved.r][lastPieceMoved.c];
                            Board[lastPieceMoved.r][lastPieceMoved.c] = aux;
                            return NOT_FINISHED;
                    }
                    Board[saverPiece.r][saverPiece.c] = Board[lastPieceMoved.r][lastPieceMoved.c];
                    Board[lastPieceMoved.r][lastPieceMoved.c] = aux;
                }
            } // se sono sotto scacco dall'ultimo pezzo mosso vedo se posso mangiarlo
            inExamSquare = lastPieceMoved;
            if((Board[inExamSquare.r][inExamSquare.c] == W_PAWN) || (Board[inExamSquare.r][inExamSquare.c] == W_KNIGHT)) 
                return CHECK_MATE; // se il pezzo che mi mette sotto scacco non posso mangiarlo ed è un pedone o cavallo è matto
            else if ((abs(inExamSquare.r - bKingPosition.r) <= 1) && (abs(inExamSquare.r - bKingPosition.r) <= 1))
                return CHECK_MATE; // pezzo troppo vicino, non posso spostarmi ne mangiarlo => ho perso
        }
        else // scacco di scoperta, trovo il pezzo che mi fa scacco
        {
            if(goodDiagonal(lastFreedCell.r, lastFreedCell.c, bKingPosition.r, bKingPosition.c)) 
            {
                int i = bKingPosition.r, j = bKingPosition.c; 
                int acc_i, acc_j;
                acc_i = (bKingPosition.r < lastFreedCell.r)? 1 : -1;
                acc_j = (bKingPosition.c < lastFreedCell.c)? 1 : -1;
                while(checkInBound(i, j) && Board[i][j] == EMPTY)
                {
                    i+=acc_i;
                    j+=acc_j;
                }
                if (!checkInBound(i, j)) // non dovrebbe poter succedere ma nel dubbio metto il check
                {
                    return CHECK_MATE; // ?????
                }
                inExamSquare.r = i; inExamSquare.c = j;
            }
            else if(goodLine(lastFreedCell.r, lastFreedCell.c, bKingPosition.r, bKingPosition.c)){
                int i = bKingPosition.r, j = bKingPosition.c; 
                int acc_i, acc_j;
                if(bKingPosition.r == lastFreedCell.r)
                {
                    acc_i = 0;
                    if (bKingPosition.c < lastFreedCell.c)
                    {
                        acc_j = 1;
                    }
                    else
                        acc_j = -1;
                }
                else
                {
                    acc_j = 0;
                    if (bKingPosition.r < lastFreedCell.r)
                    {
                        acc_i = 1;
                    }
                    else
                        acc_i = -1;
                }
                while(checkInBound(i, j) && Board[i][j] == EMPTY)
                {
                    i+=acc_i;
                    j+=acc_j;
                }
                if (!checkInBound(i, j)) // non dovrebbe poter succedere ma nel dubbio metto il check
                {
                    return CHECK_MATE;
                }
                inExamSquare.r = i; inExamSquare.c = j;
            }
            else if((auxLastFreedCell.r != -1) && goodDiagonal(auxLastFreedCell.r, auxLastFreedCell.c, bKingPosition.r, bKingPosition.c))
            {
                int i = bKingPosition.r, j = bKingPosition.c; 
                int acc_i, acc_j;
                acc_i = (bKingPosition.r < auxLastFreedCell.r)? 1 : -1;
                acc_j = (bKingPosition.c < auxLastFreedCell.c)? 1 : -1;
                while(checkInBound(i, j) && Board[i][j] == EMPTY)
                {
                    i+=acc_i;
                    j+=acc_j;
                }
                if (!checkInBound(i, j)) // non dovrebbe poter succedere ma nel dubbio metto il check
                {
                    return CHECK_MATE; // ?????
                }
                inExamSquare.r = i; inExamSquare.c = j;
            } 
        }
        if((goodLine(inExamSquare.r, inExamSquare.c, bKingPosition.r, bKingPosition.c)))
        {
            int i = bKingPosition.r, j = bKingPosition.c;
            int acc_i, acc_j;

            if(inExamSquare.r == bKingPosition.r)
            {
                acc_i = 0;
                if (inExamSquare.c > bKingPosition.c)
                    acc_j = 1;
                else
                    acc_j = -1;
            }
            else
            {
                acc_j = 0;
                if (inExamSquare.c > bKingPosition.c)
                    acc_i = 1;
                else
                    acc_i = -1;
            }
            i = i + acc_i;
            j = j + acc_j;
            while((i != inExamSquare.r) && (j != inExamSquare.c))
            {
                struct position aux_p = {.r = i, .c = j};
                isItCheck(aux_p, BLACK, OVER_WRITE);
                struct position saverPiece;
                while(attackers != NULL)
                {
                    saverPiece = pop_position(&attackers);
                    type_pezzo aux;
                    aux = Board[i][j];
                    Board[i][j] = Board[saverPiece.r][saverPiece.c];
                    Board[saverPiece.r][saverPiece.c] = EMPTY;
                    if (!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {
                        Board[saverPiece.r][saverPiece.c] = Board[i][j];
                        Board[i][j] = aux;
                        return NOT_FINISHED;
                    }
                    Board[saverPiece.r][saverPiece.c] = Board[i][j];
                    Board[i][j] = aux;
                }
                i = i + acc_i;
                j = j + acc_j;
            }
        }
        else if(goodDiagonal(inExamSquare.r, inExamSquare.c, bKingPosition.r, bKingPosition.c))
        {
            int i = bKingPosition.r, j = bKingPosition.c;
            int acc_i, acc_j;

            if(inExamSquare.r < bKingPosition.r)
            {
                acc_i = -1;
                if (inExamSquare.c > bKingPosition.c)
                    acc_j = 1;
                else
                    acc_j = -1;
            }
            else
            {
                acc_j = + 1;
                if (inExamSquare.c > bKingPosition.c)
                    acc_i = 1;
                else
                    acc_i = -1;
            }
            i = i + acc_i;
            j = j + acc_j;
            while((i != inExamSquare.r) && (j != inExamSquare.c))
            {
                struct position aux_p = {.r = i, .c = j};
                isItCheck(aux_p, BLACK, OVER_WRITE);

                struct position saverPiece;
                while(attackers != NULL)
                {
                    saverPiece = pop_position(&attackers);
                    type_pezzo aux;
                    aux = Board[i][j];
                    Board[i][j] = Board[saverPiece.r][saverPiece.c];
                    Board[saverPiece.r][saverPiece.c] = EMPTY;
                    if (!isItCheck(bKingPosition, BLACK, NOT_OVER_WRITE))
                    {
                        Board[saverPiece.r][saverPiece.c] = Board[i][j];
                        Board[i][j] = aux;
                        return NOT_FINISHED;
                    }
                    Board[saverPiece.r][saverPiece.c] = Board[i][j];
                    Board[i][j] = aux;
                }
                i = i + acc_i;
                j = j + acc_j;
            }
        }
        return ret_value;
    }
    else // black turn branch
    {
        //vedo se il bianco è sotto scacco matto
        int ret_value = CHECK_MATE;
        for(int i = -1; i <= 1; i++)
        {
            for (int j = -1; j < 1; j++)
            {
                if (!((i == 0) && (j == 0)))
                {
                    if(checkInBound(wKingPosition.r+i, wKingPosition.c+j)) 
                    {
                        if(!isWhitePiece(Board[wKingPosition.r + i][wKingPosition.c + j]))
                        {
                            type_pezzo aux = Board[wKingPosition.r + i][wKingPosition.c + j];
                            Board[wKingPosition.r + i][wKingPosition.c + j] = W_KING;
                            Board[wKingPosition.r][wKingPosition.c] = EMPTY;
                            struct position aux_k_position = {.r = wKingPosition.r + i, .c = wKingPosition.c + j};
                            if(!isItCheck(aux_k_position, WHITE, NOT_OVER_WRITE))
                            {
                                Board[wKingPosition.r + i][wKingPosition.c + j] = aux;
                                Board[wKingPosition.r][wKingPosition.c] = W_KING;
                                return NOT_FINISHED;
                            }
                            Board[wKingPosition.r + i][wKingPosition.c + j] = aux;
                            Board[wKingPosition.r][wKingPosition.c] = W_KING;
                        }
                    }
                }
            }
        } // fino a qui valutate le mosse del re per il nero
        struct position inExamSquare;
        if(isValidMove(lastPieceMoved, wKingPosition))
        {
            flush_position_list(&attackers); // libero la lista di pezzi
            isItCheck(lastPieceMoved, WHITE, OVER_WRITE);
            struct position saverPiece;
            while(attackers != NULL)
            {
                saverPiece = pop_position(&attackers);
                if(isValidMove(saverPiece, lastPieceMoved))
                {
                    type_pezzo aux;
                    aux = Board[lastPieceMoved.r][lastPieceMoved.c];
                    Board[lastPieceMoved.r][lastPieceMoved.c] = Board[saverPiece.r][saverPiece.c];
                    Board[saverPiece.r][saverPiece.c] = EMPTY;
                    if (!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {
                        Board[saverPiece.r][saverPiece.c] = Board[lastPieceMoved.r][lastPieceMoved.c];
                        Board[lastPieceMoved.r][lastPieceMoved.c] = aux;
                        return NOT_FINISHED;
                    }
                    Board[saverPiece.r][saverPiece.c] = Board[lastPieceMoved.r][lastPieceMoved.c];
                    Board[lastPieceMoved.r][lastPieceMoved.c] = aux;
                }
            } // se sono sotto scacco dall'ultimo pezzo mosso vedo se posso mangiarlo
            inExamSquare = lastPieceMoved;
            if((Board[inExamSquare.r][inExamSquare.c] == B_PAWN) || (Board[inExamSquare.r][inExamSquare.c] == B_KNIGHT))
                return CHECK_MATE; // se il pezzo che mi mette sotto scacco non posso mangiarlo ed è un pedone o cavallo è matto
            else if ((abs(inExamSquare.r - wKingPosition.r) <= 1) && (abs(inExamSquare.r - wKingPosition.r) <= 1))
                return CHECK_MATE; // il pezzo è attaccato al re, se non posso spostarmi o mangiarlo ho perso
        }
        else // scacco di scoperta, trovo il pezzo che mi fa scacco
        {
            if(goodDiagonal(lastFreedCell.r, lastFreedCell.c, wKingPosition.r, wKingPosition.c)) 
            {
                int i = wKingPosition.r, j = wKingPosition.c; 
                int acc_i, acc_j;
                acc_i = (wKingPosition.r < lastFreedCell.r)? 1 : -1;
                acc_j = (wKingPosition.c < lastFreedCell.c)? 1 : -1;
                while(checkInBound(i, j) && Board[i][j] == EMPTY)
                {
                    i+=acc_i;
                    j+=acc_j;
                }
                if (!checkInBound(i, j)) // non dovrebbe poter succedere ma nel dubbio metto il check
                {
                    return CHECK_MATE;
                }
                inExamSquare.r = i; inExamSquare.c = j;
            }
            else if(goodLine(lastFreedCell.r, lastFreedCell.c, wKingPosition.r, wKingPosition.c)){
                int i = wKingPosition.r, j = wKingPosition.c; 
                int acc_i, acc_j;
                if(wKingPosition.r == lastFreedCell.r)
                {
                    acc_i = 0;
                    if (wKingPosition.c < lastFreedCell.c)
                    {
                        acc_j = 1;
                    }
                    else
                        acc_j = -1;
                }
                else
                {
                    acc_j = 0;
                    if (wKingPosition.r < lastFreedCell.r)
                    {
                        acc_i = 1;
                    }
                    else
                        acc_i = -1;
                }
                while(checkInBound(i, j) && Board[i][j] == EMPTY)
                {
                    i+=acc_i;
                    j+=acc_j;
                }
                if (!checkInBound(i, j)) // non dovrebbe poter succedere ma nel dubbio metto il check
                {
                    return CHECK_MATE;
                }
                inExamSquare.r = i; inExamSquare.c = j;
            }
            else if((auxLastFreedCell.r != -1) && goodDiagonal(auxLastFreedCell.r, auxLastFreedCell.c, wKingPosition.r, wKingPosition.c))
            {
                int i = wKingPosition.r, j = wKingPosition.c; 
                int acc_i, acc_j;
                acc_i = (wKingPosition.r < auxLastFreedCell.r)? 1 : -1;
                acc_j = (wKingPosition.c < auxLastFreedCell.c)? 1 : -1;
                while(checkInBound(i, j) && Board[i][j] == EMPTY)
                {
                    i+=acc_i;
                    j+=acc_j;
                }
                if (!checkInBound(i, j)) // non dovrebbe poter succedere ma nel dubbio metto il check
                {
                    return CHECK_MATE; // ?????
                }
                inExamSquare.r = i; inExamSquare.c = j;
            } 
        }
        if(goodLine(inExamSquare.r, inExamSquare.c, wKingPosition.r, wKingPosition.c))
        {
            int i = wKingPosition.r, j = wKingPosition.c;
            int acc_i, acc_j;

            if(inExamSquare.r == wKingPosition.r)
            {
                acc_i = 0;
                if (inExamSquare.c > wKingPosition.c)
                    acc_j = 1;
                else
                    acc_j = -1;
            }
            else
            {
                acc_j = 0;
                if (inExamSquare.c > wKingPosition.c)
                    acc_i = 1;
                else
                    acc_i = -1;
            }
            i = i + acc_i;
            j = j + acc_j;
            while((i != inExamSquare.r) && (j != inExamSquare.c))
            {
                struct position aux_p = {.r = i, .c = j};
                isItCheck(aux_p, WHITE, OVER_WRITE);
                struct position saverPiece;
                while(attackers != NULL)
                {
                    saverPiece = pop_position(&attackers);
                    type_pezzo aux;
                    aux = Board[i][j];
                    Board[i][j] = Board[saverPiece.r][saverPiece.c];
                    Board[saverPiece.r][saverPiece.c] = EMPTY;
                    if (!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {
                        Board[saverPiece.r][saverPiece.c] = Board[i][j];
                        Board[i][j] = aux;
                        return NOT_FINISHED;
                    }
                    Board[saverPiece.r][saverPiece.c] = Board[i][j];
                    Board[i][j] = aux;
                }
                i = i + acc_i;
                j = j + acc_j;
            }
        }
        else if(goodDiagonal(inExamSquare.r, inExamSquare.c, wKingPosition.r, wKingPosition.c))
        {
            int i = wKingPosition.r, j = wKingPosition.c;
            int acc_i, acc_j;

            if(inExamSquare.r < wKingPosition.r)
            {
                acc_i = -1;
                if (inExamSquare.c > wKingPosition.c)
                    acc_j = 1;
                else
                    acc_j = -1;
            }
            else
            {
                acc_j = + 1;
                if (inExamSquare.c > wKingPosition.c)
                    acc_i = 1;
                else
                    acc_i = -1;
            }
            i = i + acc_i;
            j = j + acc_j;
            while((i != inExamSquare.r) && (j != inExamSquare.c))
            {
                struct position aux_p = {.r = i, .c = j};
                isItCheck(aux_p, WHITE, OVER_WRITE);

                struct position saverPiece;
                while(attackers != NULL)
                {
                    saverPiece = pop_position(&attackers);
                    type_pezzo aux;
                    aux = Board[i][j];
                    Board[i][j] = Board[saverPiece.r][saverPiece.c];
                    Board[saverPiece.r][saverPiece.c] = EMPTY;
                    if (!isItCheck(wKingPosition, WHITE, NOT_OVER_WRITE))
                    {
                        Board[saverPiece.r][saverPiece.c] = Board[i][j];
                        Board[i][j] = aux;
                        return NOT_FINISHED;
                    }
                    Board[saverPiece.r][saverPiece.c] = Board[i][j];
                    Board[i][j] = aux;
                }
                i = i + acc_i;
                j = j + acc_j;
            }
        }
    }
    return CHECK_MATE;
} 

int promotion(int color)
{
        for(int i = 0; i < 8; i++)
            if(color == WHITE)
                if(Board[7][i] == W_PAWN)
                    return i;
            else
                if(Board[0][i] == B_PAWN)
                    return i;
    return 0;
}
