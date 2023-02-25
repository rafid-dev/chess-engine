#include <stdio.h>
#include "defs.h"

int CheckBoard(const S_BOARD *pos) {

	int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int t_bigPce[2] = { 0, 0};
	int t_majPce[2] = { 0, 0};
	int t_minPce[2] = { 0, 0};
	int t_material[2] = { 0, 0};

	int sq64,t_piece,t_pce_num,sq120,colour,pcount;

	U64 t_pawns[3] = {0ULL, 0ULL, 0ULL};

	t_pawns[WHITE] = pos->pawns[WHITE];
	t_pawns[BLACK] = pos->pawns[BLACK];
	t_pawns[BOTH] = pos->pawns[BOTH];

	// check piece lists
	for(t_piece = wP; t_piece <= bK; ++t_piece) {
		for(t_pce_num = 0; t_pce_num < pos->pceNum[t_piece]; ++t_pce_num) {
			sq120 = pos->pList[t_piece][t_pce_num];
			ASSERT(pos->pieces[sq120]==t_piece);
		}
	}

	// check piece count and other counters
	for(sq64 = 0; sq64 < 64; ++sq64) {
		sq120 = SQ120(sq64);
		t_piece = pos->pieces[sq120];
		t_pceNum[t_piece]++;
		colour = PieceCol[t_piece];
		if( PieceBig[t_piece] == TRUE) t_bigPce[colour]++;
		if( PieceMin[t_piece] == TRUE) t_minPce[colour]++;
		if( PieceMaj[t_piece] == TRUE) t_majPce[colour]++;

		t_material[colour] += PieceVal[t_piece];
	}

	for(t_piece = wP; t_piece <= bK; ++t_piece) {
		ASSERT(t_pceNum[t_piece]==pos->pceNum[t_piece]);
	}

	// check bitboards count
	pcount = CNT(t_pawns[WHITE]);
	ASSERT(pcount == pos->pceNum[wP]);
	
	pcount = CNT(t_pawns[BLACK]);
	ASSERT(pcount == pos->pceNum[bP]);
	pcount = CNT(t_pawns[BOTH]);
	ASSERT(pcount == (pos->pceNum[bP] + pos->pceNum[wP]));

	// check bitboards squares
	while(t_pawns[WHITE]) {
		sq64 = POP(&t_pawns[WHITE]);
		ASSERT(pos->pieces[SQ120(sq64)] == wP);
	}

	while(t_pawns[BLACK]) {
		sq64 = POP(&t_pawns[BLACK]);
		ASSERT(pos->pieces[SQ120(sq64)] == bP);
	}

	while(t_pawns[BOTH]) {
		sq64 = POP(&t_pawns[BOTH]);
		ASSERT( (pos->pieces[SQ120(sq64)] == bP) || (pos->pieces[SQ120(sq64)] == wP) );
	}

	ASSERT(t_material[WHITE]==pos->material[WHITE] && t_material[BLACK]==pos->material[BLACK]);
	ASSERT(t_minPce[WHITE]==pos->minPiece[WHITE] && t_minPce[BLACK]==pos->minPiece[BLACK]);
	ASSERT(t_majPce[WHITE]==pos->majPiece[WHITE] && t_majPce[BLACK]==pos->majPiece[BLACK]);
	ASSERT(t_bigPce[WHITE]==pos->bigPiece[WHITE] && t_bigPce[BLACK]==pos->bigPiece[BLACK]);

	ASSERT(pos->side==WHITE || pos->side==BLACK);
	ASSERT(GeneratePosKey(pos)==pos->posKey);

	ASSERT(pos->enPas==NO_SQ || ( RanksBrd[pos->enPas]==RANK_6 && pos->side == WHITE)
		 || ( RanksBrd[pos->enPas]==RANK_3 && pos->side == BLACK));

	ASSERT(pos->pieces[pos->KingSq[WHITE]] == wK);
	ASSERT(pos->pieces[pos->KingSq[BLACK]] == bK);

	return TRUE;
}

void UpdateListsMaterial (S_BOARD *pos){
	int piece, sq, index, colour;

	for (index = 0; index < 64; ++index){
		sq = index;
		piece = pos->pieces[index];
		if (piece!=OFFBOARD && piece!=EMPTY){
			colour = PieceCol[piece];
			if (PieceBig[piece] == TRUE){
				pos->bigPiece[colour]++;
			}
			if (PieceMaj[piece] == TRUE){
				pos->majPiece[colour]++;
			}
			if (PieceMin[piece] == TRUE){
				pos->minPiece[colour]++;
			}

			pos->material[colour] += PieceVal[piece];

			pos->pList[piece][pos->pceNum[piece]] = sq;
			pos->pceNum[piece]++;


			if (piece==wK){
				pos->KingSq[WHITE] = sq;
			}
			if (piece==bK){
				pos->KingSq[BLACK] = sq;
			}

			if (piece == wP){
				SETBIT(pos->pawns[WHITE], SQ64(sq));
				SETBIT(pos->pawns[BOTH], SQ64(sq));
			}else if (piece == bP){
				SETBIT(pos->pawns[BLACK], SQ64(sq));
				SETBIT(pos->pawns[BOTH], SQ64(sq));
			}
		}
	}
}


void ResetBoard(S_BOARD *pos) {

	int index = 0;

	for(index = 0; index < 64; ++index) {
		pos->pieces[index] = OFFBOARD;
	}

	for(index = 0; index < 64; ++index) {
		pos->pieces[SQ120(index)] = EMPTY;
	}

	for(index = 0; index < 2; ++index) {
		pos->bigPiece[index] = 0;
		pos->majPiece[index] = 0;
		pos->minPiece[index] = 0;
		pos->material[index] = 0;
	}

	for(index = 0; index < 3; ++index) {
		pos->pawns[index] = 0ULL;
	}

	for(index = 0; index < 13; ++index) {
		pos->pceNum[index] = 0;
	}

	pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;

	pos->side = BOTH;
	pos->enPas = NO_SQ;
	pos->fiftyMove = 0;

	pos->ply = 0;
	pos->hisPly = 0;

	pos->castlePerm = 0;

	pos->posKey = 0ULL;
}

void PrintBoard(const S_BOARD *pos) {
	int sq, file, rank, piece;

	printf("\nGame Board:\n\n");

	for(rank = RANK_8; rank >= RANK_1; rank--) {
		printf("%d  ", rank + 1);
		for(file = FILE_A; file <= FILE_H; file++) {
			sq = FR2SQ(file, rank);
			piece = pos->pieces[sq];
			printf("%3c", PceChar[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	for(file = FILE_A; file <= FILE_H; file++)
		printf("%3c", 'a' + file);

	printf("\n");
	printf("side:%c\n", SideChar[pos->side]);
	printf("enPas:%d\n", pos->enPas);
	printf("castle:%c%c%c%c\n",
		pos->castlePerm & WKCA ? 'K' : '-',
		pos->castlePerm & WQCA ? 'Q' : '-',
		pos->castlePerm & BKCA ? 'k' : '-',
		pos->castlePerm & BQCA ? 'q' : '-');

	printf("PosKey:%0llx\n", pos->posKey);
}

int ParseFen(char *fen, S_BOARD *pos) {

	ASSERT(fen!=NULL);
	ASSERT(pos!=NULL);

	int  rank = RANK_8;
    int  file = FILE_A;
    int  piece = 0;
    int  count = 0;
    int  i = 0;
	int  sq64 = 0;
	int  sq120 = 0;

	ResetBoard(pos);

	while ((rank >= RANK_1) && *fen) {
	    count = 1;
		switch (*fen) {
            case 'p': piece = bP; break;
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;
            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                printf("FEN error \n");
                return -1;
        }

		for (i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
			sq120 = SQ120(sq64);
            if (piece != EMPTY) {
                pos->pieces[sq120] = piece;
            }
			file++;
        }
		fen++;
	}

	ASSERT(*fen == 'w' || *fen == 'b');

	pos->side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	for (i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
		switch(*fen) {
			case 'K': pos->castlePerm |= WKCA; break;
			case 'Q': pos->castlePerm |= WQCA; break;
			case 'k': pos->castlePerm |= BKCA; break;
			case 'q': pos->castlePerm |= BQCA; break;
			default:	     break;
        }
		fen++;
	}
	fen++;

	ASSERT(pos->castlePerm>=0 && pos->castlePerm <= 15);

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file>=FILE_A && file <= FILE_H);
		ASSERT(rank>=RANK_1 && rank <= RANK_8);

		pos->enPas = FR2SQ(file,rank);
    }

	pos->posKey = GeneratePosKey(pos);

	UpdateListsMaterial(pos);

	return 0;
}

void MirrorBoard(S_BOARD *pos) {

    int tempPiecesArray[64];
    int tempSide = pos->side^1;
	int SwapPiece[13] = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK };
    int tempCastlePerm = 0;
    int tempEnPas = NO_SQ;

	int sq;
	int tp;

    if (pos->castlePerm & WKCA) tempCastlePerm |= BKCA;
    if (pos->castlePerm & WQCA) tempCastlePerm |= BQCA;

    if (pos->castlePerm & BKCA) tempCastlePerm |= WKCA;
    if (pos->castlePerm & BQCA) tempCastlePerm |= WQCA;

	if (pos->enPas != NO_SQ)  {
        tempEnPas = SQ120(Mirror64[SQ64(pos->enPas)]);
    }

    for (sq = 0; sq < 64; sq++) {
        tempPiecesArray[sq] = pos->pieces[SQ120(Mirror64[sq])];
    }

    ResetBoard(pos);

	for (sq = 0; sq < 64; sq++) {
        tp = SwapPiece[tempPiecesArray[sq]];
        pos->pieces[SQ120(sq)] = tp;
    }

	pos->side = tempSide;
    pos->castlePerm = tempCastlePerm;
    pos->enPas = tempEnPas;

    pos->posKey = GeneratePosKey(pos);

	UpdateListsMaterial(pos);

    ASSERT(CheckBoard(pos));
}

// ASCII pieces
char ascii_pieces[13] = ".PNBRQKpnbrqk";

// unicode pieces
char *unicode_pieces[13] = {".", "♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

void print_board(const S_BOARD *pos)
{
    // print offset
    printf("\n");

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop ober board files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // print ranks
            if (!file)
                printf("  %d ", 8 - rank);
            
            // define piece variable
            int piece = -1;
            
            // loop over all piece bitboards
            for (int bb_piece = wP; bb_piece <= bK; bb_piece++)
            {
                // if there is a piece on current square
                if (get_bit(pos->bitboards[bb_piece], square)){
                    // get piece code
                    piece = bb_piece;
				}
            }
            
            printf(" %s", (piece == -1) ? "." : unicode_pieces[piece]);
        }
        
        // print new line every rank
        printf("\n");
    }
    
    // print board files
    printf("\n     a b c d e f g h\n\n");
    
    // print side to move
    printf("     Side:     %s\n", !pos->side ? "white" : "black");
    
    // print enpassant square
    printf("     Enpassant:   %s\n", (pos->enPas != no_sq) ? square_to_coordinates[pos->enPas] : "no");
    
    // print castling rights
    printf("     Castling:  %c%c%c%c\n\n", (pos->castlePerm & WKCA) ? 'K' : '-',
                                           (pos->castlePerm & WQCA) ? 'Q' : '-',
                                           (pos->castlePerm & BKCA) ? 'k' : '-',
                                           (pos->castlePerm & BQCA) ? 'q' : '-');
    
    // print hash key
    printf("     Hash key:  %llx\n\n", pos->posKey);
}

void reset_board(S_BOARD *pos) {

	int index = 0;
	
    for (int i = 0; i < 64; i++){
        pos->pieces[i] = EMPTY;
    }

	for (int piece = wP; piece <= bK; piece++)
        pos->bitboards[piece] = 0ULL;

    for (int i = 0; i < 3;i++){
        pos->occupancies[i] = 0ULL;
    }

	pos->side = BOTH;
	pos->enPas = no_sq;
	pos->fiftyMove = 0;

	pos->ply = 0;
	pos->hisPly = 0;

	pos->castlePerm = 0;

	pos->posKey = 0ULL;
}

int char_pieces[] = {
    ['P'] = wP,
    ['N'] = wN,
    ['B'] = wB,
    ['R'] = wR,
    ['Q'] = wQ,
    ['K'] = wK,

    ['p'] = bP,
    ['n'] = bN,
    ['b'] = bB,
    ['r'] = bR,
    ['q'] = bQ,
    ['k'] = bK
};

// parse FEN string
void parse_fen(char *fen, S_BOARD *pos)
{
    reset_board(pos);

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // init current square
            int square = rank * 8 + file;
            
            // match ascii pieces within FEN string
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                // init piece type
                int piece = char_pieces[*fen];
                
                // set piece on corresponding bitboard
                set_bit(pos->bitboards[piece], square);
                pos->pieces[square] = piece;
                
                // increment pointer to FEN string
                fen++;
            }
            
            // match empty square numbers within FEN string
            if (*fen >= '0' && *fen <= '9')
            {
                // init offset (convert char 0 to int 0)
                int offset = *fen - '0';
                
                // define piece variable
                int piece = -1;
                
                // loop over all piece bitboards
                for (int bb_piece = wP; bb_piece <= bK; bb_piece++)
                {
                    // if there is a piece on current square
                    if (get_bit(pos->bitboards[bb_piece], square))
                        piece = bb_piece;
                }
                
                // on empty current square
                if (piece == -1)
                    // decrement file
                    file--;
                
                // adjust file counter
                file += offset;
                
                // increment pointer to FEN string
                fen++;
            }
            
            // match rank separator
            if (*fen == '/')
                // increment pointer to FEN string
                fen++;
        }
    }
    
    // got to parsing side to move (increment pointer to FEN string)
    fen++;
    
    // parse side to move
    (*fen == 'w') ? (pos->side = WHITE) : (pos->side = BLACK);
    
    // go to parsing castling rights
    fen += 2;
    
    // parse castling rights
    while (*fen != ' ')
    {
        switch (*fen)
        {
            case 'K': pos->castlePerm |= WKCA; break;
            case 'Q': pos->castlePerm |= WQCA; break;
            case 'k': pos->castlePerm |= BKCA; break;
            case 'q': pos->castlePerm |= BQCA; break;
            case '-': break;
        }

        // increment pointer to FEN string
        fen++;
    }
    
    // got to parsing enpassant square (increment pointer to FEN string)
    fen++;
    
    // parse enpassant square
    if (*fen != '-')
    {
        // parse enpassant file & rank
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        
        // init enpassant square
        pos->enPas = rank * 8 + file;
    }
    // no enpassant square
    else{
        pos->enPas = no_sq;
	}

    for (int piece = wP; piece <= wK; piece++)
        pos->occupancies[WHITE] |= pos->bitboards[piece];
    
    for (int piece = bP; piece <= bK; piece++)
        pos->occupancies[BLACK] |= pos->bitboards[piece];
    
    // init all occupancies
    pos->occupancies[BOTH] |= pos->occupancies[WHITE];
    pos->occupancies[BOTH] |= pos->occupancies[BLACK];
    
    // init hash key
    pos->posKey = GeneratePosKey(pos);
}