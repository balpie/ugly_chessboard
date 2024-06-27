#include "input-output.h"

type_pezzo inputPiece()
{
    type_pezzo p;
    scanf("%*[\n]%*c"); 
    while((p = getchar()) != EMPTY);
    return p;
}