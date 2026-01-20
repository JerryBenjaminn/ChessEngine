#include "OpeningBook.h"

#include <unordered_map>

#include "MoveGen.h"

namespace {
using BookMap = std::unordered_map<uint64_t, std::vector<std::string>>;

void AddLine(BookMap& book, const std::vector<std::string>& moves) {
    Board board;
    board.LoadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    for (const auto& uci : moves) {
        auto parsed = Move::ParseUci(uci);
        if (!parsed.has_value()) {
            return;
        }
        auto legal = GenerateLegalMoves(board);
        bool found = false;
        Move chosen = *parsed;
        for (const auto& move : legal) {
            if (move.ToUci() == parsed->ToUci()) {
                chosen = move;
                found = true;
                break;
            }
        }
        if (!found) {
            return;
        }
        book[board.Hash()].push_back(chosen.ToUci());
        MoveUndo undo = ApplyMove(board, chosen);
        board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
    }
}

const BookMap& GetBook() {
    static BookMap book;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        AddLine(book, {"e2e4", "e7e5", "g1f3", "b8c6"});
        AddLine(book, {"e2e4", "c7c5"});
        AddLine(book, {"d2d4", "d7d5"});
        AddLine(book, {"d2d4", "g8f6"});
        AddLine(book, {"c2c4"});
        AddLine(book, {"d2d4", "d7d5", "c1g5"});  // Trompowsky-style line
        AddLine(book, {"e2e4", "d7d5"});  // Scandinavian
    }
    return book;
}
}  // namespace

bool GetBookMove(const Board& board,
                 const std::vector<Move>& legalMoves,
                 int plyCount,
                 int maxBookPlies,
                 Move& outMove) {
    if (plyCount >= maxBookPlies) {
        return false;
    }

    const auto& book = GetBook();
    auto it = book.find(board.Hash());
    if (it == book.end()) {
        return false;
    }

    for (const auto& uci : it->second) {
        for (const auto& move : legalMoves) {
            if (move.ToUci() == uci) {
                outMove = move;
                return true;
            }
        }
    }

    return false;
}
