#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "Board.h"
#include "ConsoleRenderer.h"
#include "MoveGen.h"
#include "OpeningBook.h"
#include "Search.h"

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    const std::string start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board board;
    if (!board.LoadFen(start_fen)) {
        std::cerr << "Failed to load FEN.\n";
        return 1;
    }

    int mode = 1;
    std::cout << "Choose mode: 1) Human vs AI  2) AI vs AI : ";
    std::string mode_input;
    if (std::getline(std::cin, mode_input)) {
        if (!mode_input.empty() && mode_input[0] == '2') {
            mode = 2;
        }
    }

    char human_side = 'w';
    int ai_delay_ms = 0;
    if (mode == 1) {
        std::cout << "Choose side (w/b): ";
        std::string side_input;
        if (std::getline(std::cin, side_input)) {
            if (!side_input.empty() && (side_input[0] == 'w' || side_input[0] == 'b')) {
                human_side = side_input[0];
            }
        }
    } else {
        std::cout << "AI vs AI speed (fast/slow): ";
        std::string speed_input;
        if (std::getline(std::cin, speed_input)) {
            if (speed_input == "slow") {
                ai_delay_ms = 500;
            }
        }
    }

    int ply_count = 0;
    const int max_book_plies = 6;
    bool book_enabled = true;
    bool expert_mode = false;
    const int max_ai_plies = 300;
    std::unordered_map<uint64_t, int> repetition_counts;
    repetition_counts.clear();
    repetition_counts[board.Hash()] = 1;
    std::vector<std::pair<uint64_t, std::string>> repetition_history;
    repetition_history.emplace_back(board.Hash(), "start");
#ifdef NDEBUG
    bool debug_log = false;
#else
    bool debug_log = true;
#endif
    int ai_budget_ms = mode == 2 ? 1000 : 500;

    while (true) {
        auto legal_moves = GenerateLegalMoves(board);
        if (legal_moves.empty()) {
            Color side = board.SideToMove() == 'w' ? Color::White : Color::Black;
            if (InCheck(board, side)) {
                std::cout << "GAME OVER: checkmate\n";
            } else {
                std::cout << "GAME OVER: stalemate\n";
            }
            return 0;
        }

        if (board.HalfmoveClock() >= 100) {
            std::cout << "Draw by 50-move rule (halfmove=" << board.HalfmoveClock() << ").\n";
            return 0;
        }

        if (mode == 2 && ply_count >= max_ai_plies) {
            std::cout << "Draw by max plies (ply=" << ply_count << ", limit=" << max_ai_plies << ").\n";
            return 0;
        }

        RenderBoard(board, true);

        if (mode == 1 && board.SideToMove() == human_side) {
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
                if (input == "debug on") {
                    debug_log = true;
                    std::cout << "Debug logging enabled.\n";
                    continue;
                }
                if (input == "debug off") {
                    debug_log = false;
                    std::cout << "Debug logging disabled.\n";
                    continue;
                }
                if (input.rfind("time", 0) == 0) {
                    std::string value = input.size() > 4 ? input.substr(4) : "";
                    if (!value.empty() && value[0] == ' ') {
                        value.erase(0, value.find_first_not_of(' '));
                    }
                    if (value.empty()) {
                        std::cout << "Time budget: " << ai_budget_ms << " ms\n";
                        continue;
                    }
                    try {
                        int ms = std::stoi(value);
                        if (ms < 50) {
                            ms = 50;
                        } else if (ms > 5000) {
                            ms = 5000;
                        }
                        ai_budget_ms = ms;
                        std::cout << "Time budget set to " << ai_budget_ms << " ms\n";
                    } catch (...) {
                        std::cout << "Time budget: " << ai_budget_ms << " ms\n";
                    }
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
                repetition_history.emplace_back(board.Hash(), chosen.ToUci());
                int& count = repetition_counts[board.Hash()];
                count += 1;
                if (count >= 3) {
                    std::cout << "Draw by repetition (hash=" << board.Hash() << ", count=" << count
                              << ").\n";
                    if (debug_log) {
                        std::cout << "Recent plies:\n";
                        int start = static_cast<int>(repetition_history.size()) - 3;
                        if (start < 0) {
                            start = 0;
                        }
                        for (size_t i = static_cast<size_t>(start); i < repetition_history.size(); ++i) {
                            std::cout << repetition_history[i].second << " hash=" << repetition_history[i].first
                                      << '\n';
                        }
                    }
                    return 0;
                }
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
                bool depth1_completed = false;
                int timed_out_depth = -1;
                int root_move_count = 0;
                const int budget_ms = ai_budget_ms;
                auto start_time = std::chrono::steady_clock::now();
                auto deadline = start_time + std::chrono::milliseconds(budget_ms);
                int rep_count = repetition_counts[board.Hash()];
                int max_plies = mode == 2 ? max_ai_plies : -1;
                SetSearchDrawContext(ply_count, max_plies, rep_count);
                int score = SearchBestMoveTimed(board,
                                                3,
                                                deadline,
                                                best,
                                                depth_reached,
                                                nodes,
                                                qnodes,
                                                depth1_completed,
                                                timed_out_depth,
                                                root_move_count);
                auto end_time = std::chrono::steady_clock::now();
                auto start_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(start_time.time_since_epoch()).count();
                auto deadline_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(deadline.time_since_epoch()).count();
                auto end_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(end_time.time_since_epoch()).count();
                if (debug_log) {
                    std::cout << "AI diag: budget_ms=" << budget_ms << " start_ms=" << start_ms
                              << " deadline_ms=" << deadline_ms << " end_ms=" << end_ms
                              << " depth=" << depth_reached << " nodes=" << nodes << " qnodes=" << qnodes
                              << " root_moves=" << root_move_count
                              << " depth1_completed=" << (depth1_completed ? "true" : "false")
                              << " timed_out_depth=" << timed_out_depth << '\n';
                }
                if (depth_reached == 0) {
                    best = legal_moves.front();
                    if (debug_log) {
                        std::cout << "TIME FALLBACK\n";
                    }
                }
                std::cout << "AI plays: " << best.ToUci() << " (depth " << depth_reached
                          << ", score " << score << ", nodes " << nodes << ", qnodes " << qnodes << ")\n";
            }
            MoveUndo undo = ApplyMove(board, best);
            board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
            ply_count += 1;
            repetition_history.emplace_back(board.Hash(), best.ToUci());
            int& count = repetition_counts[board.Hash()];
            count += 1;
            if (count >= 3) {
                std::cout << "Draw by repetition (hash=" << board.Hash() << ", count=" << count << ").\n";
                if (debug_log) {
                    std::cout << "Recent plies:\n";
                    int start = static_cast<int>(repetition_history.size()) - 3;
                    if (start < 0) {
                        start = 0;
                    }
                    for (size_t i = static_cast<size_t>(start); i < repetition_history.size(); ++i) {
                        std::cout << repetition_history[i].second << " hash=" << repetition_history[i].first
                                  << '\n';
                    }
                }
                return 0;
            }
            if (mode == 2 && ai_delay_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(ai_delay_ms));
            }
        }
    }

    return 0;
}
