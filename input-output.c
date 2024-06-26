#include "input-output.h"

extern int isWhitePiece(type_pezzo p);

extern int isBlackPiece(type_pezzo p);

type_pezzo inputPiece(int color)
{
    type_pezzo p;
    scanf("%*[\n]"); 
    scanf("%*c"); 
    if(color == WHITE)
        while(!isWhitePiece(p = getchar()));
    else
        while(!isBlackPiece(p = getchar())); 
    return p;
}