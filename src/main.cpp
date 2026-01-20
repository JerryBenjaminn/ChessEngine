#include <chrono>
#include <iostream>
#include <string>

#include "Board.h"
#include "ConsoleRenderer.h"
#include "MoveGen.h"
#include "Search.h"

int main() {
    const std::string start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board board;
    if (!board.LoadFen(start_fen)) {
        std::cerr << "Failed to load FEN.\n";
        return 1;
    }

    while (true) {
        auto legal_moves = GenerateLegalMoves(board);
        if (legal_moves.empty()) {
            Color side = board.SideToMove() == 'w' ? Color::White : Color::Black;
            if (InCheck(board, side)) {
                std::cout << (board.SideToMove() == 'w' ? "Black wins by checkmate.\n"
                                                        : "White wins by checkmate.\n");
            } else {
                std::cout << "Stalemate.\n";
            }
            break;
        }

        RenderBoard(board, true);

        if (board.SideToMove() == 'w') {
            while (true) {
                std::cout << "Your move: ";
                std::string input;
                if (!std::getline(std::cin, input)) {
                    return 0;
                }
                if (input == "quit") {
                    return 0;
                }
                if (input == "moves") {
                    for (const auto& move : legal_moves) {
                        std::cout << move.ToUci() << ' ';
                    }
                    std::cout << '\n';
                    continue;
                }

                auto parsed = Move::ParseUci(input);
                if (!parsed.has_value()) {
                    std::cout << "Illegal move\n";
                    continue;
                }

                bool found = false;
                Move chosen = *parsed;
                for (const auto& move : legal_moves) {
                    if (move.ToUci() == chosen.ToUci()) {
                        chosen = move;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    std::cout << "Illegal move\n";
                    continue;
                }

                MoveUndo undo = ApplyMove(board, chosen);
                board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
                break;
            }
        } else {
            Move best(0, 0);
            int depth_reached = 0;
            uint64_t nodes = 0;
            uint64_t qnodes = 0;
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
            int score = SearchBestMoveTimed(board, 3, deadline, best, depth_reached, nodes, qnodes);
            std::cout << "AI plays: " << best.ToUci() << " (depth " << depth_reached << ", score "
                      << score << ", nodes " << nodes << ", qnodes " << qnodes << ")\n";
            MoveUndo undo = ApplyMove(board, best);
            board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
        }
    }

    return 0;
}
