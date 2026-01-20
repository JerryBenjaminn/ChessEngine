#include <iostream>

#include "Board.h"
#include "ConsoleRenderer.h"

int main() {
    const std::string start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board board;
    if (!board.LoadFen(start_fen)) {
        std::cerr << "Failed to load FEN.\n";
        return 1;
    }

    RenderBoard(board, true);
    return 0;
}
