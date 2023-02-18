#include <stdio.h>
#include "defs.h"

static void CheckUp(S_SEARCHINFO *info)
{
    // check if time up or interupt from GUI

    if (info->timeset == TRUE && GetTimeMs() > info->stoptime)
    {
        info->stopped = TRUE;
    }

    ReadInput(info);
}

static void PickNextMove(int movenum, S_MOVELIST *list)
{
    S_MOVE temp;
    int index = 0;
    int bestscore = 0;
    int bestnum = movenum;

    for (index = movenum; index < list->count; ++index)
    {
        if (list->moves[index].score > bestscore)
        {
            bestscore = list->moves[index].score;
            bestnum = index;
        }
    }

    temp = list->moves[movenum];
    list->moves[movenum] = list->moves[bestnum];
    list->moves[bestnum] = temp;
}

int IsRepetition(const S_BOARD *pos)
{
    int index = 0;

    for (index = pos->hisPly - pos->fiftyMove; index < pos->hisPly - 1; ++index)
    {
        ASSERT(index >= 0 && index < MAXGAMEMOVES);
        if (pos->posKey == pos->history[index].posKey)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info)
{
    int index = 0;
    int index2 = 0;

    for (index = 0; index < 13; ++index)
    {
        for (index2 = 0; index2 < BRD_SQ_NUM; ++index2)
        {
            pos->searchHistory[index][index2] = 0;
        }
    }

    for (index = 0; index < 2; ++index)
    {
        for (index2 = 0; index2 < MAXDEPTH; ++index2)
        {
            pos->searchKillers[index][index2] = 0;
        }
    }

    pos->HashTable->overWrite=0;
	pos->HashTable->hit=0;
	pos->HashTable->cut=0;
	pos->ply = 0;

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info)
{
    ASSERT(CheckBoard(pos));

    if ((info->nodes & 2047) == 0)
    {
        CheckUp(info);
    }

    if (IsRepetition(pos) || pos->fiftyMove >= 100)
    {
        return 0;
    }

    if (pos->ply > MAXDEPTH - 1)
    {
        return EvalPosition(pos);
    }

    int score = EvalPosition(pos);

    if (score >= beta)
    {
        return beta;
    }

    if (score > alpha)
    {
        alpha = score;
    }

    S_MOVELIST list[1];
    GenerateAllCaps(pos, list);

    // int MoveNum = 0;
    int Legal = 0;
    int OldAlpha = alpha;
    int BestMove = NOMOVE;
    score = -INF_BOUND;

    for (int MoveNum = 0; MoveNum < list->count; ++MoveNum)
    {
        PickNextMove(MoveNum, list);
        if (!MakeMove(pos, list->moves[MoveNum].move))
        {
            continue;
        }

        Legal++;
        score = -Quiescence(-beta, -alpha, pos, info);
        TakeMove(pos);

        if (info->stopped == TRUE)
        {
            return 0;
        }

        if (score > alpha)
        {
            if (score >= beta)
            {
                if (Legal == 1)
                {
                    info->fhf++;
                }
                info->fh++;
                return beta;
            }
            alpha = score;
            BestMove = list->moves[MoveNum].move;
        }
    }

    return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull)
{
    ASSERT(CheckBoard(pos));

    if (depth == 0)
    {
        info->nodes++;
        return Quiescence(alpha, beta, pos, info);
    }
    if ((info->nodes & 2047) == 0)
    {
        CheckUp(info);
    }
    info->nodes++;

    if (IsRepetition(pos) || pos->fiftyMove >= 100 && pos->ply)
    {
        return 0;
    }

    if (pos->ply > MAXDEPTH - 1)
    {
        return EvalPosition(pos);
    }

    int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos);

    if (InCheck == TRUE)
    {
        depth++;
    }

    int score = -INF_BOUND;

    int PvMove = NOMOVE;
    if( ProbeHashEntry(pos, &PvMove, &score, alpha, beta, depth) == TRUE ) {
		pos->HashTable->cut++;
		return score;
	}

    if (DoNull && !InCheck && pos->ply && (pos->bigPiece[pos->side] > 1) && depth >= 4)
    {
        MakeNullMove(pos);
        score = -AlphaBeta(-beta, -beta + 1, depth - 4, pos, info, FALSE);
        TakeNullMove(pos);
        if (info->stopped == TRUE)
        {
            return 0;
        }
        if (score >= beta && abs(score) < ISMATE)
        {
            return beta;
        }
    }

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;
    int Legal = 0;
    int OldAlpha = alpha;
    int BestMove = NOMOVE;
    int bestscore = -INF_BOUND;
    score = -INF_BOUND;

    if( PvMove != NOMOVE) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				break;
			}
		}
	}

    for (MoveNum = 0; MoveNum < list->count; ++MoveNum)
    {
        PickNextMove(MoveNum, list);
        if (!MakeMove(pos, list->moves[MoveNum].move))
        {
            continue;
        }

        Legal++;
        score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, TRUE);
        TakeMove(pos);

        if (info->stopped == TRUE)
        {
            return 0;
        }

        if (score > bestscore)
        {
            bestscore = score;
            BestMove = list->moves[MoveNum].move;
            if (score > alpha)
            {
                if (score >= beta)
                {
                    if (Legal == 1)
                    {
                        info->fhf++;
                    }
                    info->fh++;

                    if (!(list->moves[MoveNum].move & MFLAGCAP))
                    {
                        pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
                        pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
                    }

                    StoreHashEntry(pos, BestMove, beta, HFBETA, depth);

                    return beta;
                }
                alpha = score;
                if (!(list->moves[MoveNum].move & MFLAGCAP))
                {
                    pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
                }
            }
        }
    }

    if (Legal == 0)
    {
        if (InCheck == TRUE)
        {
            return -29000 + pos->ply;
        }
        else
        {
            return 0;
        }
    }

    if (alpha != OldAlpha)
    {
        StoreHashEntry(pos, BestMove, bestscore, HFEXACT, depth);
    }else{
        StoreHashEntry(pos, BestMove, bestscore, HFALPHA, depth);
    }

    return alpha;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info)
{
    // iterative deepening, search init
    CheckBoard(pos);

    int bestmove = NOMOVE;
    int bestscore = -INF_BOUND;
    int currentDepth = 0;
    int pvMoves = 0;
    int pvNum = 0;
    ClearForSearch(pos, info);

    for (currentDepth = 1; currentDepth <= info->depth; currentDepth++)
    {
        bestscore = AlphaBeta(-INF_BOUND, INF_BOUND, currentDepth, pos, info, TRUE);
        if (info->stopped == TRUE)
        {
            break;
        }

        pvMoves = GetPvLine(currentDepth, pos);
        bestmove = pos->PvArray[0];

        printf("info score cp %d depth %d nodes %ld time %d ", bestscore, currentDepth, info->nodes, GetTimeMs() - info->starttime);

        pvMoves = GetPvLine(currentDepth, pos);
        printf("pv");
        for (pvNum = 0; pvNum < pvMoves; ++pvNum)
        {
            printf(" %s", PrMove(pos->PvArray[pvNum]));
        }
        printf("\n");
    }
    printf("bestmove %s\n", PrMove(bestmove));
}