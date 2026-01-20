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

    return 0;
}
