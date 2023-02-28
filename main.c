#include <stdio.h>
#include "defs.h"

int main()
{

    AllInit();

    // S_BOARD pos[1];

    // ParseFen("3k4/8/4r3/8/1b6/5NB1/8/3K2Q1 w - - 0 1", pos);

    // S_ATTACKLIST atklist[1];

    // get_attackers(pos, WHITE, OFFBOARD, E1, atklist);

    // printf("%d\n", atklist->count);

     Uci_Loop();

    return 0;
}