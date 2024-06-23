#include "board.h"

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

int turn = WHITE;
unsigned int castle_privileges = WHITE_CASTLE_PRIVILEGE | BLACK_CASTLE_PRIVILEGE;
int game_status = NOT_FINISHED;

struct position wKingPosition = {.r = 4, .c = 0};
struct position bKingPosition = {.r = 4, .c = 7};
struct position lastPieceMoved = {.r = -1, .c = -1};

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
    while((i != r_f - acc_i) && (j != c_f - acc_j)) // controllo fino alla penultima cella della diagonale
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
    int acc, i, ret = 1;
    if (abs(r_i - r_f) == 1 || abs(c_i - c_f) == 1)
    {
        return 1; //linea sicuramente libera, posto adiacente
    }
    if (r_i == r_f)
    {
        acc = (c_f > c_i)? 1 : -1;
        i = c_i + acc;
        while(i < c_f - acc)
        {
            if (Board[r_i][i] != EMPTY) 
            {
                ret = 0;
                break;
            }
            i += acc;
        }
    }
    else // c_i == c_f
    {
        acc = (r_f > r_i)? 1 : -1;
        i = r_i + acc;
        while(i < r_f - acc)
        {
            if (Board[i][c_i] != EMPTY) 
            {
                ret = 0;
                break;
            }
            i += acc;
        }
    }
    return ret;
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
    if(isWhitePiece(Board[riga_f][colonna_f]) && isWhitePiece(Board[riga_i][colonna_i]) || isWhitePiece(Board[riga_f][colonna_f]) && isWhitePiece(Board[riga_i][colonna_i]))
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
                    if(isItCheck(WHITE))
                    {
                        Board[riga_i][colonna_i] = W_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
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
                    if(isItCheck(WHITE))
                    {
                        Board[riga_i][colonna_i] = W_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
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
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = W_PAWN;
                    if(isItCheck(WHITE))
                    {
                        Board[riga_i][colonna_i] = W_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
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
                    if(isItCheck(BLACK))
                    {
                        Board[riga_i][colonna_i] = B_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
            }
            else if (colonna_i == colonna_f)
            {
                if(Board[riga_f][colonna_f] == EMPTY)
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = B_PAWN;
                    if(isItCheck(BLACK))
                    {
                        Board[riga_i][colonna_i] = B_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
                else return 0;
            }
            else 
                return 0;
        }
        else if (riga_i == riga_f + 2 ) // considera ipotesi enpassant
            if(colonna_f == colonna_i && riga_i == 6 && Board[riga_i - 1][colonna_i] == EMPTY && Board[riga_i - 2][colonna_i] == EMPTY)
                {
                    type_pezzo aux = Board[riga_f][colonna_f];
                    Board[riga_i][colonna_i] = EMPTY;
                    Board[riga_f][colonna_f] = B_PAWN;
                    if(isItCheck(BLACK))
                    {
                        Board[riga_i][colonna_i] = B_PAWN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
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
            if(isItCheck(WHITE))
            {
                Board[riga_i][colonna_i] = W_KNIGHT;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
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
            if(isItCheck(BLACK))
            {
                Board[riga_i][colonna_i] = B_KNIGHT;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
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
            if(isItCheck(WHITE))
            {
                Board[riga_i][colonna_i] = W_BISHOP;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
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
            if(isItCheck(BLACK))
            {
                Board[riga_i][colonna_i] = B_BISHOP;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
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
            if(isItCheck(WHITE))
            {
                Board[riga_i][colonna_i] = W_ROOK;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            lastPieceMoved.r = riga_f;
            lastPieceMoved.c = colonna_f;
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
            if(isItCheck(BLACK))
            {
                Board[riga_i][colonna_i] = B_ROOK;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            lastPieceMoved.r = riga_f;
            lastPieceMoved.c = colonna_f;
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
                if(isItCheck(WHITE))
                {
                    Board[riga_i][colonna_i] = W_QUEEN;
                    Board[riga_f][colonna_f] = aux;
                    return 0;
                }
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
                    if(isItCheck(WHITE))
                    {
                        Board[riga_i][colonna_i] = W_QUEEN;
                        Board[riga_f][colonna_f] = aux;
                        return 0;
                    }
                    lastPieceMoved.r = riga_f;
                    lastPieceMoved.c = colonna_f;
                    return 1;
                }
            }
        }
        return 0;
    case B_QUEEN:
        if(riga_i != riga_f && colonna_i != colonna_f) // si sta muovendo in diagonale
        {
            if (!goodDiagonal(riga_i, colonna_i, riga_f, colonna_f))
            {
                return 0; // mossa non valida
            }
            if (freeDiagonal(riga_i, colonna_i, riga_f, colonna_f))
            {
                type_pezzo aux = Board[riga_f][colonna_f];
                Board[riga_i][colonna_i] = EMPTY;
                Board[riga_f][colonna_f] = B_QUEEN;
                if(isItCheck(BLACK))
                {
                    Board[riga_i][colonna_i] = B_QUEEN;
                    Board[riga_f][colonna_f] = aux;
                    return 0;
                }
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                return 1;
            }
            return 0;
        }
        else 
        {
            if (!goodLine(riga_i, colonna_i, riga_f, colonna_f))
            {
                return 0;
            }
            if (freeLine(riga_i, colonna_i, riga_f, colonna_f))
            {
                type_pezzo aux = Board[riga_f][colonna_f];
                Board[riga_i][colonna_i] = EMPTY;
                Board[riga_f][colonna_f] = B_QUEEN;
                if(isItCheck(BLACK))
                {
                    Board[riga_i][colonna_i] = B_QUEEN;
                    Board[riga_f][colonna_f] = aux;
                    return 0;
                }
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                return 1;
            }
            return 0;
        }
    case W_KING:
        if((riga_i == riga_f || riga_i == riga_f - 1 || riga_i == riga_f + 1) && (colonna_i == colonna_f || colonna_i == colonna_f - 1 || colonna_i == colonna_f + 1))
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = W_KING;
            if(isItCheck(WHITE))
            {
                Board[riga_i][colonna_i] = W_KING;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            else
            {
                wKingPosition.r = riga_f;
                wKingPosition.c = colonna_f;
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                return 1;
            }
        }
        else return 0;
    case B_KING:
        if((riga_i == riga_f || riga_i == riga_f - 1 || riga_i == riga_f + 1) && (colonna_i == colonna_f || colonna_i == colonna_f - 1 || colonna_i == colonna_f + 1))
        {
            type_pezzo aux = Board[riga_f][colonna_f];
            Board[riga_i][colonna_i] = EMPTY;
            Board[riga_f][colonna_f] = W_KING;
            if(isItCheck(WHITE))
            {
                Board[riga_i][colonna_i] = W_KING;
                Board[riga_f][colonna_f] = aux;
                return 0;
            }
            else
            {
                lastPieceMoved.r = riga_f;
                lastPieceMoved.c = colonna_f;
                bKingPosition.r = riga_f;
                bKingPosition.c = colonna_f;
                return 1;
            }
        }
        else return 0;
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
    if(turn == WHITE)
        printf("Sta al bianco: ");
    else
        printf("Sta al nero: ");
}

int isItCheck(int color)
{
    int r, c;
    if(color == WHITE)
    {
        r = wKingPosition.r;
        c = wKingPosition.c;
    }
    else
    {
        r = bKingPosition.r;
        c = bKingPosition.c;
    }
    int check = 0;
    if(!checkInBound(r, c))
    {
        return 0; // non può essere scacco fuori dalla scacchiera
    }
    int i = r + 1, j = c; //scorro verso l'alto
    while(checkInBound(i, j))  //ciclo "linee"
    {
        if(Board[i][j] == EMPTY)
        {
            i++;
        }
        else
        {
            if((color == WHITE) && isWhitePiece(Board[i][j]) || (color == BLACK) && isBlackPiece(Board[i][j]))
            {
                break; // verso l'alto ci sono case vuote e/o un pezzo dello stesso colore
            }
            if(Board[i][j] == W_ROOK || Board[i][j] == W_QUEEN || Board[i][j] == B_ROOK || Board[i][j] == B_QUEEN)
            {
                check = 1;
                return check;
            }
            break;
        }
    };
    i = r - 1, j = c; //scorro verso il basso 
    while(checkInBound(i, j))  //ciclo "linee"
    {
        if(Board[i][j] == EMPTY)
        {
            i--;
        }
        else
        {
            if((color == WHITE) && isWhitePiece(Board[i][j]) || (color == BLACK) && isBlackPiece(Board[i][j]))
            {
                break; // verso il basso ci sono case vuote e/o un pezzo dello stesso colore
            }
            if(Board[i][j] == W_ROOK || Board[i][j] == W_QUEEN || Board[i][j] == B_ROOK || Board[i][j] == B_QUEEN)
            {
                check = 1;
                return check;
            }
            break;
        }
    };
    i = r, j = c + 1; //scorro verso destra
    while(checkInBound(i, j))  //ciclo "linee"
    {
        if(Board[i][j] == EMPTY)
        {
            j++;
        }
        else
        {
            if((color == WHITE) && isWhitePiece(Board[i][j]) || (color == BLACK) && isBlackPiece(Board[i][j]))
            {
                break; // verso il basso ci sono case vuote e/o un pezzo dello stesso colore
            }
            if(Board[i][j] == W_ROOK || Board[i][j] == W_QUEEN || Board[i][j] == B_ROOK || Board[i][j] == B_QUEEN)
            {
                check = 1;
                return check;
            }
            break;
        }
    };
    i = r, j = c - 1; //scorro verso sinistra
    while(checkInBound(i, j))  //ciclo "linee"
    {
        if(Board[i][j] == EMPTY)
        {
            j--;
        }
        else
        {
            if((color == WHITE) && isWhitePiece(Board[i][j]) || (color == BLACK) && isBlackPiece(Board[i][j]))
            {
                break; // verso il basso ci sono case vuote e/o un pezzo dello stesso colore
            }
            if(Board[i][j] == W_ROOK || Board[i][j] == W_QUEEN || Board[i][j] == B_ROOK || Board[i][j] == B_QUEEN)
            {
                check = 1;
                return check;
            }
            break;
        }
    };
    i = r + 1, j = c + 1; //diagonale alto dx
    while(checkInBound(i, j))  //ciclo "linee"
    {
        if(Board[i][j] == EMPTY)
        {
            j++;
            i++;
        }
        else
        {
            if((color == WHITE) && isWhitePiece(Board[i][j]) || (color == BLACK) && isBlackPiece(Board[i][j]))
            {
                break; // verso il basso ci sono case vuote e/o un pezzo dello stesso colore
            }
            if(Board[i][j] == W_BISHOP || Board[i][j] == W_QUEEN || Board[i][j] == B_BISHOP|| Board[i][j] == B_QUEEN)
            {
                check = 1;
                return check;
            }
            break;
        }
    };
    i = r + 1, j = c - 1; //diagonale alto sx
    while(checkInBound(i, j))  //ciclo "linee"
    {
        if(Board[i][j] == EMPTY)
        {
            j--;
            i++;
        }
        else
        {
            if((color == WHITE) && isWhitePiece(Board[i][j]) || (color == BLACK) && isBlackPiece(Board[i][j]))
            {
                break; // verso il basso ci sono case vuote e/o un pezzo dello stesso colore
            }
            if(Board[i][j] == W_BISHOP || Board[i][j] == W_QUEEN || Board[i][j] == B_BISHOP|| Board[i][j] == B_QUEEN)
            {
                check = 1;
                return check;
            }
            break;
        }
    };
    i = r - 1, j = c - 1; //diagonale basso sx
    while(checkInBound(i, j))  //ciclo "linee"
    {
        if(Board[i][j] == EMPTY)
        {
            j--;
            i--;
        }
        else
        {
            if((color == WHITE) && isWhitePiece(Board[i][j]) || (color == BLACK) && isBlackPiece(Board[i][j]))
            {
                break; // verso il basso ci sono case vuote e/o un pezzo dello stesso colore
            }
            if(Board[i][j] == W_BISHOP || Board[i][j] == W_QUEEN || Board[i][j] == B_BISHOP|| Board[i][j] == B_QUEEN)
            {
                check = 1;
                return check;
            }
            break;
        }
    };
    return check;
    i = r - 1, j = c + 1; //diagonale basso dx
    while(checkInBound(i, j))  //ciclo "linee"
    {
        if(Board[i][j] == EMPTY)
        {
            j++;
            i--;
        }
        else
        {
            if((color == WHITE) && isWhitePiece(Board[i][j]) || (color == BLACK) && isBlackPiece(Board[i][j]))
            {
                break; // verso il basso ci sono case vuote e/o un pezzo dello stesso colore
            }
            if(Board[i][j] == W_BISHOP || Board[i][j] == W_QUEEN || Board[i][j] == B_BISHOP|| Board[i][j] == B_QUEEN)
            {
                check = 1;
                return check;
            }
            break;
        }
    };
    //check cavalli
    if(color == WHITE)
    {
        if (checkInBound(r+1, c+2) && Board[r+1][c+2] == B_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r+1, c-2) && Board[r+1][c-2] == B_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r-1, c+2) && Board[r-1][c+2] == B_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r-1, c-2) && Board[r+1][c-2] == B_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r+2, c+1) && Board[r+1][c+2] == B_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r+2, c-1) && Board[r+1][c-2] == B_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r-2, c+1) && Board[r-1][c+2] == B_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r-2, c-1) && Board[r+1][c-2] == B_KNIGHT)
        {
            check = 1;
            return check;
        }
    }
    if(color == BLACK)
    {
        if (checkInBound(r+1, c+2) && Board[r+1][c+2] == W_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r+1, c-2) && Board[r+1][c-2] == W_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r-1, c+2) && Board[r-1][c+2] == W_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r-1, c-2) && Board[r+1][c-2] == W_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r+2, c+1) && Board[r+1][c+2] == W_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r+2, c-1) && Board[r+1][c-2] == W_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r-2, c+1) && Board[r-1][c+2] == W_KNIGHT)
        {
            check = 1;
            return check;
        }
        else if (checkInBound(r-2, c-1) && Board[r+1][c-2] == W_KNIGHT)
        {
            check = 1;
            return check;
        }
    }
    return check;
}

int checkMate()
{
    if(turn == WHITE)
    {
        //vedo se il nero è sotto scacco matto
        for(int i = -1; i <= 1; i++)
        {
            for (int j = -1; j < 1; j++)
            {
                if(checkInBound(bKingPosition.r+i, bKingPosition.c+j)) 
                {
                    if(!isBlackPiece(Board[bKingPosition.r + i][bKingPosition.c + j]))
                    {
                        type_pezzo aux = Board[bKingPosition.r + i][bKingPosition.c + j];
                        Board[bKingPosition.r + i][bKingPosition.c + j] = B_KING;
                        Board[bKingPosition.r][bKingPosition.c] = EMPTY;
                        if(!isItCheck(BLACK))
                        {
                            return NOT_FINISHED;
                        }
                        Board[bKingPosition.r + i][bKingPosition.c + j] = aux;
                        Board[bKingPosition.r][bKingPosition.c] = B_KING;
                    }
                }
            }
        } // fino a qui valutate le mosse del re
    }
} 