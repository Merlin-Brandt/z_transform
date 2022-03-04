


#include "cells.h"
#include "cells_render.h"
#include "stdio.h"

int cellCount;
int cellGridW, cellGridH;

CellLive *cellLife;
// the average of the neighbors
static CellLive *cellNeighborAvg;

// the average of neighbors the cell can live best
float cell_bestAvg;
// range of the best average
float cell_best_range;
// the amount of life a cell looses multiplied with
// the difference between its current neighbor average and the best neighboor average
// cellLive-= dieSpeed * abs(bestAvg - neighborAvg)
// can be negative
float cellDieSpeed;
// the minimum of life a cell must have to ... live
// if (cellLife < minLife) cellLife = 0;
float cell_minLife;
// amount of life every cell gets disregarding environment
float cellLifeBoost;
int cell_diagNeighbors;

void cells_alloc(int a_cellGridW, int a_cellGridH)
{
    // set sizes
    cellGridW = a_cellGridW;
	cellGridH = a_cellGridH;
    cellCount = cellGridW * cellGridH;

    // allocate
    cellLife = malloc(cellCount * sizeof(*cellLife));
	cellNeighborAvg = malloc(cellCount * sizeof(*cellNeighborAvg));

    // initialize
    memset(cellLife, 0, cellCount * sizeof(*cellLife));
    memset(cellNeighborAvg, 0, cellCount * sizeof(*cellNeighborAvg));
}

void cells_realloc(int a_cellGridW, int a_cellGridH)
{
    // set sizes
    cellGridW = a_cellGridW;
    cellGridH = a_cellGridH;
    cellCount = cellGridW * cellGridH;

    // allocate
	cellLife = realloc(cellLife, cellCount * sizeof(*cellLife));
	cellNeighborAvg = realloc(cellNeighborAvg, cellCount * sizeof(*cellNeighborAvg));

    // initialize
    memset(cellLife, 0, cellCount * sizeof(*cellLife));
    memset(cellNeighborAvg, 0, cellCount * sizeof(*cellNeighborAvg));
}

void cells_dealloc()
{
    cells_realloc(0, 0);
}

void cells_clean()
{
    int cellI;

    for (cellI = 0; cellI < cellCount; ++cellI)
        cellLife[cellI] = 0;
}

void cells_gen(unsigned seed, int full)
{
    srand(seed);
    int xOff;
    int yOff;
    int genW;
    int genH;
    int x, y;
	int genMode = rand() % 7;
	
	if (full)
	{
		xOff = yOff = 0;
		genW = cellGridW;
		genH = cellGridH;
	}
	else
	{
		do
		{
			xOff = rand() % cellGridW;
			yOff = rand() % cellGridH;
			genW = rand() % (cellGridW - xOff);
			genH = rand() % (cellGridH - yOff);
		}
		while (genW * genH < 16);	
	}
	
#warning genMode automatically set to zero for debugging
	switch(genMode = 0)
	{
		case 0:
			for (x = 0; x < genW; ++x)
				for (y = 0; y < genH; ++y)
					cellLife[cell_index(xOff + x, yOff + y)] = rand() % 2;
				break;
		case 1:
			for (x = 0; x < genW; ++x)
				for (y = 0; y < genH; ++y)
					cellLife[cell_index(xOff + x, yOff + y)] = (x * y) % 2;
			break;
		case 2:
			for (x = 0; x < genW; ++x)
				for (y = 0; y < genH; ++y)
					cellLife[cell_index(xOff + x, yOff + y)] = (x + y) % 2;
			break;
		case 3:
			for (x = 0; x < genW; ++x)
				for (y = 0; y < genH; ++y)
					cellLife[cell_index(xOff + x, yOff + y)] = (x * y + x + y) % 2;
				break;
		case 4:
			for (x = 0; x < genW; ++x)
				for (y = 0; y < genH; ++y)
					cellLife[cell_index(xOff + x, yOff + y)] = (x * y * y) % 2;
			break;
		case 5:
			for (x = 0; x < genW; ++x)
				for (y = 0; y < genH; ++y)
					cellLife[cell_index(xOff + x, yOff + y)] = 0;
				break;
		case 6:
			for (x = 0; x < genW; ++x)
				for (y = 0; y < genH; ++y)
					cellLife[cell_index(xOff + x, yOff + y)] = 1;
				break;
		default:
			fprintf(stderr, "Internal generation error (invalid mode '%i')\n", genMode);
			exit(EXIT_FAILURE);
			break;
	}
}

void cells_logic_set(float a_bestAvg, float a_best_range, float a_cellDieSpeed, float a_minLife, float a_cellLifeBoost, int a_diagNeighbors)
{	
	cell_bestAvg = a_bestAvg;
	cell_best_range = a_best_range;
	cellDieSpeed = a_cellDieSpeed;
	cell_minLife = a_minLife;
	cellLifeBoost = a_cellLifeBoost;
	cell_diagNeighbors = a_diagNeighbors;
}

void cells_logic_rand(unsigned seed)
{
	srand(seed);
	
	cells_logic_set(
		rand() % 10000 / 10000., 
		rand() % 10000 / 10000., 
		rand() % 20000 / 10000. - 1, 
		rand() % 10000 / 10000., 
		rand() % 20000 / 20000. - .5, 
		rand() % 2);
}

void cells_logic_save(char const *filename)
{
	FILE *file = fopen(filename, "w");
	fprintf(file, "best_avg: %g\nbest_range: %g\ndie_speed: %g\nmin_life: %g\nlife_boost: %g\ndiagonal_neighbors: %i", 
			cell_bestAvg, cell_best_range, cellDieSpeed, cell_minLife, cellLifeBoost, cell_diagNeighbors);
	fclose(file);
}

void cells_logic_load(char const *filename)
{
	FILE *file = fopen(filename, "r");
	if (file)
	{
		int num = fscanf(file, "best_avg: %g best_range: %g die_speed: %g min_life: %g life_boost: %g diagonal_neighbors: %i", 
					 &cell_bestAvg, &cell_best_range, &cellDieSpeed, &cell_minLife, &cellLifeBoost, &cell_diagNeighbors);
		fclose(file);
		
		if (num != 5)
			fprintf(stderr, "Failed to data from %s\n", filename);	
	}
	else
		fprintf(stderr, "Failed to open '%s'\n", filename);	
}

void cells_logic()
{
    int i, x, y;

    // calculate neighbor averages
    // initialize directs and diagonals to direct average
    i = 0;
    for (x = 0; x < cellGridW; ++x)
        for (y = 0; y < cellGridH; ++y)
        {
            cellNeighborAvg[i] = 0;

            if (x == 0 || x == cellGridW-1 || y == 0 || y == cellGridH-1)
            {
                // if coordinate is off the grid
                int isXPOff = x + 1 == cellGridW;
				int isXMOff = x - 1 == -1;
				int isYPOff = y + 1 == cellGridH;
				int isYMOff = y - 1 == - 1;
                
                if (isXPOff) 
					cellNeighborAvg[i]+= cellLife[cell_index(0, y)];
				else
					cellNeighborAvg[i]+= cellLife[i + cellGridH];
				
				if (isXMOff) 
					cellNeighborAvg[i]+= cellLife[cell_index(cellGridW - 1, y)];
				else
					cellNeighborAvg[i]+= cellLife[i - cellGridH];
				
				if (isYPOff) 
					cellNeighborAvg[i]+= cellLife[cell_index(x, 0)];
				else
					cellNeighborAvg[i]+= cellLife[i + 1];
				
				if (isYMOff) 
					cellNeighborAvg[i]+= cellLife[cell_index(x, cellGridH - 1)];
				else
					cellNeighborAvg[i]+= cellLife[i - 1];
				
                if (cell_diagNeighbors)
                {
					// I KNOW, SO COMPLICATEEEEEED
					if (isXPOff && isYPOff) 
						cellNeighborAvg[i]+= cellLife[cell_index(0, 0)];
					else if (isXPOff)
						cellNeighborAvg[i]+= cellLife[cell_index(0, y + 1)];
					else if (isYPOff)
						cellNeighborAvg[i]+= cellLife[cell_index(x + 1, 0)];
					else
						cellNeighborAvg[i]+= cellLife[cell_index(x + 1, y + 1)];
					
					if (isXPOff && isYMOff) 
						cellNeighborAvg[i]+= cellLife[cell_index(0, cellGridH - 1)];
					else if (isXPOff)
						cellNeighborAvg[i]+= cellLife[cell_index(0, y - 1)];
					else if (isYMOff)
						cellNeighborAvg[i]+= cellLife[cell_index(x + 1, cellGridH - 1)];
					else
						cellNeighborAvg[i]+= cellLife[cell_index(x + 1, y - 1)];
					
					if (isXMOff && isYPOff) 
						cellNeighborAvg[i]+= cellLife[cell_index(cellGridW - 1, 0)];
					else if (isXMOff)
						cellNeighborAvg[i]+= cellLife[cell_index(cellGridW - 1, y + 1)];
					else if (isYPOff)
						cellNeighborAvg[i]+= cellLife[cell_index(x - 1, 0)];
					else
						cellNeighborAvg[i]+= cellLife[cell_index(x - 1, y + 1)];
					
					if (isXMOff && isYMOff) 
						cellNeighborAvg[i]+= cellLife[cell_index(cellGridW - 1, cellGridH - 1)];
					else if (isXMOff)
						cellNeighborAvg[i]+= cellLife[cell_index(cellGridW - 1, y - 1)];
					else if (isYMOff)
						cellNeighborAvg[i]+= cellLife[cell_index(x - 1, cellGridH - 1)];
					else
						cellNeighborAvg[i]+= cellLife[cell_index(x - 1, y - 1)];
                }
            }
            else
            {
                cellNeighborAvg[i]+= cellLife[i + 1];
                cellNeighborAvg[i]+= cellLife[i - 1];
                cellNeighborAvg[i]+= cellLife[i + cellGridH];
                cellNeighborAvg[i]+= cellLife[i - cellGridH];
                if (cell_diagNeighbors)
                {
                    cellNeighborAvg[i]+= cellLife[i + cellGridH + 1];
                    cellNeighborAvg[i]+= cellLife[i + cellGridH - 1];
                    cellNeighborAvg[i]+= cellLife[i - cellGridH + 1];
                    cellNeighborAvg[i]+= cellLife[i - cellGridH - 1];
                }
            }

			
			
			cellNeighborAvg[i]/= (cell_diagNeighbors ? 8 : 4);
            ++i;
        }

    i = 0;
    for (x = 0; x < cellGridW; ++x)
        for (y = 0; y < cellGridH; ++y)
        {
			float diffToBest = fabs(cell_bestAvg - cellNeighborAvg[i]);
			cellLife[i]-= cellDieSpeed * (diffToBest - cell_best_range);
			cellLife[i]+= cellLifeBoost;
			if (cellLife[i] < cell_minLife)
				cellLife[i] = 0;
            ++i;
        }
}

