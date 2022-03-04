#ifndef CELLS_H_INCLUDED
#define CELLS_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

typedef float CellLive;
typedef long unsigned CellIndex;

extern int cellCount;
extern int cellGridW, cellGridH;

extern CellLive *cellLife;
extern float cell_bestAvg;
extern float cellDieSpeed;
extern float cell_minLife;
extern float cellLifeBoost;
extern int cell_diagNeighbors;

static inline CellIndex cell_index(int x, int y)
{
    if (x < 0 || x >= cellGridW || y < 0 || y >= cellGridH)
        return -1;
	return x * cellGridH + y;
}

void cells_alloc(int a_cellGridW, int a_cellGridH);
void cells_realloc(int a_cellGridW, int a_cellGridH);
void cells_dealloc(void);
void cells_clean(void);
void cells_gen(unsigned seed, int full);
void cells_logic_set(float a_bestAvg, float a_best_range, float a_cellDieSpeed, float a_minLife, float a_cellLifeBoost, int a_diagNeighbors);
void cells_logic_rand(unsigned seed);
void cells_logic_save(char const *filename);
void cells_logic_load(char const *filename);
void cells_logic(void);

#endif
