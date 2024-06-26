
#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <stdio.h>

#define WHITE 0
#define BLACK 1

typedef char type_pezzo;

/// @brief preleva da stdin un type_pezzo
/// @return ritorna il pezzo prelevato
type_pezzo inputPiece(int color);

#endif