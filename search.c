#include <stdio.h>
#include <string.h>
#include "defs.h"

static inline void CheckUp(S_SEARCHINFO *info)
{
    // check if time up or interupt from GUI

    if (info->timeset == TRUE && GetTimeMs() > info->stoptime)
    {
        info->stopped = TRUE;
    }
}

static inline void PickNextMove(int movenum, S_MOVELIST *list)
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

static inline int IsRepetition(const S_BOARD *pos)
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

static inline void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table)
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

    table->overWrite=0;
	table->hit=0;
	table->cut=0;
    table->currentAge++;

	pos->ply = 0;

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static inline int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info)
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
    int MovesSearched = 0;
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
        info->nodes++;
        MovesSearched++;
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
                if (MovesSearched == 1)
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

static inline int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table, S_STACK *ss, int DoNull)
{
    ASSERT(CheckBoard(pos));

    if (depth == 0)
    {
        return Quiescence(alpha, beta, pos, info);
    }
    if ((info->nodes & 2047) == 0)
    {
        CheckUp(info);
    }
    

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

    int isRoot = (pos->ply == 0);
    int score = -INF_BOUND;
    int isPvNode = beta-alpha != 1;
    int PvMove = NOMOVE;    
    int posEval = EvalPosition(pos);

    if (!InCheck)
        ss->eval = posEval;
    
    bool improving = !InCheck && posEval > (ss-2)->eval;

    if( ProbeHashEntry(pos, table, &PvMove, &score, alpha, beta, depth) == TRUE ) {
		table->cut++;
		return score;
	}

    if (!isPvNode && !InCheck && pos->ply){
        /* Reverse Futility Pruning (RFP) */
        if (depth <= 5 && posEval >= beta && posEval - (depth * 75) >= beta && posEval < ISMATE)
        {
            return posEval;
        }
        /* NULL Move Pruning (NMP) */
        if ((pos->bigPiece[pos->side] > 1) && depth >= 4)
        {
            MakeNullMove(pos);
            score = -AlphaBeta(-beta, -beta + 1, depth - 4, pos, info, table, ss, FALSE);
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
    }

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;
    int MovesSearched = 0;
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

        int skipQuiets = FALSE;
        int isQuiet = !(list->moves[MoveNum].move & MFLAGCAP);
        // Late move pruning
        if (!isRoot && depth <= 4 && MovesSearched > depth*8){
            skipQuiets = TRUE;
        }

        if (skipQuiets && isQuiet){
            continue;
        }

        if (!MakeMove(pos, list->moves[MoveNum].move))
        {
            continue;
        }

        info->nodes++;
        MovesSearched++;
        int move = list->moves[MoveNum].move;
        int do_fullsearch = FALSE;
        int histScore = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];

        // Late Move Reduction (LMR)
        if (!InCheck && depth >= 3 && MovesSearched >= 5 && isQuiet){

            int reduction = LMRTable[MIN(depth, 64)][MAX(MovesSearched, 63)];
            reduction = MIN(depth - 1, MAX(1, reduction));

            score = -AlphaBeta(-alpha-1, -alpha, depth - reduction, pos, info, table, ss, TRUE);

            do_fullsearch = score > alpha && reduction != 1;

        }else{
            do_fullsearch = !isPvNode || MovesSearched > 1;
        }

        // Full depth search on a null window
        if (do_fullsearch){
            score = -AlphaBeta(-alpha-1, -alpha, depth - 1, pos, info, table, ss, TRUE);
        }

        // Principal Variation Search (PVS)
        if (isPvNode && (MovesSearched == 1 || score > alpha)){
            score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, table, ss, TRUE);
        }

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
                    if (MovesSearched == 1)
                    {
                        info->fhf++;
                    }
                    info->fh++;

                    if (isQuiet)
                    {
                        pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
                        pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
                    }

                    StoreHashEntry(pos,table, BestMove, beta, HFBETA, depth);

                    return beta;
                }
                alpha = score;
                if (isQuiet)
                {
                    pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
                }
            }
        }
    }

    if (MovesSearched == 0)
    {
        if (InCheck == TRUE)
        {
            return -ISMATE + pos->ply;
        }
        else
        {
            return 0;
        }
    }

    if (alpha != OldAlpha)
    {
        StoreHashEntry(pos, table, BestMove, bestscore, HFEXACT, depth);
    }else{
        StoreHashEntry(pos, table, BestMove, bestscore, HFALPHA, depth);
    }

    return alpha;
}

int SearchPosition_Thread(void *data){
    S_SEARCH_THREAD_DATA *searchdata = (S_SEARCH_THREAD_DATA *)data;
    S_BOARD *pos = malloc(sizeof(S_BOARD));
    memcpy(pos, searchdata->originalPosition, sizeof(S_BOARD));
    SearchPosition(pos, searchdata->info, searchdata->ttable);
    free(pos);
    printf("Freed\n");
    return 0;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table)
{
    // iterative deepening, search init
    CheckBoard(pos);

    S_STACK stack[MAXPLY+10];
    S_STACK *ss = stack+7;

    int bestmove = NOMOVE;
    int bestscore = -INF_BOUND;
    int currentDepth = 0;
    int pvMoves = 0;
    int pvNum = 0;
    ClearForSearch(pos, info, table);

    for (currentDepth = 1; currentDepth <= info->depth; currentDepth++)
    {
        bestscore = AlphaBeta(-INF_BOUND, INF_BOUND, currentDepth, pos, info, table, ss, TRUE);
        if (info->stopped == TRUE)
        {
            break;
        }

        pvMoves = GetPvLine(currentDepth, pos, table);
        bestmove = pos->PvArray[0];

        printf("info score cp %d depth %d nodes %ld time %d pv", bestscore, currentDepth, info->nodes, GetTimeMs() - info->starttime);

        for (pvNum = 0; pvNum < pvMoves; ++pvNum)
        {
            printf(" %s", PrMove(pos->PvArray[pvNum]));
        }
        printf("\n");
    }
    printf("bestmove %s\n", PrMove(bestmove));
}