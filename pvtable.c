#include "defs.h"
#include <stdint.h>


// Alterantive to % (Modulo) operator. 4x faster.
uint32_t reduce(uint32_t x, uint32_t N) {
  return ((uint64_t) x * (uint64_t) N) >> 32 ;
}

int GetPvLine(const int depth, S_BOARD *pos) {

	ASSERT(depth < MAXDEPTH && depth >= 1);

	int move = ProbePvTable(pos);
	int count = 0;
	
	while(move != NOMOVE && count < depth) {
	
		ASSERT(count < MAXDEPTH);
	
		if( MoveExists(pos, move) ) {
			MakeMove(pos, move);
			pos->PvArray[count++] = move;
		} else {
			break;
		}		
		move = ProbePvTable(pos);
	}
	
	while(pos->ply > 0) {
		TakeMove(pos);
	}
	
	return count;
	
}


const int PvSize = 0x100000 * 2;

void ClearPvTable(S_PVTABLE *table){
    S_PVENTRY *pvEntry;

    for (pvEntry = table->ptable; pvEntry < table->ptable + table->numEntries;pvEntry++){
        pvEntry->posKey = 0ULL;
        pvEntry->move = 0;
    }
}

void InitPvTable(S_PVTABLE *table){
    table->numEntries = PvSize / sizeof(S_PVENTRY);
    table->numEntries -= 2;
    table->ptable = NULL;
    free(table->ptable);
    table->ptable = (S_PVENTRY *) malloc(table->numEntries * sizeof(S_PVENTRY));
    ClearPvTable(table);
    printf("PvTable init complete with %d entries\n", table->numEntries);
}

void StorePvMove (S_BOARD *pos, const int move){
    int index = pos->posKey % pos->PvTable->numEntries;
    ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);

    pos->PvTable->ptable[index].move = move;
    pos->PvTable->ptable[index].posKey = pos->posKey;
}

int ProbePvTable (const S_BOARD *pos){
    int index = pos->posKey % pos->PvTable->numEntries;
    ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);
    
    if (pos->PvTable->ptable[index].posKey == pos->posKey){
        return pos->PvTable->ptable[index].move;
    }
    return NOMOVE;
}