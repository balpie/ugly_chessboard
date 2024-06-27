#include "input-output.h"

void flushStdin() // fai header e source file per io
{
    scanf("%*[^\n]");
    scanf("%*c");
}

type_pezzo inputPiece()
{
    type_pezzo p;
    while((p = getchar()) != EMPTY);
    flushStdin();
    return p;
}

void inputMove(struct position* mov)
{
    char c;
    mov[0].c = getchar();
    mov[0].r = getchar();
    mov[1].c = getchar();
    mov[1].r = getchar();
    flushStdin();

    mov[0].c -= 'a';
    mov[0].r -= '1';
    mov[1].c -= 'a';
    mov[1].r -= '1';
}