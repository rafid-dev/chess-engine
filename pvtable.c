#include "defs.h"
#include <stdint.h>

S_HASHTABLE HashTable[1];

int GetPvLine(const int depth, S_BOARD *pos, const S_HASHTABLE* table)
{

    ASSERT(depth < MAXDEPTH && depth >= 1);

    int move = ProbePvMove(pos, table);
    int count = 0;

    while (move != NOMOVE && count < depth)
    {

        ASSERT(count < MAXDEPTH);

        if (MoveExists(pos, move))
        {
            MakeMove(pos, move);
            pos->PvArray[count++] = move;
        }
        else
        {
            break;
        }
        move = ProbePvMove(pos, table);
    }

    while (pos->ply > 0)
    {
        TakeMove(pos);
    }

    return count;
}

const int PvSize = 0x100000 * 16;

void ClearHashTable(S_HASHTABLE *table)
{

    S_HASHENTRY *tableEntry;

    for (tableEntry = table->ptable; tableEntry < table->ptable + table->numEntries; tableEntry++)
    {
        tableEntry->posKey = 0ULL;
        tableEntry->move = NOMOVE;
        tableEntry->depth = 0;
        tableEntry->score = 0;
        tableEntry->flags = 0;
        tableEntry->age = 0;
    }
    table->newWrite = 0;
    table->currentAge = 0;
}

void InitHashTable(S_HASHTABLE *table, const int MB)
{
    int HashSize = 0x100000 * MB;
    table->numEntries = HashSize / sizeof(S_HASHENTRY);
    table->numEntries -= 2;
    if (table->ptable != NULL)
    {
        free(table->ptable);
    }
    table->ptable = (S_HASHENTRY *)malloc(table->numEntries * sizeof(S_HASHENTRY));
    if (table->ptable == NULL)
    {
        printf("Hash Allocation Failed, trying %dMB..\n", MB/2);
        InitHashTable(table, MB/2);
    }
    else
    {
        ClearHashTable(table);
        printf("HashTable init complete with %d entries\n", table->numEntries);
    }
}

void StoreHashEntry(S_BOARD *pos, S_HASHTABLE* table, const int move, int score, const int flags, const int depth, const int eval)
{

    int index = pos->posKey % table->numEntries;

    ASSERT(index >= 0 && index <= table->numEntries - 1);
    ASSERT(depth >= 1 && depth < MAXDEPTH);
    ASSERT(flags >= HFALPHA && flags <= HFEXACT);
    ASSERT(score >= -INF_BOUND && score <= INF_BOUND);
    ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

    int replace = FALSE;

    if (table->ptable[index].posKey == 0)
    {
        table->newWrite++;
        replace = TRUE;
    }
    else
    {
        if (table->ptable[index].age < table->currentAge || table->ptable[index].depth <= depth){
            replace = TRUE;
        }
    }

    if (replace == FALSE){
        return;
    }

    if (score > ISMATE)
        score += pos->ply;
    else if (score < -ISMATE)
        score -= pos->ply;

    table->ptable[index].move = move;
    table->ptable[index].posKey = pos->posKey;
    table->ptable[index].flags = flags;
    table->ptable[index].score = score;
    table->ptable[index].depth = depth;
    table->ptable[index].age = table->currentAge;
    table->ptable[index].eval = eval;
}

int ProbeHashEntry(S_BOARD *pos, S_HASHTABLE* table, int *move, S_HASHENTRY *tte, int alpha, int beta, int depth)
{

    int index =pos->posKey % table->numEntries;

    ASSERT(index >= 0 && index <= table->numEntries - 1);
    ASSERT(depth >= 1 && depth < MAXDEPTH);
    ASSERT(alpha < beta);
    ASSERT(alpha >= -INF_BOUND && alpha <= INF_BOUND);
    ASSERT(beta >= -INF_BOUND && beta <= INF_BOUND);
    ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

    if (table->ptable[index].posKey == pos->posKey)
    {
        *move = table->ptable[index].move;
        tte->flags = table->ptable[index].flags;
        tte->depth = table->ptable[index].depth;
        tte->eval = table->ptable[index].eval;

        ASSERT(table->ptable[index].depth >= 1 && table->ptable[index].depth < MAXDEPTH);
        ASSERT(table->ptable[index].flags >= HFALPHA && table->ptable[index].flags <= HFEXACT);

        tte->score = table->ptable[index].score;
        
        if (tte->score > ISMATE)
            tte->score -= pos->ply;
        else if (tte->score < -ISMATE)
            tte->score += pos->ply;

        return TRUE;
    }

    return FALSE;
}

int ProbePvMove(const S_BOARD *pos, const S_HASHTABLE* table)
{

    int index = pos->posKey % table->numEntries;
    ASSERT(index >= 0 && index <= table->numEntries - 1);

    if (table->ptable[index].posKey == pos->posKey)
    {
        return table->ptable[index].move;
    }

    return NOMOVE;
}