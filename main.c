#include <stdio.h>
#include "defs.h"

int main (){

    AllInit();

    S_BOARD pos[1];

    ParseFen(START_FEN, pos);

    PerftTest(4, pos);
    //Uci_Loop();

    
    return 0;
}