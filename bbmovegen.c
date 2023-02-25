#include "defs.h"

const int VictimScore[13] = {0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};
static int MvvLvaScores[13][13];

void InitMvvlva()
{
    int attacker;
    int victim;

    for (attacker = wP; attacker <= bK; ++attacker)
    {
        for (victim = wP; victim <= bK; ++victim)
        {
            MvvLvaScores[victim][attacker] = VictimScore[victim] + 6 - (VictimScore[attacker] / 100);
        }
    }
}

int MoveExists(S_BOARD *pos, const int move)
{
    S_MOVELIST list[1];
    generate_moves(list, pos);

    int moveNum = 0;
    for (moveNum = 0; moveNum < list->count; ++moveNum)
    {
        if (!make_move(pos, list->moves[moveNum].move))
        {
            take_move(pos);
            continue;
        }
        take_move(pos);
        if (list->moves[moveNum].move == move)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void add_move(S_MOVELIST *list, int move, S_BOARD *pos)
{
    // store move
    list->moves[list->count].move = move;

    if (get_move_capture(move))
    {
        MvvLvaScores[pos->pieces[get_move_target(move)]][pos->pieces[get_move_source(move)]] + 1000000;
    }
    else
    {
        if (pos->searchKillers[0][pos->ply] == move)
        {
            list->moves[list->count].score = 900000;
        }
        else if (pos->searchKillers[1][pos->ply] == move)
        {
            list->moves[list->count].score = 800000;
        }
        else
        {
            list->moves[list->count].move = move;
            list->moves[list->count].score = pos->searchHistory[pos->pieces[get_move_source(move)]][get_move_target(move)];
        }
        
    }

    // increment move count
    list->count++;
}

void add_capture(S_MOVELIST *list, int move, S_BOARD *pos)
{

    if (!get_move_capture(move))
    {
        return;
    }
    
    add_move(list, move, pos);
}

void add_promotion_moves(S_MOVELIST *move_list, int source_square, int target_square, int piece, int capture, S_BOARD *pos)
{
    int side = PieceToPieceColor[piece] == WHITE;
    add_move(move_list, encode_move(source_square, target_square, piece, (side ? wQ : bQ), capture, 0, 0, 0), pos);
    add_move(move_list, encode_move(source_square, target_square, piece, (side ? wR : bR), capture, 0, 0, 0), pos);
    add_move(move_list, encode_move(source_square, target_square, piece, (side ? wB : bB), capture, 0, 0, 0), pos);
    add_move(move_list, encode_move(source_square, target_square, piece, (side ? wN : bN), capture, 0, 0, 0), pos);
}

void generate_moves(S_MOVELIST *move_list, S_BOARD *pos)
{
    // init move count
    move_list->count = 0;

    // define source & target squares
    int source_square, target_square;

    // define current piece's bitboard copy & it's attacks
    U64 bitboard, attacks;

    // loop over all the bitboards
    for (int piece = wP; piece <= bK; ++piece)
    {
        // init piece bitboard copy
        bitboard = pos->bitboards[piece];

        if (pos->side == WHITE)
        {
            // pick up white pawn bitboards index
            if (piece == wP)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);


                    // init target square
                    target_square = source_square - 8;

                    // generate quiet pawn moves
                    if (!(target_square < a8) && !get_bit(pos->occupancies[BOTH], target_square))
                    {
                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7)
                        {
                            add_promotion_moves(move_list, source_square, target_square, piece, 0, pos);

                        }

                        else
                        {
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0), pos);

                            // two squares ahead pawn move
                            if ((source_square >= a2 && source_square <= h2) && !get_bit(pos->occupancies[BOTH], target_square - 8)){
                                add_move(move_list, encode_move(source_square, (target_square - 8), piece, 0, 0, 1, 0, 0), pos);
                            }
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawn_attacks[pos->side][source_square] & pos->occupancies[BLACK];

                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7)
                        {
                            add_promotion_moves(move_list, source_square, target_square, piece, 1, pos);
                        }
                        else{
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                        }

                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (pos->enPas != no_sq)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawn_attacks[pos->side][source_square] & (1ULL << pos->enPas);

                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0), pos);
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }
            // castling moves
            if (piece == wK)
            {
                // king side castling is available
                if (pos->castlePerm & WKCA)
                {
                    // make sure square between king and king's rook are empty
                    if (!get_bit(pos->occupancies[BOTH], f1) && !get_bit(pos->occupancies[BOTH], g1))
                    {
                        // make sure king and the f1 squares are not under attacks
                        if (!is_square_attacked(e1, BLACK, pos) && !is_square_attacked(f1, BLACK, pos))
                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1), pos);
                    }
                }

                // queen side castling is available
                if (pos->castlePerm & WQCA)
                {
                    // make sure square between king and queen's rook are empty
                    if (!get_bit(pos->occupancies[BOTH], d1) && !get_bit(pos->occupancies[BOTH], c1) && !get_bit(pos->occupancies[BOTH], b1))
                    {
                        // make sure king and the d1 squares are not under attacks
                        if (!is_square_attacked(e1, BLACK, pos) && !is_square_attacked(d1, BLACK, pos))
                            add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1), pos);
                    }
                }
            }
        }
        else
        {
            if (piece == bP)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init target square
                    target_square = source_square + 8;

                    // generate quiet pawn moves
                    if (!(target_square > h1) && !get_bit(pos->occupancies[BOTH], target_square))
                    {
                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_promotion_moves(move_list, source_square, target_square, piece, 0, pos);
                        }

                        else
                        {
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0), pos);

                            // two squares ahead pawn move
                            if ((source_square >= a7 && source_square <= h7) && !get_bit(pos->occupancies[BOTH], target_square + 8)){
                                add_move(move_list, encode_move(source_square, (target_square + 8), piece, 0, 0, 1, 0, 0), pos);
                            }
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawn_attacks[pos->side][source_square] & pos->occupancies[WHITE];

                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_promotion_moves(move_list, source_square, target_square, piece, 1, pos);
                        }

                        else{
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                        }
                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (pos->enPas != no_sq)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawn_attacks[pos->side][source_square] & (1ULL << pos->enPas);

                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0), pos);
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }
            // castling moves
            if (piece == bK)
            {
                // king side castling is available
                if (pos->castlePerm & BKCA)
                {
                    // make sure square between king and king's rook are empty
                    if (!get_bit(pos->occupancies[BOTH], f8) && !get_bit(pos->occupancies[BOTH], g8))
                    {
                        // make sure king and the f8 squares are not under attacks
                        if (!is_square_attacked(e8, WHITE, pos) && !is_square_attacked(f8, WHITE, pos))
                            add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1), pos);
                    }
                }

                // queen side castling is available
                if (pos->castlePerm & BQCA)
                {
                    // make sure square between king and queen's rook are empty
                    if (!get_bit(pos->occupancies[BOTH], d8) && !get_bit(pos->occupancies[BOTH], c8) && !get_bit(pos->occupancies[BOTH], b8))
                    {
                        // make sure king and the d8 squares are not under attacks
                        if (!is_square_attacked(e8, WHITE, pos) && !is_square_attacked(d8, WHITE, pos))
                            add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1), pos);
                    }
                }
            }
        }
        // genarate knight moves
        if ((pos->side == WHITE) ? piece == wN : piece == bN)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = knight_attacks[source_square] & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quiet move
                    if (!get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0), pos);

                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        // generate bishop moves
        if ((pos->side == WHITE) ? piece == wB : piece == bB)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_bishop_attacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quiet move
                    if (!get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0), pos);

                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        // generate rook moves
        if ((pos->side == WHITE) ? piece == wR : piece == bR)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_rook_attacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quiet move
                    if (!get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0), pos);

                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        // generate queen moves
        if ((pos->side == WHITE) ? piece == wQ : piece == bQ)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_queen_attacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quiet move
                    if (!get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0), pos);

                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        // generate king moves
        if ((pos->side == WHITE) ? piece == wK : piece == bK)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = king_attacks[source_square] & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);
                    if (!is_square_attacked(target_square, ((pos->side == WHITE) ? BLACK : WHITE), pos))
                    {
                        // quiet move
                        if (!get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square))
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0), pos);
                        else
                        {
                            // capture move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                        }
                    }
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
    }
}

void generate_capture_moves(S_MOVELIST *move_list, S_BOARD *pos)
{
    // init move count
    move_list->count = 0;

    // define source & target squares
    int source_square, target_square;

    // define current piece's bitboard copy & it's attacks
    U64 bitboard, attacks;

    // loop over all the bitboards
    for (int piece = wP; piece <= bK; ++piece)
    {
        // init piece bitboard copy
        bitboard = pos->bitboards[piece];

        if (pos->side == WHITE)
        {
            // pick up white pawn bitboards index
            if (piece == wP)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init pawn attacks bitboard
                    attacks = pawn_attacks[pos->side][source_square] & pos->occupancies[BLACK];

                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7)
                        {
                            add_promotion_moves(move_list, source_square, target_square, piece, 1, pos);
                        }
                        else{
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                        }

                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (pos->enPas != no_sq)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawn_attacks[pos->side][source_square] & (1ULL << pos->enPas);

                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0), pos);
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }
        }
        else
        {
            if (piece == bP)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);

                    // init pawn attacks bitboard
                    attacks = pawn_attacks[pos->side][source_square] & pos->occupancies[WHITE];

                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);

                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_promotion_moves(move_list, source_square, target_square, piece, 1, pos);
                        }

                        else{
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                        }
                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (pos->enPas != no_sq)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawn_attacks[pos->side][source_square] & (1ULL << pos->enPas);

                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0), pos);
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }
        }
        // genarate knight moves
        if ((pos->side == WHITE) ? piece == wN : piece == bN)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = knight_attacks[source_square] & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    if (get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square))
                    {
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                    }
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        // generate bishop moves
        if ((pos->side == WHITE) ? piece == wB : piece == bB)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_bishop_attacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quiet move
                    if (get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square)){
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                    }
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        // generate rook moves
        if ((pos->side == WHITE) ? piece == wR : piece == bR)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_rook_attacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    // quiet move
                    if (get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                    }

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        // generate queen moves
        if ((pos->side == WHITE) ? piece == wQ : piece == bQ)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = get_queen_attacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);

                    if (get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square)){
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                    }

                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        // generate king moves
        if ((pos->side == WHITE) ? piece == wK : piece == bK)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = king_attacks[source_square] & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);

                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);
                    if (!is_square_attacked(target_square, ((pos->side == WHITE) ? BLACK : WHITE), pos))
                    {
                        // quiet move
                        if (get_bit(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square)){
                            // capture move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0), pos);
                        }
                    }
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
    }
}

