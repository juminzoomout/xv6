#include "param.h"

int swap_blocks = SWAPMAX;
int swap_size_bytes = SWAPMAX * 512;
int swap_pages = SWAPMAX / 8;

char *swap_bitmap = 0;
