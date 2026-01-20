#include <cassert>
#include <sstream>
#include <vector>

#include "Board.h"
#include "Move.h"

int main() {
    auto move = Move::ParseUci("e2e4");
    assert(move.has_value());
    assert(move->ToUci() == "e2e4");

    auto a1 = SquareFromString("a1");
    auto h8 = SquareFromString("h8");
    assert(a1.has_value());
    assert(h8.has_value());
    assert(SquareToString(*a1).value() == "a1");
    assert(SquareToString(*h8).value() == "h8");

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

    return 0;
}
