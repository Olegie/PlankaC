#include "plankac_internal.h"

static PLC_FRAME *plc_chess_frame(PLC_FRAME *frame)
{
    if (frame != 0 && frame->heap_owner != 0) {
        return frame->heap_owner;
    }
    return frame;
}

static int plc_chess_kind(int piece)
{
    return piece / 10;
}

static int plc_chess_side(int piece)
{
    return piece % 10;
}

static int plc_chess_file(int square)
{
    return square / 10;
}

static int plc_chess_rank(int square)
{
    return square % 10;
}

static int plc_chess_square(int file, int rank)
{
    return file * 10 + rank;
}

static int plc_chess_valid_square(int square)
{
    int file;
    int rank;

    file = plc_chess_file(square);
    rank = plc_chess_rank(square);
    return file >= 1 && file <= 8 && rank >= 1 && rank <= 8;
}

static int plc_chess_value(int piece)
{
    int kind;

    kind = plc_chess_kind(piece);
    if (kind == 1) {
        return 1;
    }
    if (kind == 2 || kind == 3) {
        return 3;
    }
    if (kind == 4) {
        return 5;
    }
    if (kind == 5) {
        return 9;
    }
    return 0;
}

static int plc_chess_record_slot(PLC_FRAME *frame, int board_id, int square)
{
    int i;

    for (i = 0; i < frame->record_sizes[board_id]; ++i) {
        if (frame->record_keys[board_id][i] == square) {
            return i;
        }
    }
    return -1;
}

static int plc_chess_record_set(PLC_FRAME *frame, int board_id, int square,
    int piece, char *err, unsigned err_size)
{
    int slot;

    if (board_id < 0 || board_id >= frame->record_count) {
        plc_set_error(err, err_size, "bad chess board id");
        return 0;
    }
    slot = plc_chess_record_slot(frame, board_id, square);
    if (slot < 0) {
        if (frame->record_sizes[board_id] >= PLC_MAX_RECORD_FIELDS_PER_RECORD) {
            plc_set_error(err, err_size, "chess board record is full");
            return 0;
        }
        slot = frame->record_sizes[board_id];
        frame->record_keys[board_id][slot] = square;
        ++frame->record_sizes[board_id];
    }
    frame->record_values[board_id][slot] = (double)piece;
    return 1;
}

static int plc_chess_load_board(PLC_FRAME *frame, int board_id,
    int board[9][9], char *err, unsigned err_size)
{
    int file;
    int rank;
    int i;

    frame = plc_chess_frame(frame);
    if (frame == 0 || board_id < 0 || board_id >= frame->record_count) {
        plc_set_error(err, err_size, "bad chess board id");
        return 0;
    }
    for (file = 0; file < 9; ++file) {
        for (rank = 0; rank < 9; ++rank) {
            board[file][rank] = 0;
        }
    }
    for (i = 0; i < frame->record_sizes[board_id]; ++i) {
        int square;

        square = frame->record_keys[board_id][i];
        if (plc_chess_valid_square(square)) {
            board[plc_chess_file(square)][plc_chess_rank(square)] =
                (int)frame->record_values[board_id][i];
        }
    }
    return 1;
}

static int plc_chess_path_clear(int board[9][9], int from_square,
    int to_square)
{
    int from_file;
    int from_rank;
    int to_file;
    int to_rank;
    int step_file;
    int step_rank;
    int file;
    int rank;

    from_file = plc_chess_file(from_square);
    from_rank = plc_chess_rank(from_square);
    to_file = plc_chess_file(to_square);
    to_rank = plc_chess_rank(to_square);
    step_file = to_file > from_file ? 1 : (to_file < from_file ? -1 : 0);
    step_rank = to_rank > from_rank ? 1 : (to_rank < from_rank ? -1 : 0);
    file = from_file + step_file;
    rank = from_rank + step_rank;
    while (file != to_file || rank != to_rank) {
        if (board[file][rank] != 0) {
            return 0;
        }
        file += step_file;
        rank += step_rank;
    }
    return 1;
}

static int plc_chess_piece_pattern(int board[9][9], int piece,
    int from_square, int to_square, int attack_only)
{
    int from_file;
    int from_rank;
    int to_file;
    int to_rank;
    int df;
    int dr;
    int kind;
    int side;
    int target;

    if (!plc_chess_valid_square(from_square)
            || !plc_chess_valid_square(to_square)
            || from_square == to_square || piece == 0) {
        return 0;
    }
    from_file = plc_chess_file(from_square);
    from_rank = plc_chess_rank(from_square);
    to_file = plc_chess_file(to_square);
    to_rank = plc_chess_rank(to_square);
    df = to_file - from_file;
    dr = to_rank - from_rank;
    kind = plc_chess_kind(piece);
    side = plc_chess_side(piece);
    target = board[to_file][to_rank];
    if (!attack_only && target != 0 && plc_chess_side(target) == side) {
        return 0;
    }
    if (kind == 1) {
        int forward;

        forward = side == 1 ? 1 : -1;
        if (attack_only) {
            return dr == forward && (df == 1 || df == -1);
        }
        if (df == 0 && dr == forward && target == 0) {
            return 1;
        }
        if (df == 0 && dr == 2 * forward && target == 0
                && ((side == 1 && from_rank == 2)
                    || (side == 2 && from_rank == 7))
                && board[from_file][from_rank + forward] == 0) {
            return 1;
        }
        return dr == forward && (df == 1 || df == -1)
            && target != 0 && plc_chess_side(target) != side;
    }
    if (kind == 2) {
        return (abs(df) == 1 && abs(dr) == 2)
            || (abs(df) == 2 && abs(dr) == 1);
    }
    if (kind == 6) {
        return abs(df) <= 1 && abs(dr) <= 1;
    }
    if (kind == 4 && (df == 0 || dr == 0)) {
        return plc_chess_path_clear(board, from_square, to_square);
    }
    if (kind == 3 && abs(df) == abs(dr)) {
        return plc_chess_path_clear(board, from_square, to_square);
    }
    if (kind == 5 && (df == 0 || dr == 0 || abs(df) == abs(dr))) {
        return plc_chess_path_clear(board, from_square, to_square);
    }
    return 0;
}

static int plc_chess_side_in_check_board(int board[9][9], int side)
{
    int file;
    int rank;
    int king_square;
    int opposite;

    king_square = 0;
    for (file = 1; file <= 8; ++file) {
        for (rank = 1; rank <= 8; ++rank) {
            int piece;

            piece = board[file][rank];
            if (piece != 0 && plc_chess_kind(piece) == 6
                    && plc_chess_side(piece) == side) {
                king_square = plc_chess_square(file, rank);
            }
        }
    }
    if (king_square == 0) {
        return 1;
    }
    opposite = side == 1 ? 2 : 1;
    for (file = 1; file <= 8; ++file) {
        for (rank = 1; rank <= 8; ++rank) {
            int piece;

            piece = board[file][rank];
            if (piece != 0 && plc_chess_side(piece) == opposite
                    && plc_chess_piece_pattern(board, piece,
                        plc_chess_square(file, rank), king_square, 1)) {
                return 1;
            }
        }
    }
    return 0;
}

static int plc_chess_legal_board_move(int board[9][9], int from_square,
    int to_square, int enforce_king_safety)
{
    int from_file;
    int from_rank;
    int to_file;
    int to_rank;
    int moving_piece;
    int target_piece;
    int side;

    if (!plc_chess_valid_square(from_square)
            || !plc_chess_valid_square(to_square)) {
        return 0;
    }
    from_file = plc_chess_file(from_square);
    from_rank = plc_chess_rank(from_square);
    to_file = plc_chess_file(to_square);
    to_rank = plc_chess_rank(to_square);
    moving_piece = board[from_file][from_rank];
    target_piece = board[to_file][to_rank];
    if (!plc_chess_piece_pattern(board, moving_piece, from_square,
            to_square, 0)) {
        return 0;
    }
    if (target_piece != 0
            && plc_chess_side(target_piece) == plc_chess_side(moving_piece)) {
        return 0;
    }
    if (enforce_king_safety) {
        side = plc_chess_side(moving_piece);
        board[to_file][to_rank] = moving_piece;
        board[from_file][from_rank] = 0;
        if (plc_chess_side_in_check_board(board, side)) {
            board[from_file][from_rank] = moving_piece;
            board[to_file][to_rank] = target_piece;
            return 0;
        }
        board[from_file][from_rank] = moving_piece;
        board[to_file][to_rank] = target_piece;
    }
    return 1;
}

int plc_chess_model_legal_move(PLC_FRAME *frame, int board_id,
    int from_square, int to_square, int enforce_king_safety,
    int *legal, char *err, unsigned err_size)
{
    int board[9][9];

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    if (legal != 0) {
        *legal = plc_chess_legal_board_move(board, from_square,
            to_square, enforce_king_safety);
    }
    return 1;
}

int plc_chess_model_apply_move(PLC_FRAME *frame, int board_id,
    int from_square, int to_square, int enforce_king_safety,
    char *err, unsigned err_size)
{
    int board[9][9];
    int moving_piece;
    int legal;

    frame = plc_chess_frame(frame);
    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    legal = plc_chess_legal_board_move(board, from_square,
        to_square, enforce_king_safety);
    if (!legal) {
        plc_set_error(err, err_size, "illegal chess move");
        return 0;
    }
    moving_piece = board[plc_chess_file(from_square)]
        [plc_chess_rank(from_square)];
    return plc_chess_record_set(frame, board_id, from_square, 0,
            err, err_size)
        && plc_chess_record_set(frame, board_id, to_square, moving_piece,
            err, err_size);
}

int plc_chess_model_side_in_check(PLC_FRAME *frame, int board_id, int side,
    int *in_check, char *err, unsigned err_size)
{
    int board[9][9];

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    if (in_check != 0) {
        *in_check = plc_chess_side_in_check_board(board, side);
    }
    return 1;
}

int plc_chess_model_checkmate(PLC_FRAME *frame, int board_id, int side,
    int *is_mate, char *err, unsigned err_size)
{
    int board[9][9];
    int file;
    int rank;
    int to_file;
    int to_rank;

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    if (!plc_chess_side_in_check_board(board, side)) {
        if (is_mate != 0) {
            *is_mate = 0;
        }
        return 1;
    }
    for (file = 1; file <= 8; ++file) {
        for (rank = 1; rank <= 8; ++rank) {
            int piece;

            piece = board[file][rank];
            if (piece == 0 || plc_chess_side(piece) != side) {
                continue;
            }
            for (to_file = 1; to_file <= 8; ++to_file) {
                for (to_rank = 1; to_rank <= 8; ++to_rank) {
                    if (plc_chess_legal_board_move(board,
                            plc_chess_square(file, rank),
                            plc_chess_square(to_file, to_rank), 1)) {
                        if (is_mate != 0) {
                            *is_mate = 0;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    if (is_mate != 0) {
        *is_mate = 1;
    }
    return 1;
}

int plc_chess_model_material_score(PLC_FRAME *frame, int board_id, int side,
    int *score, char *err, unsigned err_size)
{
    int board[9][9];
    int file;
    int rank;
    int total;

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    total = 0;
    for (file = 1; file <= 8; ++file) {
        for (rank = 1; rank <= 8; ++rank) {
            int piece;

            piece = board[file][rank];
            if (piece != 0 && plc_chess_side(piece) == side) {
                total += plc_chess_value(piece);
            }
        }
    }
    if (score != 0) {
        *score = total;
    }
    return 1;
}

int plc_chess_model_best_capture_score(PLC_FRAME *frame, int board_id,
    int side, int *score, char *err, unsigned err_size)
{
    int board[9][9];
    int file;
    int rank;
    int to_file;
    int to_rank;
    int best;

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    best = 0;
    for (file = 1; file <= 8; ++file) {
        for (rank = 1; rank <= 8; ++rank) {
            int piece;

            piece = board[file][rank];
            if (piece == 0 || plc_chess_side(piece) != side) {
                continue;
            }
            for (to_file = 1; to_file <= 8; ++to_file) {
                for (to_rank = 1; to_rank <= 8; ++to_rank) {
                    int target;

                    target = board[to_file][to_rank];
                    if (target != 0 && plc_chess_side(target) != side
                            && plc_chess_legal_board_move(board,
                                plc_chess_square(file, rank),
                                plc_chess_square(to_file, to_rank), 1)
                            && plc_chess_value(target) > best) {
                        best = plc_chess_value(target);
                    }
                }
            }
        }
    }
    if (score != 0) {
        *score = best;
    }
    return 1;
}

int plc_chess_model_legal_move_count(PLC_FRAME *frame, int board_id,
    int side, int *count, char *err, unsigned err_size)
{
    int board[9][9];
    int file;
    int rank;
    int to_file;
    int to_rank;
    int total;

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    total = 0;
    for (file = 1; file <= 8; ++file) {
        for (rank = 1; rank <= 8; ++rank) {
            int piece;

            piece = board[file][rank];
            if (piece == 0 || plc_chess_side(piece) != side) {
                continue;
            }
            for (to_file = 1; to_file <= 8; ++to_file) {
                for (to_rank = 1; to_rank <= 8; ++to_rank) {
                    if (plc_chess_legal_board_move(board,
                            plc_chess_square(file, rank),
                            plc_chess_square(to_file, to_rank), 1)) {
                        ++total;
                    }
                }
            }
        }
    }
    if (count != 0) {
        *count = total;
    }
    return 1;
}

int plc_chess_model_stalemate(PLC_FRAME *frame, int board_id, int side,
    int *is_stalemate, char *err, unsigned err_size)
{
    int board[9][9];
    int file;
    int rank;
    int to_file;
    int to_rank;

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    if (is_stalemate != 0) {
        *is_stalemate = 0;
    }
    if (plc_chess_side_in_check_board(board, side)) {
        return 1;
    }
    for (file = 1; file <= 8; ++file) {
        for (rank = 1; rank <= 8; ++rank) {
            int piece;

            piece = board[file][rank];
            if (piece == 0 || plc_chess_side(piece) != side) {
                continue;
            }
            for (to_file = 1; to_file <= 8; ++to_file) {
                for (to_rank = 1; to_rank <= 8; ++to_rank) {
                    if (plc_chess_legal_board_move(board,
                            plc_chess_square(file, rank),
                            plc_chess_square(to_file, to_rank), 1)) {
                        return 1;
                    }
                }
            }
        }
    }
    if (is_stalemate != 0) {
        *is_stalemate = 1;
    }
    return 1;
}

int plc_chess_model_pawn_promotion(int from_square, int to_square,
    int side)
{
    int from_rank;
    int to_rank;
    int df;

    if (!plc_chess_valid_square(from_square)
            || !plc_chess_valid_square(to_square)) {
        return 0;
    }
    from_rank = plc_chess_rank(from_square);
    to_rank = plc_chess_rank(to_square);
    df = plc_chess_file(to_square) - plc_chess_file(from_square);
    if (df < -1 || df > 1) {
        return 0;
    }
    if (side == 1) {
        return from_rank == 7 && to_rank == 8;
    }
    if (side == 2) {
        return from_rank == 2 && to_rank == 1;
    }
    return 0;
}

int plc_chess_model_can_castle_path(PLC_FRAME *frame, int board_id,
    int side, int king_from, int rook_from, int *can_castle,
    char *err, unsigned err_size)
{
    int board[9][9];
    int king_file;
    int king_rank;
    int rook_file;
    int rook_rank;
    int direction;
    int file;
    int target_file;
    int king_piece;
    int rook_piece;
    int old_king_square;

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    if (can_castle != 0) {
        *can_castle = 0;
    }
    if (!plc_chess_valid_square(king_from)
            || !plc_chess_valid_square(rook_from)) {
        return 1;
    }
    king_file = plc_chess_file(king_from);
    king_rank = plc_chess_rank(king_from);
    rook_file = plc_chess_file(rook_from);
    rook_rank = plc_chess_rank(rook_from);
    if (king_rank != rook_rank || king_file != 5
            || (side == 1 && king_rank != 1)
            || (side == 2 && king_rank != 8)) {
        return 1;
    }
    king_piece = board[king_file][king_rank];
    rook_piece = board[rook_file][rook_rank];
    if (plc_chess_kind(king_piece) != 6 || plc_chess_side(king_piece) != side
            || plc_chess_kind(rook_piece) != 4
            || plc_chess_side(rook_piece) != side) {
        return 1;
    }
    direction = rook_file > king_file ? 1 : -1;
    file = king_file + direction;
    while (file != rook_file) {
        if (board[file][king_rank] != 0) {
            return 1;
        }
        file += direction;
    }
    if (plc_chess_side_in_check_board(board, side)) {
        return 1;
    }
    target_file = direction > 0 ? 7 : 3;
    old_king_square = king_from;
    file = king_file + direction;
    while (file != target_file + direction) {
        board[plc_chess_file(old_king_square)]
            [plc_chess_rank(old_king_square)] = 0;
        board[file][king_rank] = king_piece;
        if (plc_chess_side_in_check_board(board, side)) {
            return 1;
        }
        old_king_square = plc_chess_square(file, king_rank);
        file += direction;
    }
    if (can_castle != 0) {
        *can_castle = 1;
    }
    return 1;
}

int plc_chess_model_position_signature(PLC_FRAME *frame, int board_id,
    int *signature, char *err, unsigned err_size)
{
    int board[9][9];
    int file;
    int rank;
    int value;

    if (!plc_chess_load_board(frame, board_id, board, err, err_size)) {
        return 0;
    }
    value = 17;
    for (rank = 1; rank <= 8; ++rank) {
        for (file = 1; file <= 8; ++file) {
            value = (value * 31 + board[file][rank] * (file * 10 + rank))
                % 32767;
        }
    }
    if (signature != 0) {
        *signature = value;
    }
    return 1;
}

int plc_chess_model_en_passant(int from_square, int to_square,
    int last_from, int last_to, int side)
{
    int from_file;
    int from_rank;
    int to_file;
    int to_rank;
    int last_from_file;
    int last_from_rank;
    int last_to_file;
    int last_to_rank;

    if (!plc_chess_valid_square(from_square)
            || !plc_chess_valid_square(to_square)
            || !plc_chess_valid_square(last_from)
            || !plc_chess_valid_square(last_to)) {
        return 0;
    }
    from_file = plc_chess_file(from_square);
    from_rank = plc_chess_rank(from_square);
    to_file = plc_chess_file(to_square);
    to_rank = plc_chess_rank(to_square);
    last_from_file = plc_chess_file(last_from);
    last_from_rank = plc_chess_rank(last_from);
    last_to_file = plc_chess_file(last_to);
    last_to_rank = plc_chess_rank(last_to);
    if (last_from_file != last_to_file || abs(last_from_rank - last_to_rank) != 2) {
        return 0;
    }
    if (abs(from_file - last_to_file) != 1 || to_file != last_to_file) {
        return 0;
    }
    if (side == 1) {
        return from_rank == 5 && last_from_rank == 7
            && last_to_rank == 5 && to_rank == 6;
    }
    if (side == 2) {
        return from_rank == 4 && last_from_rank == 2
            && last_to_rank == 4 && to_rank == 3;
    }
    return 0;
}

int plc_chess_model_fen_signature(PLC_FRAME *frame, int board_id,
    int side, int castling, int ep_square, int *signature,
    char *err, unsigned err_size)
{
    int base;

    if (!plc_chess_model_position_signature(frame, board_id, &base,
            err, err_size)) {
        return 0;
    }
    if (signature != 0) {
        *signature = (base * 31 + side * 101 + castling * 17
            + ep_square * 13) % 32767;
    }
    return 1;
}
