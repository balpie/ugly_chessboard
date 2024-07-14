#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include "board.h"
#include <stdio.h>

/// @brief preleva da stdin un type_pezzo
type_pezzo inputPiece();
void inputMove(struct position* mov);

// fai funzioni gestione errori !!!
#endif