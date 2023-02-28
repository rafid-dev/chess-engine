#include <stdio.h>
#include <string.h>
#include "defs.h"

int see_values[13] = {0, 100, 300, 300, 500, 1000, 0,
                      100, 300, 300, 500, 1000, 0};

static inline int swap_off(S_BOARD *pos, S_ATTACKLIST *atks, S_ATTACKLIST *defs, int s, int sv)
{
    int value = 0;
    int ai; // attack index
    int di; // def index
    int av; // attack value
    int dv; // def value

    if (!atks->count)
    {
        return 0;
    }

    if (!defs->count)
    {
        return sv;
    }

    ai = atks->count - 1;
    di = defs->count - 1;
    av = see_values[pos->pieces[atks->square[ai]]];
    dv = see_values[pos->pieces[defs->square[di]]];

    if (((ai > di) && (av == dv)) || (av < dv))
    {
        atks->count--;
        value = sv;
        value -= swap_off(pos, defs, atks, s, av);
    }

    // printf("returning value %d\n", value);
    return value;
}

static inline int see(S_BOARD *pos, int move)
{
    int f = FROMSQ(move);
    int t = TOSQ(move);
    int side = pos->side;
    int value;

    S_ATTACKLIST atks[1];
    S_ATTACKLIST defs[1];

    get_attackers(pos, side ^ 1, OFFBOARD, t, atks);
    get_attackers(pos, side, f, t, defs);
    value = see_values[pos->pieces[t]];
    value -= swap_off(pos, atks, defs, f, see_values[pos->pieces[t]]);

    return value;
}

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

    table->overWrite = 0;
    table->hit = 0;
    table->cut = 0;
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

    if ((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply)
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
    int ttescore = -INF_BOUND;
    int tteflag = HFNONE;
    int tteDepth = 0;
    int tteEval = 0;
    int score = -INF_BOUND;
    int isPvNode = beta - alpha != 1;
    int PvMove = NOMOVE;
    
    int eval;
    int excluded = ss->excluded;
    int ttHit = FALSE;   

    if (!excluded && ProbeHashEntry(pos, table, &PvMove, &ttescore, &tteflag, &tteDepth, &ttHit, &tteEval, alpha, beta, depth) == TRUE)
    {
        table->cut++;
        return ttescore;
    }
    
    ss->staticEval = eval = ttHit ? tteEval : EvalPosition(pos);
    

    int improving = (pos->ply > 0) && (!InCheck && ss->staticEval > (ss - 1)->staticEval);

    if (!isPvNode && !InCheck && pos->ply)
    {
         if (ttHit){
             eval = ttescore;
         }

        /* Reverse Futility Pruning (RFP) */
        if (depth <= 6 && eval >= beta && eval - (depth*75) >= beta && eval < ISMATE)
        {
            return eval;
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
        // // Razoring
        // if ((depth <= 3) && (posEval + 119 + 182 * (depth - 1) <= alpha)){
        //     return Quiescence(alpha, beta, pos, info);
        // }
    }

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;
    int MovesSearched = 0;
    int quietsSearched = 0;
    int OldAlpha = alpha;
    int BestMove = NOMOVE;
    int bestscore = -INF_BOUND;
    int LMPCount = depth * 8;

    score = -INF_BOUND;

    if (PvMove != NOMOVE)
    {
        for (MoveNum = 0; MoveNum < list->count; ++MoveNum)
        {
            if (list->moves[MoveNum].move == PvMove)
            {
                list->moves[MoveNum].score = 2000000;
                break;
            }
        }
    }
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum)
    {
        PickNextMove(MoveNum, list);

        int move = list->moves[MoveNum].move;
        // if (move==ss->excluded)continue;

        int skipQuiets = FALSE;
        int isQuiet = (!(move & MFLAGCAP) && !(move & MFLAGPROM));
        int extension = 0;

        /*if (!isRoot){
            // Singular Extension
            if ((excluded == NOMOVE) && depth >= 8 && (move == PvMove) && (tteflag == HFBETA) && abs(ttescore) < ISMATE && tteDepth >= depth - 2){
                int singularBeta = ttescore - 2 * depth;
                int singularDepth = (depth - 1) / 2;

                ss->excluded = PvMove;
                int singularScore = AlphaBeta(singularBeta - 1, singularBeta, singularDepth, pos, info, table, ss, TRUE);
                ss->excluded = NOMOVE;

                if (singularScore < singularBeta){
                    extension = 1;
                }
            }
        }*/
        if (isQuiet)
        {

            // Late Move Pruning/Movecount pruning
            if (!isPvNode && !InCheck && depth < 4 && (quietsSearched >= depth * 8))
            {
                continue;
            }
        }

        int newDepth = depth + extension;
        if (!MakeMove(pos, move))
        {
            continue;
        }

        info->nodes++;
        MovesSearched++;

        if (isQuiet && skipQuiets)
            continue;
        if (isQuiet)
        {
            quietsSearched++;
        }

        int do_fullsearch = FALSE;
        int histScore = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];

        // Late Move Reduction (LMR)
        if (!InCheck && depth >= 3 && MovesSearched >= 5 && isQuiet)
        {

            int reduction = LMRTable[MIN(depth, 64)][MAX(MovesSearched, 63)];
            reduction = MIN(depth - 1, MAX(1, reduction));

            score = -AlphaBeta(-alpha - 1, -alpha, newDepth - reduction, pos, info, table, ss, TRUE);

            do_fullsearch = score > alpha && reduction != 1;
        }
        else
        {
            do_fullsearch = !isPvNode || MovesSearched > 1;
        }

        // Full depth search on a null window
        if (do_fullsearch)
        {
            score = -AlphaBeta(-alpha - 1, -alpha, newDepth - 1, pos, info, table, ss, TRUE);
        }

        // Principal Variation Search (PVS)
        if (isPvNode && (MovesSearched == 1 || score > alpha))
        {
            score = -AlphaBeta(-beta, -alpha, newDepth - 1, pos, info, table, ss, TRUE);
        }

        TakeMove(pos);

        if (!isRoot && info->stopped == TRUE)
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
                        // Update killers
                        pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
                        pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
                    }

                    StoreHashEntry(pos, table, BestMove, beta, HFBETA, depth, ss->staticEval);

                    return beta;
                }
                alpha = score;
                if (isQuiet)
                {
                    pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
                }
            }
        }
        if (isRoot && info->stopped == TRUE && BestMove != NOMOVE)
        {
            break;
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

    int f = bestscore >= beta ? HFBETA : (alpha != OldAlpha) ? HFEXACT
                                                             : HFALPHA;

    if (!excluded)
        StoreHashEntry(pos, table, BestMove, bestscore, f, depth, ss->staticEval);

    return alpha;
}

int SearchPosition_Thread(void *data)
{
    S_SEARCH_THREAD_DATA *searchdata = (S_SEARCH_THREAD_DATA *)data;
    S_BOARD *pos = malloc(sizeof(S_BOARD));
    memcpy(pos, searchdata->originalPosition, sizeof(S_BOARD));
    SearchPosition(pos, searchdata->info, searchdata->ttable);
    free(pos);
    return 0;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table)
{
    // iterative deepening, search init
    CheckBoard(pos);

    S_STACK stack[MAXPLY + 10];
    S_STACK *ss = stack + 7;

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

        printf("info score cp %d depth %d nodes %ld time %d ", bestscore, currentDepth, info->nodes, GetTimeMs() - info->starttime);
        printf("pv");
        for (pvNum = 0; pvNum < pvMoves; ++pvNum)
        {
            printf(" %s", PrMove(pos->PvArray[pvNum]));
        }
        printf("\n");
    }
    printf("bestmove %s\n", PrMove(bestmove));
}
