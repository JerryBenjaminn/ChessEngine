#include <cassert>
#include <cstdint>
#include <set>
#include <sstream>
#include <vector>

#include "Board.h"
#include "MoveGen.h"
#include "Move.h"
#include "Search.h"
#include "TranspositionTable.h"

int main() {
    auto move = Move::ParseUci("e2e4");
    assert(move.has_value());
    assert(move->ToUci() == "e2e4");

    auto a1_square = SquareFromString("a1");
    auto h8_square = SquareFromString("h8");
    assert(a1_square.has_value());
    assert(h8_square.has_value());
    assert(SquareToString(*a1_square).value() == "a1");
    assert(SquareToString(*h8_square).value() == "h8");

    auto promo = Move::ParseUci("e7e8q");
    assert(promo.has_value());
    assert(promo->ToUci() == "e7e8q");

    assert(!SquareFromString("e9").has_value());
    assert(!SquareFromString("i2").has_value());
    assert(!Move::ParseUci("e2").has_value());

    const std::string start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board board;
    assert(board.LoadFen(start_fen));

    int e1 = SquareFromString("e1").value();
    int e8 = SquareFromString("e8").value();
    assert(board.PieceAt(e1) == 'K');
    assert(board.PieceAt(e8) == 'k');

    std::string ascii = board.ToAscii();
    assert(ascii.find("♔") != std::string::npos);
    assert(ascii.find("♚") != std::string::npos);

    std::istringstream ascii_stream(ascii);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(ascii_stream, line)) {
        lines.push_back(line);
    }
    assert(lines.size() == 9);
    assert(lines[0] == "  a b c d e f g h");
    assert(lines[1].rfind("8 ♜", 0) == 0);
    assert(lines[8].rfind("1 ♖", 0) == 0);

    Board bad;
    assert(!bad.LoadFen("8/8/8/8/8/8/8 w - - 0 1"));

    auto moves = GeneratePseudoLegalMoves(board);
    assert(moves.size() == 20);

    int g1 = SquareFromString("g1").value();
    std::set<std::string> g1_moves;
    for (const auto& m : moves) {
        if (m.from() == g1) {
            g1_moves.insert(m.ToUci());
        }
    }
    assert(g1_moves.size() == 2);
    assert(g1_moves.count("g1f3") == 1);
    assert(g1_moves.count("g1h3") == 1);

    int e2 = SquareFromString("e2").value();
    std::set<std::string> e2_moves;
    for (const auto& m : moves) {
        if (m.from() == e2) {
            e2_moves.insert(m.ToUci());
        }
    }
    assert(e2_moves.size() == 2);
    assert(e2_moves.count("e2e3") == 1);
    assert(e2_moves.count("e2e4") == 1);

    int a1 = SquareFromString("a1").value();
    int a1_count = 0;
    for (const auto& m : moves) {
        if (m.from() == a1) {
            a1_count += 1;
        }
    }
    assert(a1_count == 0);

    const std::string black_pawn_fen = "8/4p3/8/8/8/8/8/8 b - - 0 1";
    Board black_pawn_board;
    assert(black_pawn_board.LoadFen(black_pawn_fen));
    auto black_moves = GeneratePseudoLegalMoves(black_pawn_board);
    int e7 = SquareFromString("e7").value();
    bool has_e7e6 = false;
    for (const auto& m : black_moves) {
        if (m.from() == e7 && m.ToUci() == "e7e6") {
            has_e7e6 = true;
            break;
        }
    }
    assert(has_e7e6);

    const std::string black_capture_fen = "8/8/8/4p3/3P1P2/8/8/8 b - - 0 1";
    Board black_capture_board;
    assert(black_capture_board.LoadFen(black_capture_fen));
    auto capture_moves = GeneratePseudoLegalMoves(black_capture_board);
    int e5 = SquareFromString("e5").value();
    bool has_capture = false;
    for (const auto& m : capture_moves) {
        if (m.from() == e5 && (m.ToUci() == "e5d4" || m.ToUci() == "e5f4")) {
            has_capture = true;
            break;
        }
    }
    assert(has_capture);

    const std::string pinned_fen = "4r3/8/8/8/8/8/4R3/4K3 w - - 0 1";
    Board pinned_board;
    assert(pinned_board.LoadFen(pinned_fen));
    auto pinned_legal = GenerateLegalMoves(pinned_board);
    bool has_illegal = false;
    for (const auto& m : pinned_legal) {
        if (m.ToUci() == "e2a2") {
            has_illegal = true;
            break;
        }
    }
    assert(!has_illegal);

    const std::string king_move_fen = "4k3/8/8/7r/4K3/8/8/8 w - - 0 1";
    Board king_move_board;
    assert(king_move_board.LoadFen(king_move_fen));
    auto king_moves = GenerateLegalMoves(king_move_board);
    bool has_e4e5 = false;
    for (const auto& m : king_moves) {
        if (m.ToUci() == "e4e5") {
            has_e4e5 = true;
            break;
        }
    }
    assert(!has_e4e5);

    const std::string check_fen = "4k3/8/8/8/8/8/4r3/4K3 w - - 0 1";
    Board check_board;
    assert(check_board.LoadFen(check_fen));
    assert(InCheck(check_board, Color::White));

    const std::string perft_start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
    Board perft_start_board;
    assert(perft_start_board.LoadFen(perft_start_fen));
    assert(Perft(perft_start_board, 1) == 20);
    assert(Perft(perft_start_board, 2) == 400);
    assert(Perft(perft_start_board, 3) == 8902);

    const std::string perft_krk_fen = "4k3/8/8/8/8/8/4R3/4K3 w - - 0 1";
    Board perft_krk_board;
    assert(perft_krk_board.LoadFen(perft_krk_fen));
    assert(Perft(perft_krk_board, 1) == 16);
    assert(Perft(perft_krk_board, 2) == 66);

    const std::string perft_kqk_fen = "4k3/8/8/8/8/8/4Q3/4K3 w - - 0 1";
    Board perft_kqk_board;
    assert(perft_kqk_board.LoadFen(perft_kqk_fen));
    assert(Perft(perft_kqk_board, 1) == 25);
    assert(Perft(perft_kqk_board, 2) == 94);

    const std::string promo_quiet_fen = "8/4P3/8/3b4/8/8/2k5/K7 w - - 0 1";
    Board promo_quiet_board;
    assert(promo_quiet_board.LoadFen(promo_quiet_fen));
    auto promo_quiet_moves = GenerateLegalMoves(promo_quiet_board);
    std::set<std::string> promo_quiet_set;
    for (const auto& m : promo_quiet_moves) {
        if (m.from() == SquareFromString("e7").value()) {
            promo_quiet_set.insert(m.ToUci());
        }
    }
    assert(promo_quiet_set.size() == 4);
    assert(promo_quiet_set.count("e7e8q") == 1);
    assert(promo_quiet_set.count("e7e8r") == 1);
    assert(promo_quiet_set.count("e7e8b") == 1);
    assert(promo_quiet_set.count("e7e8n") == 1);

    const std::string promo_capture_fen = "3r4/4P3/8/3b4/8/8/2k5/K7 w - - 0 1";
    Board promo_capture_board;
    assert(promo_capture_board.LoadFen(promo_capture_fen));
    auto promo_capture_moves = GenerateLegalMoves(promo_capture_board);
    std::set<std::string> promo_capture_set;
    for (const auto& m : promo_capture_moves) {
        if (m.from() == SquareFromString("e7").value() && m.ToUci().rfind("e7d8", 0) == 0) {
            promo_capture_set.insert(m.ToUci());
        }
    }
    assert(promo_capture_set.size() == 4);
    assert(promo_capture_set.count("e7d8q") == 1);
    assert(promo_capture_set.count("e7d8r") == 1);
    assert(promo_capture_set.count("e7d8b") == 1);
    assert(promo_capture_set.count("e7d8n") == 1);

    assert(Perft(promo_quiet_board, 1) == 4);
    assert(Perft(promo_quiet_board, 2) == 76);

    const std::string ep_fen = "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1";
    Board ep_board;
    assert(ep_board.LoadFen(ep_fen));
    auto ep_moves = GenerateLegalMoves(ep_board);
    bool has_ep = false;
    Move ep_move(0, 0);
    for (const auto& m : ep_moves) {
        if (m.ToUci() == "e5d6") {
            has_ep = true;
            ep_move = m;
            break;
        }
    }
    assert(has_ep);

    MoveUndo ep_undo = ApplyMove(ep_board, ep_move);
    int d5 = SquareFromString("d5").value();
    int e5_ep = SquareFromString("e5").value();
    int d6 = SquareFromString("d6").value();
    assert(ep_board.PieceAt(d5) == '.');
    assert(ep_board.PieceAt(e5_ep) == '.');
    assert(ep_board.PieceAt(d6) == 'P');
    UndoMoveApply(ep_board, ep_undo);
    assert(ep_board.PieceAt(d5) == 'p');
    assert(ep_board.PieceAt(e5_ep) == 'P');
    assert(ep_board.PieceAt(d6) == '.');

    const std::string ep_illegal_fen = "k3r3/8/8/3pP3/8/8/8/4K3 w - d6 0 1";
    Board ep_illegal_board;
    assert(ep_illegal_board.LoadFen(ep_illegal_fen));
    auto ep_illegal_moves = GenerateLegalMoves(ep_illegal_board);
    bool has_illegal_ep = false;
    for (const auto& m : ep_illegal_moves) {
        if (m.ToUci() == "e5d6") {
            has_illegal_ep = true;
            break;
        }
    }
    assert(!has_illegal_ep);

    assert(Perft(ep_board, 1) == 7);
    assert(Perft(ep_board, 2) == 38);

    const std::string castle_fen = "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1";
    Board castle_board;
    assert(castle_board.LoadFen(castle_fen));
    auto castle_moves = GenerateLegalMoves(castle_board);
    bool has_ks = false;
    bool has_qs = false;
    for (const auto& m : castle_moves) {
        if (m.ToUci() == "e1g1") {
            has_ks = true;
        } else if (m.ToUci() == "e1c1") {
            has_qs = true;
        }
    }
    assert(has_ks);
    assert(has_qs);

    const std::string castle_in_check_fen = "4k3/8/8/8/8/8/4r3/R3K2R w KQ - 0 1";
    Board castle_in_check_board;
    assert(castle_in_check_board.LoadFen(castle_in_check_fen));
    auto castle_in_check_moves = GenerateLegalMoves(castle_in_check_board);
    for (const auto& m : castle_in_check_moves) {
        assert(m.ToUci() != "e1g1");
        assert(m.ToUci() != "e1c1");
    }

    const std::string castle_attack_fen = "4k3/8/8/8/2b5/8/8/R3K2R w KQ - 0 1";
    Board castle_attack_board;
    assert(castle_attack_board.LoadFen(castle_attack_fen));
    auto castle_attack_moves = GenerateLegalMoves(castle_attack_board);
    bool has_attack_ks = false;
    bool has_attack_qs = false;
    for (const auto& m : castle_attack_moves) {
        if (m.ToUci() == "e1g1") {
            has_attack_ks = true;
        } else if (m.ToUci() == "e1c1") {
            has_attack_qs = true;
        }
    }
    assert(!has_attack_ks);
    assert(has_attack_qs);

    Move castle_move(0, 0);
    for (const auto& m : castle_moves) {
        if (m.ToUci() == "e1g1") {
            castle_move = m;
            break;
        }
    }
    MoveUndo castle_undo = ApplyMove(castle_board, castle_move);
    int g1_castle = SquareFromString("g1").value();
    int f1_castle = SquareFromString("f1").value();
    int h1_castle = SquareFromString("h1").value();
    int e1_castle = SquareFromString("e1").value();
    assert(castle_board.PieceAt(g1_castle) == 'K');
    assert(castle_board.PieceAt(f1_castle) == 'R');
    assert(castle_board.PieceAt(h1_castle) == '.');
    assert(castle_board.PieceAt(e1_castle) == '.');
    UndoMoveApply(castle_board, castle_undo);
    assert(castle_board.PieceAt(e1_castle) == 'K');
    assert(castle_board.PieceAt(h1_castle) == 'R');
    assert(castle_board.PieceAt(f1_castle) == '.');
    assert(castle_board.PieceAt(g1_castle) == '.');

    assert(Perft(castle_board, 1) == 26);
    assert(Perft(castle_board, 2) == 112);

    const std::string eval_fen_white = "8/8/8/8/8/8/4Q3/4K3 w - - 0 1";
    Board eval_board_white;
    assert(eval_board_white.LoadFen(eval_fen_white));
    assert(EvaluateMaterial(eval_board_white) > 0);

    const std::string eval_fen_black = "8/8/8/8/8/8/4Q3/4K3 b - - 0 1";
    Board eval_board_black;
    assert(eval_board_black.LoadFen(eval_fen_black));
    assert(EvaluateMaterial(eval_board_black) < 0);

    const std::string knight_a1_fen = "8/8/8/8/8/8/8/N3K2k w - - 0 1";
    Board knight_a1_board;
    assert(knight_a1_board.LoadFen(knight_a1_fen));
    const std::string knight_d4_fen = "8/8/8/8/3N4/8/8/4K2k w - - 0 1";
    Board knight_d4_board;
    assert(knight_d4_board.LoadFen(knight_d4_fen));
    assert(EvaluateMaterial(knight_d4_board) > EvaluateMaterial(knight_a1_board));

    const std::string bishop_undeveloped_fen = "8/8/8/8/8/8/8/2B1K2k w - - 0 1";
    Board bishop_undeveloped_board;
    assert(bishop_undeveloped_board.LoadFen(bishop_undeveloped_fen));
    const std::string bishop_developed_fen = "8/8/8/8/8/4B3/8/4K2k w - - 0 1";
    Board bishop_developed_board;
    assert(bishop_developed_board.LoadFen(bishop_developed_fen));
    assert(EvaluateMaterial(bishop_developed_board) > EvaluateMaterial(bishop_undeveloped_board));

    auto apply_and_undo = [](Board& b, const Move& move) {
        uint64_t start_hash = b.Hash();
        MoveUndo undo = ApplyMove(b, move);
        b.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
#ifdef HASH_DEBUG
        uint64_t before = b.Hash();
        b.RecomputeHash();
        assert(b.Hash() == before);
#endif
        UndoMoveApply(b, undo);
#ifdef HASH_DEBUG
        before = b.Hash();
        b.RecomputeHash();
        assert(b.Hash() == before);
#endif
        assert(b.Hash() == start_hash);
    };

    const std::string hash_start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board hash_start_board;
    assert(hash_start_board.LoadFen(hash_start_fen));
    auto hash_start_moves = GenerateLegalMoves(hash_start_board);
    bool found_start = false;
    for (const auto& m : hash_start_moves) {
        if (m.ToUci() == "e2e4") {
            apply_and_undo(hash_start_board, m);
            found_start = true;
            break;
        }
    }
    assert(found_start);

    const std::string hash_capture_fen = "4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1";
    Board hash_capture_board;
    assert(hash_capture_board.LoadFen(hash_capture_fen));
    auto hash_capture_moves = GenerateLegalMoves(hash_capture_board);
    bool found_halfmove_capture = false;
    for (const auto& m : hash_capture_moves) {
        if (m.ToUci() == "e4d5") {
            apply_and_undo(hash_capture_board, m);
            found_halfmove_capture = true;
            break;
        }
    }
    assert(found_halfmove_capture);

    const std::string hash_promo_fen = "8/4P3/8/3b4/8/8/2k5/K7 w - - 0 1";
    Board hash_promo_board;
    assert(hash_promo_board.LoadFen(hash_promo_fen));
    auto hash_promo_moves = GenerateLegalMoves(hash_promo_board);
    bool found_promo = false;
    for (const auto& m : hash_promo_moves) {
        if (m.ToUci() == "e7e8q") {
            apply_and_undo(hash_promo_board, m);
            found_promo = true;
            break;
        }
    }
    assert(found_promo);

    const std::string hash_ep_fen = "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1";
    Board hash_ep_board;
    assert(hash_ep_board.LoadFen(hash_ep_fen));
    auto hash_ep_moves = GenerateLegalMoves(hash_ep_board);
    bool found_ep = false;
    for (const auto& m : hash_ep_moves) {
        if (m.ToUci() == "e5d6") {
            apply_and_undo(hash_ep_board, m);
            found_ep = true;
            break;
        }
    }
    assert(found_ep);

    const std::string hash_castle_fen = "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1";
    Board hash_castle_board;
    assert(hash_castle_board.LoadFen(hash_castle_fen));
    auto hash_castle_moves = GenerateLegalMoves(hash_castle_board);
    bool found_castle = false;
    for (const auto& m : hash_castle_moves) {
        if (m.ToUci() == "e1g1") {
            apply_and_undo(hash_castle_board, m);
            found_castle = true;
            break;
        }
    }
    assert(found_castle);

    const std::string halfmove_pawn_fen = "4k3/8/8/8/8/8/4P3/4K3 w - - 10 1";
    Board halfmove_pawn_board;
    assert(halfmove_pawn_board.LoadFen(halfmove_pawn_fen));
    auto halfmove_pawn_moves = GenerateLegalMoves(halfmove_pawn_board);
    bool found_pawn = false;
    for (const auto& m : halfmove_pawn_moves) {
        if (m.ToUci() == "e2e3") {
            MoveUndo undo = ApplyMove(halfmove_pawn_board, m);
            halfmove_pawn_board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
            assert(halfmove_pawn_board.HalfmoveClock() == 0);
            UndoMoveApply(halfmove_pawn_board, undo);
            assert(halfmove_pawn_board.HalfmoveClock() == 10);
            found_pawn = true;
            break;
        }
    }
    assert(found_pawn);

    const std::string halfmove_quiet_fen = "4k3/8/8/8/8/8/8/4K3 w - - 10 1";
    Board halfmove_quiet_board;
    assert(halfmove_quiet_board.LoadFen(halfmove_quiet_fen));
    auto halfmove_quiet_moves = GenerateLegalMoves(halfmove_quiet_board);
    bool found_quiet = false;
    for (const auto& m : halfmove_quiet_moves) {
        if (m.ToUci() == "e1e2") {
            MoveUndo undo = ApplyMove(halfmove_quiet_board, m);
            halfmove_quiet_board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
            assert(halfmove_quiet_board.HalfmoveClock() == 11);
            UndoMoveApply(halfmove_quiet_board, undo);
            assert(halfmove_quiet_board.HalfmoveClock() == 10);
            found_quiet = true;
            break;
        }
    }
    assert(found_quiet);

    const std::string halfmove_capture_fen = "4k3/8/8/3p4/4P3/8/8/4K3 w - - 10 1";
    Board halfmove_capture_board;
    assert(halfmove_capture_board.LoadFen(halfmove_capture_fen));
    auto halfmove_capture_moves = GenerateLegalMoves(halfmove_capture_board);
    bool found_capture = false;
    for (const auto& m : halfmove_capture_moves) {
        if (m.ToUci() == "e4d5") {
            MoveUndo undo = ApplyMove(halfmove_capture_board, m);
            halfmove_capture_board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
            assert(halfmove_capture_board.HalfmoveClock() == 0);
            UndoMoveApply(halfmove_capture_board, undo);
            assert(halfmove_capture_board.HalfmoveClock() == 10);
            found_capture = true;
            break;
        }
    }
    assert(found_capture);

    TranspositionTable tt(1024);
    tt.Clear();
    Move best(4, 6);
    tt.Store(1234ULL, 3, 42, Bound::EXACT, &best);
    int outScore = 0;
    Move outMove(0, 0);
    assert(tt.Probe(1234ULL, 3, -100, 100, outScore, outMove));
    assert(outScore == 42);
    assert(outMove.ToUci() == "e1g1");
    assert(tt.Probe(1234ULL, 2, -100, 100, outScore, outMove));

    tt.Store(5555ULL, 2, 50, Bound::LOWER, nullptr);
    int lowerScore = 0;
    assert(tt.Probe(5555ULL, 2, -100, 40, lowerScore, outMove));
    assert(lowerScore == 50);
    assert(!tt.Probe(5555ULL, 2, -100, 60, lowerScore, outMove));

    tt.Store(6666ULL, 2, -20, Bound::UPPER, nullptr);
    int upperScore = 0;
    assert(tt.Probe(6666ULL, 2, -10, 100, upperScore, outMove));
    assert(upperScore == -20);
    assert(!tt.Probe(6666ULL, 2, -30, 100, upperScore, outMove));

    tt.Store(7777ULL, 1, 5, Bound::EXACT, nullptr);
    tt.Store(7777ULL, 4, 9, Bound::EXACT, nullptr);
    int replaceScore = 0;
    assert(tt.Probe(7777ULL, 4, -100, 100, replaceScore, outMove));
    assert(replaceScore == 9);

    return 0;
}
