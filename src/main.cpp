#include <chrono>
#include <iostream>
#include <string>

#include "Board.h"
#include "ConsoleRenderer.h"
#include "MoveGen.h"
#include "OpeningBook.h"
#include "Search.h"

int main() {
    const std::string start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board board;
    if (!board.LoadFen(start_fen)) {
        std::cerr << "Failed to load FEN.\n";
        return 1;
    }

    int ply_count = 0;
    const int max_book_plies = 6;
    bool book_enabled = true;
    bool expert_mode = false;

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
                if (input == "book on") {
                    book_enabled = true;
                    std::cout << "Opening book enabled.\n";
                    continue;
                }
                if (input == "book off") {
                    book_enabled = false;
                    std::cout << "Opening book disabled.\n";
                    continue;
                }
                if (input == "expert") {
                    expert_mode = !expert_mode;
                    std::cout << (expert_mode ? "Expert mode enabled.\n" : "Expert mode disabled.\n");
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
                ply_count += 1;
                break;
            }
        } else {
            Move best(0, 0);
            if (book_enabled && !expert_mode && GetBookMove(board, legal_moves, ply_count, max_book_plies, best)) {
                std::cout << "AI plays (book): " << best.ToUci() << '\n';
            } else {
                int depth_reached = 0;
                uint64_t nodes = 0;
                uint64_t qnodes = 0;
                auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
                int score = SearchBestMoveTimed(board, 3, deadline, best, depth_reached, nodes, qnodes);
                std::cout << "AI plays: " << best.ToUci() << " (depth " << depth_reached
                          << ", score " << score << ", nodes " << nodes << ", qnodes " << qnodes << ")\n";
            }
            MoveUndo undo = ApplyMove(board, best);
            board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
            ply_count += 1;
        }
    }

    return 0;
}
