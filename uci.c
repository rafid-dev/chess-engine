#include <string.h>
#include "defs.h"
#include "tinycthread.h"

#define INPUTBUFFER 400 * 6

thrd_t mainSearchThread;

thrd_t LaunchSearchThread(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table){
    S_SEARCH_THREAD_DATA *pSearchData = malloc(sizeof(S_SEARCH_THREAD_DATA));

    pSearchData->originalPosition = pos;
    pSearchData->info = info;
    pSearchData->ttable = table;

    thrd_t th;
    thrd_create(&th, &SearchPosition_Thread, (void*)pSearchData);

    return th;
}

void JoinSearchThread(S_SEARCHINFO* info){
    info->stopped = TRUE;
    thrd_join(mainSearchThread, NULL);
}

void ParseGo(char* line, S_SEARCHINFO *info, S_BOARD *pos, S_HASHTABLE *table) {

	int depth = -1, movestogo = 30,movetime = -1;
	int time = -1, inc = 0;
    char *ptr = NULL;
	info->timeset = FALSE;

	if ((ptr = strstr(line,"infinite"))) {
		depth = -1;
	}

	if ((ptr = strstr(line,"binc")) && pos->side == BLACK) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line,"winc")) && pos->side == WHITE) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line,"wtime")) && pos->side == WHITE) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line,"btime")) && pos->side == BLACK) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line,"movestogo"))) {
		movestogo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line,"movetime"))) {
		movetime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line,"depth"))) {
		depth = atoi(ptr + 6);
	}

	if(movetime != -1) {
		time = movetime;
		movestogo = 1;
	}

	info->starttime = GetTimeMs();
	info->depth = depth;

	if(time != -1) {
		info->timeset = TRUE;
		time /= movestogo;
        
        // Hack to fix illegal move (empty move)
		if (time > 1500) time -= 50;

		info->stoptime = info->starttime + time + inc;
	}

	if(depth == -1) {
		info->depth = MAXDEPTH;
	}

	printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
		time,info->starttime,info->stoptime,info->depth,info->timeset);
	//SearchPosition(pos, info, HashTable);
    mainSearchThread = LaunchSearchThread(pos, info, table);
}

void ParsePosition(char* lineIn, S_BOARD* pos){
    // shift pointer to the right where next token begins
    lineIn += 9;
    
    // init pointer to the current character in the command string
    char *current_char = lineIn;
    
    // parse UCI "startpos" command
    if (strncmp(lineIn, "startpos", 8) == 0)
        // init chess board with start position
        parse_fen(START_FEN, pos);
    
    // parse UCI "fen" command 
    else
    {
        // make sure "fen" command is available within command string
        current_char = strstr(lineIn, "fen");
        
        // if no "fen" command is available within command string
        if (current_char == NULL)
            // init chess board with start position
            parse_fen(START_FEN, pos);
            
        // found "fen" substring
        else
        {
            // shift pointer to the right where next token begins
            current_char += 4;
            
            // init chess board with position from FEN string
            parse_fen(current_char, pos);
        }
    }
    
    // parse moves after position
    current_char = strstr(lineIn, "moves");
    
    // moves available
    if (current_char != NULL)
    {
        // shift pointer to the ri ght where next token begins
        current_char += 6;
        
        // loop over moves within a move string
        while(*current_char)
        {
            // parse next move
            int move = ParseMove(current_char, pos);
            
            // if no more moves
            if (move == 0)
                // break out of the loop
                break;
            
            make_move(pos, move);
            
            // move current character mointer to the end of current move
            while (*current_char && *current_char != ' ') current_char++;
            
            // go to the next move
            current_char++;
        }        
    }
    print_board(pos);

}


void Uci_Loop (){
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char line[INPUTBUFFER];
    printf("id name %s\n", NAME);
    printf("id author Slender\n");
    printf("option name Hash type spin default 64 min 4 max %d\n",64);
    printf("uciok\n");

    S_BOARD pos[1];
    S_SEARCHINFO info[1];
    HashTable->ptable = NULL;
    info->quit = 0;
    int MB = 64;
    InitHashTable(HashTable, MB);

    while (TRUE){
        memset(&line[0], 0,sizeof(line));
        fflush(stdout);
        if (!fgets(line, INPUTBUFFER, stdin)){
            continue;
        }
        if (line[0] == '\n'){
            continue;
        }
        if (!strncmp(line, "run", 3)){
            parse_fen(START_FEN, pos);
            ParseGo("go infinite", info, pos, HashTable);
        }else if (!strncmp(line, "eval", 4)){
            printf("%d\n", EvalPosition(pos));
        }else if (!strncmp(line, "perft", 5)){
            PerftTest(5, pos);
        }else if (!strncmp(line, "print", 5)){
	        print_board(pos);
        }else if (!strncmp(line, "isready", 7)){
            printf("readyok\n");
            continue;
        }else if (!strncmp(line, "position", 8)){
            ParsePosition(line, pos);
        }else if (!strncmp(line, "ucinewgame", 10)){
            ClearHashTable(HashTable);
            ParsePosition("position startpos\n", pos);
        }else if (!strncmp(line, "go", 2)){
            ParseGo(line, info, pos, HashTable);
        }else if (!strncmp(line, "quit", 4)){

            info->quit = TRUE;
            JoinSearchThread(info);

            break;
        }else if (!strncmp(line, "stop", 4)){

            JoinSearchThread(info);

        }else if (!strncmp(line, "uci", 3)){

            printf("id name %s\n", NAME);
            printf("id author Slender\n");
            printf("option name Hash type spin default 64 min 4 max %d\n",64);
            printf("uciok\n");

        } else if (!strncmp(line, "setoption name Hash value ", 26)) {			

			sscanf(line,"%*s %*s %*s %*s %d",&MB);
			if(MB < 4) MB = 4;
			if(MB > 64) MB = 64;
			printf("Set Hash to %d MB\n",MB);
			InitHashTable(HashTable, MB);

        }
        if (info->quit){
            break;
        }
    }

    free(HashTable->ptable);
}