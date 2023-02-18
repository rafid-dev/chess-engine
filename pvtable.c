#include "defs.h"
#include <stdint.h>

// Alterantive to % (Modulo) operator. 4x faster.
uint32_t reduce(uint32_t x, uint32_t N)
{
    return ((uint64_t)x * (uint64_t)N) >> 32;
}

int GetPvLine(const int depth, S_BOARD *pos)
{

    ASSERT(depth < MAXDEPTH && depth >= 1);

    int move = ProbePvMove(pos);
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
        move = ProbePvMove(pos);
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
    }
    table->newWrite = 0;
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

void StoreHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth)
{

    int index = pos->posKey % pos->HashTable->numEntries;

    ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
    ASSERT(depth >= 1 && depth < MAXDEPTH);
    ASSERT(flags >= HFALPHA && flags <= HFEXACT);
    ASSERT(score >= -INF_BOUND && score <= INF_BOUND);
    ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

    if (pos->HashTable->ptable[index].posKey == 0)
    {
        pos->HashTable->newWrite++;
    }
    else
    {
        pos->HashTable->overWrite++;
    }

    if (score > ISMATE)
        score += pos->ply;
    else if (score < -ISMATE)
        score -= pos->ply;

    pos->HashTable->ptable[index].move = move;
    pos->HashTable->ptable[index].posKey = pos->posKey;
    pos->HashTable->ptable[index].flags = flags;
    pos->HashTable->ptable[index].score = score;
    pos->HashTable->ptable[index].depth = depth;
}

int ProbePvMove(const S_BOARD *pos)
{

    int index = pos->posKey % pos->HashTable->numEntries;
    ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);

    if (pos->HashTable->ptable[index].posKey == pos->posKey)
    {
        return pos->HashTable->ptable[index].move;
    }

    return NOMOVE;
}

int ProbeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth)
{

    int index = pos->posKey % pos->HashTable->numEntries;

    ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
    ASSERT(depth >= 1 && depth < MAXDEPTH);
    ASSERT(alpha < beta);
    ASSERT(alpha >= -INF_BOUND && alpha <= INF_BOUND);
    ASSERT(beta >= -INF_BOUND && beta <= INF_BOUND);
    ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

    if (pos->HashTable->ptable[index].posKey == pos->posKey)
    {
        *move = pos->HashTable->ptable[index].move;
        if (pos->HashTable->ptable[index].depth >= depth)
        {
            pos->HashTable->hit++;

            ASSERT(pos->HashTable->ptable[index].depth >= 1 && pos->HashTable->ptable[index].depth < MAXDEPTH);
            ASSERT(pos->HashTable->ptable[index].flags >= HFALPHA && pos->HashTable->ptable[index].flags <= HFEXACT);

            *score = pos->HashTable->ptable[index].score;
            if (*score > ISMATE)
                *score -= pos->ply;
            else if (*score < -ISMATE)
                *score += pos->ply;

            switch (pos->HashTable->ptable[index].flags)
            {
            case HFALPHA:
                if (*score <= alpha)
                {
                    *score = alpha;
                    return TRUE;
                }
                break;
            case HFBETA:
                if (*score >= beta)
                {
                    *score = beta;
                    return TRUE;
                }
                break;
            case HFEXACT:
                return TRUE;
                break;
            default:
                ASSERT(FALSE);
                break;
            }
        }
    }

    return FALSE;
}
