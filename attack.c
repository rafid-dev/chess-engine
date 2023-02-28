#include "defs.h"

const int KnDir[8] = { -8, -19,	-21, -12, 8, 19, 21, 12 };
const int RkDir[4] = { -1, -10,	1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 };

int SqAttacked(const int sq, const int side, const S_BOARD *pos) {

	int pce,index,t_sq,dir;
	
	ASSERT(SqOnBoard(sq));
	ASSERT(SideValid(side));
	ASSERT(CheckBoard(pos));
	
	// pawns
	if(side == WHITE) {
		if(pos->pieces[sq-11] == wP || pos->pieces[sq-9] == wP) {
			return TRUE;
		}
	} else {
		if(pos->pieces[sq+11] == bP || pos->pieces[sq+9] == bP) {
			return TRUE;
		}	
	}
	
	// knights
	for(index = 0; index < 8; ++index) {		
		pce = pos->pieces[sq + KnDir[index]];
		ASSERT(PceValidEmptyOffbrd(pce));
		if(pce != OFFBOARD && IsKn(pce) && PieceCol[pce]==side) {
			return TRUE;
		}
	}
	
	// rooks, queens
	for(index = 0; index < 4; ++index) {		
		dir = RkDir[index];
		t_sq = sq + dir;
		ASSERT(SqIs120(t_sq));
		pce = pos->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(pce));
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsRQ(pce) && PieceCol[pce] == side) {
					return TRUE;
				}
				break;
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
		}
	}
	
	// bishops, queens
	for(index = 0; index < 4; ++index) {		
		dir = BiDir[index];
		t_sq = sq + dir;
		ASSERT(SqIs120(t_sq));
		pce = pos->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(pce));
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsBQ(pce) && PieceCol[pce] == side) {
					return TRUE;
				}
				break;
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
		}
	}
	
	// kings
	for(index = 0; index < 8; ++index) {		
		pce = pos->pieces[sq + KiDir[index]];
		ASSERT(PceValidEmptyOffbrd(pce));
		if(pce != OFFBOARD && IsKi(pce) && PieceCol[pce]==side) {
			return TRUE;
		}
	}
	
	return FALSE;
	
}

const int PieceDirections[13][8] = {
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {-8, -19, -21, -12, 8, 19, 21, 12},
    {-9, -11, 11, 9, 0, 0, 0, 0},
    {-1, -10, 1, 10, 0, 0, 0, 0},
    {-1, -10, 1, 10, -9, -11, 11, 9},
    {-1, -10, 1, 10, -9, -11, 11, 9},
    {0, 0, 0, 0, 0, 0, 0},
    {-8, -19, -21, -12, 8, 19, 21, 12},
    {-9, -11, 11, 9, 0, 0, 0, 0},
    {-1, -10, 1, 10, 0, 0, 0, 0},
    {-1, -10, 1, 10, -9, -11, 11, 9},
    {-1, -10, 1, 10, -9, -11, 11, 9}};

const int NumDirections[13] = {
    0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8};

int get_attackers(S_BOARD *pos, int side, int from, int to, S_ATTACKLIST *attacklist)
{
    int f = 0;
    int s = to;

    int *atkvec;

    attacklist->count = 0;

    int king = (pos->side == WHITE ? wK : bK);
    int queen = (pos->side == WHITE ? wQ : bQ);
    int rook = (pos->side == WHITE ? wR : bR);
    int bishop = (pos->side == WHITE ? wB : bB);
    int knight = (pos->side == WHITE ? wN : bN);

    int dir;
    int t_sq;
    int pce;
    int num;

    for (num = 0; num <= 1;)
    {
        pce = (num == 0 ? knight : king);
        for (int i = 0; i < NumDirections[pce]; i++)
        {
            dir = PieceDirections[pce][i];
            t_sq = s + dir;

            if (SQOFFBOARD(t_sq))
            {
                continue;
            }

            if (pos->pieces[t_sq] == pce)
            {
                attacklist->square[attacklist->count] = t_sq;
                attacklist->count++;
                continue;
            }
        }
        num++;
    }

    for (int num = 0; num <= 2;)
    {
        pce = (num == 0 ? bishop : (num == 1 ? rook : queen));
        for (int i = 0; i < NumDirections[pce]; i++)
        {
            dir = PieceDirections[pce][i];
            t_sq = s + dir;

            while (!SQOFFBOARD(t_sq))
            {
                if (pos->pieces[t_sq] == pce)
                {
                    attacklist->square[attacklist->count] = t_sq;
                    attacklist->count++;
                    break;
                }
                t_sq += dir;
            }
        }
        num++;
    }

    if (side == WHITE)
    {
        if (pos->pieces[s - 9] == wP)
        {
            attacklist->square[attacklist->count] = s - 9;
            attacklist->count++;
        }
        if (pos->pieces[s - 11] == wP)
        {
            attacklist->square[attacklist->count] = s - 11;
            attacklist->count++;
        }
    }
    else
    {
        if (pos->pieces[s + 9] == bP)
        {
            attacklist->square[attacklist->count] = s + 9;
            attacklist->count++;
        }
        if (pos->pieces[s + 11] == bP)
        {
            attacklist->square[attacklist->count] = s + 11;
            attacklist->count++;
        }
    }
    return 0;
}