#include <stdio.h>
#include <string.h>

#include "defs.h"

long leafNodes;

void Perft(int depth, S_BOARD *pos) {


	if(depth == 0) {
        leafNodes++;
        return;
    }	

    S_MOVELIST list[1];
    generate_moves(list, pos);
      
    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {	
        if ( !make_move(pos,list->moves[MoveNum].move)){
            take_move(pos);
            continue;
        }
        Perft(depth - 1, pos);
        take_move(pos);
    }

    return;
}

long PerftTest(int depth, S_BOARD *pos) {


	print_board(pos);
	printf("\nStarting Test To Depth:%d\n",depth);	
	leafNodes = 0;
    int start = GetTimeMs();
    S_MOVELIST list[1];
    generate_moves(list, pos);	


    int move;	    
    int MoveNum = 0;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

        move = list->moves[MoveNum].move;

        if ( !make_move(pos,move))  {
            take_move(pos);
            continue;
        }


        long cumnodes = leafNodes;
        Perft(depth - 1, pos);

        take_move(pos);
        

        long oldnodes = leafNodes - cumnodes;
        printf("%s : %ld\n",PrMove(move),oldnodes);
    }
	
	printf("\nTest Complete : %ld nodes visited in %dms\n",leafNodes, GetTimeMs() - start);

    return leafNodes;
}

void PerftSuite(int target_depth, S_BOARD *pos, int line_start){
    FILE* perft_files = fopen("perfomancetest.txt", "r");
    if (perft_files == NULL){
        printf("unable to open file\n");
        return;
    }
    char line[512];
    int line_number = 0;
    while (fgets(line, sizeof(line), perft_files)){
        line_number++;
        if (line_number <= line_start){
            continue;
        }
        char* token;
        char* remaining;

        token = strtok(line, ",");

        parse_fen(token, pos);
        printf("Parsed FEN %s\n", token);

        long target_nodes;
        int node_depth = 0;
        while (node_depth < target_depth){
            token = strtok(NULL, ",");
            node_depth++;
        }
        target_nodes = strtol(token, &remaining, 10);

        printf("Starting Perft Test. Target Nodes: %ld Target depth:%d\n", target_nodes, target_depth);

        long nodes_reached = PerftTest(target_depth, pos);
        if (nodes_reached != target_nodes){
            printf("Perft test failed. Expected %ld nodes but got %ld\n", target_nodes, nodes_reached);
            break;
        }
        printf("Test %d passed!\n", line_number);
    }
}