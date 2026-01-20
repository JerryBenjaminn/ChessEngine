#include <cassert>
#include <set>
#include <sstream>
#include <vector>

#include "Board.h"
#include "MoveGen.h"
#include "Move.h"

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

    return 0;
}
