#include "Board.h"

#include <cctype>
#include <cstdlib>
#include <sstream>

#include "Move.h"

namespace {
bool zobrist_initialized = false;
uint64_t zobrist_piece_keys[12][64];
uint64_t zobrist_castling_keys[16];
uint64_t zobrist_enpassant_file_keys[8];
uint64_t zobrist_side_key = 0;

uint64_t SplitMix64(uint64_t& state) {
    uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

void InitZobrist() {
    if (zobrist_initialized) {
        return;
    }
    uint64_t seed = 0xC0FFEE1234567890ULL;
    for (int p = 0; p < 12; ++p) {
        for (int sq = 0; sq < 64; ++sq) {
            zobrist_piece_keys[p][sq] = SplitMix64(seed);
        }
    }
    for (int i = 0; i < 16; ++i) {
        zobrist_castling_keys[i] = SplitMix64(seed);
    }
    for (int i = 0; i < 8; ++i) {
        zobrist_enpassant_file_keys[i] = SplitMix64(seed);
    }
    zobrist_side_key = SplitMix64(seed);
    zobrist_initialized = true;
}

int PieceIndex(char piece) {
    switch (piece) {
        case 'P':
            return 0;
        case 'N':
            return 1;
        case 'B':
            return 2;
        case 'R':
            return 3;
        case 'Q':
            return 4;
        case 'K':
            return 5;
        case 'p':
            return 6;
        case 'n':
            return 7;
        case 'b':
            return 8;
        case 'r':
            return 9;
        case 'q':
            return 10;
        case 'k':
            return 11;
        default:
            return -1;
    }
}

int CastlingMask(const std::string& rights) {
    if (rights == "-" || rights.empty()) {
        return 0;
    }
    int mask = 0;
    if (rights.find('K') != std::string::npos) {
        mask |= 1;
    }
    if (rights.find('Q') != std::string::npos) {
        mask |= 2;
    }
    if (rights.find('k') != std::string::npos) {
        mask |= 4;
    }
    if (rights.find('q') != std::string::npos) {
        mask |= 8;
    }
    return mask;
}

bool IsPieceChar(char c) {
    switch (c) {
        case 'P':
        case 'N':
        case 'B':
        case 'R':
        case 'Q':
        case 'K':
        case 'p':
        case 'n':
        case 'b':
        case 'r':
        case 'q':
        case 'k':
            return true;
        default:
            return false;
    }
}

bool IsCastlingChar(char c) {
    return c == 'K' || c == 'Q' || c == 'k' || c == 'q';
}

const char* PieceToSymbol(char piece) {
    switch (piece) {
        case 'P':
            return "♙";
        case 'R':
            return "♖";
        case 'N':
            return "♘";
        case 'B':
            return "♗";
        case 'Q':
            return "♕";
        case 'K':
            return "♔";
        case 'p':
            return "♟";
        case 'r':
            return "♜";
        case 'n':
            return "♞";
        case 'b':
            return "♝";
        case 'q':
            return "♛";
        case 'k':
            return "♚";
        default:
            return "·";
    }
}
}  // namespace

Board::Board()
    : squares_(),
      side_to_move_('w'),
      castling_rights_("-"),
      en_passant_square_(-1),
      halfmove_clock_(0),
      hash_(0) {
    squares_.fill('.');
    RecomputeHash();
}

bool Board::LoadFen(const std::string& fen) {
    squares_.fill('.');
    side_to_move_ = 'w';
    castling_rights_ = "-";
    en_passant_square_ = -1;

    std::istringstream iss(fen);
    std::string board_part;
    std::string side_part;
    std::string castling_part;
    std::string en_passant_part;
    std::string halfmove_part;
    if (!(iss >> board_part >> side_part >> castling_part >> en_passant_part)) {
        return false;
    }
    if (iss >> halfmove_part) {
        try {
            halfmove_clock_ = std::stoi(halfmove_part);
            if (halfmove_clock_ < 0) {
                return false;
            }
        } catch (...) {
            return false;
        }
    } else {
        halfmove_clock_ = 0;
    }

    int rank = 7;
    int file = 0;
    for (char c : board_part) {
        if (c == '/') {
            if (file != 8 || rank == 0) {
                return false;
            }
            rank -= 1;
            file = 0;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            int empty = c - '0';
            if (empty < 1 || empty > 8 || file + empty > 8) {
                return false;
            }
            file += empty;
            continue;
        }

        if (!IsPieceChar(c) || file >= 8) {
            return false;
        }

        squares_[rank * 8 + file] = c;
        file += 1;
    }

    if (rank != 0 || file != 8) {
        return false;
    }

    if (side_part != "w" && side_part != "b") {
        return false;
    }
    side_to_move_ = side_part[0];

    if (castling_part != "-") {
        std::string seen;
        for (char c : castling_part) {
            if (!IsCastlingChar(c) || seen.find(c) != std::string::npos) {
                return false;
            }
            seen.push_back(c);
        }
        castling_rights_ = castling_part;
    }

    if (en_passant_part != "-") {
        auto square = SquareFromString(en_passant_part);
        if (!square) {
            return false;
        }
        en_passant_square_ = *square;
    }

    RecomputeHash();
    return true;
}

std::string Board::ToAscii() const {
    std::ostringstream out;
    out << "  a b c d e f g h\n";
    for (int rank = 7; rank >= 0; --rank) {
        out << (rank + 1) << ' ';
        for (int file = 0; file < 8; ++file) {
            char piece = squares_[rank * 8 + file];
            out << PieceToSymbol(piece);
            if (file < 7) {
                out << ' ';
            }
        }
        if (rank > 0) {
            out << '\n';
        }
    }
    return out.str();
}

char Board::PieceAt(int index) const {
    if (index < 0 || index > 63) {
        return '\0';
    }
    return squares_[index];
}

char Board::SideToMove() const {
    return side_to_move_;
}

void Board::SetPieceAt(int index, char piece) {
    if (index < 0 || index > 63) {
        return;
    }
    InitZobrist();
    char old = squares_[index];
    int old_index = PieceIndex(old);
    if (old_index >= 0) {
        hash_ ^= zobrist_piece_keys[old_index][index];
    }
    squares_[index] = piece;
    int new_index = PieceIndex(piece);
    if (new_index >= 0) {
        hash_ ^= zobrist_piece_keys[new_index][index];
    }
}

void Board::SetSideToMove(char side) {
    if (side == 'w' || side == 'b') {
        if (side_to_move_ == side) {
            return;
        }
        InitZobrist();
        if (side_to_move_ == 'b') {
            hash_ ^= zobrist_side_key;
        }
        if (side == 'b') {
            hash_ ^= zobrist_side_key;
        }
        side_to_move_ = side;
    }
}

int Board::EnPassantSquare() const {
    return en_passant_square_;
}

void Board::SetEnPassantSquare(int square) {
    if (square < -1 || square > 63) {
        return;
    }
    if (en_passant_square_ == square) {
        return;
    }
    InitZobrist();
    if (en_passant_square_ != -1) {
        int old_file = en_passant_square_ % 8;
        hash_ ^= zobrist_enpassant_file_keys[old_file];
    }
    en_passant_square_ = square;
    if (en_passant_square_ != -1) {
        int new_file = en_passant_square_ % 8;
        hash_ ^= zobrist_enpassant_file_keys[new_file];
    }
}

const std::string& Board::CastlingRights() const {
    return castling_rights_;
}

void Board::SetCastlingRights(const std::string& rights) {
    InitZobrist();
    int old_mask = CastlingMask(castling_rights_);
    std::string normalized = rights.empty() ? "-" : rights;
    int new_mask = CastlingMask(normalized);
    if (old_mask != new_mask) {
        hash_ ^= zobrist_castling_keys[old_mask];
        hash_ ^= zobrist_castling_keys[new_mask];
    }
    castling_rights_ = normalized;
}

int Board::HalfmoveClock() const {
    return halfmove_clock_;
}

void Board::SetHalfmoveClock(int halfmove) {
    if (halfmove < 0) {
        return;
    }
    halfmove_clock_ = halfmove;
}

uint64_t Board::Hash() const {
    return hash_;
}

void Board::RecomputeHash() {
    InitZobrist();
    hash_ = 0;
    for (int i = 0; i < 64; ++i) {
        int idx = PieceIndex(squares_[i]);
        if (idx >= 0) {
            hash_ ^= zobrist_piece_keys[idx][i];
        }
    }
    if (side_to_move_ == 'b') {
        hash_ ^= zobrist_side_key;
    }
    int mask = CastlingMask(castling_rights_);
    hash_ ^= zobrist_castling_keys[mask];
    if (en_passant_square_ != -1) {
        int file = en_passant_square_ % 8;
        hash_ ^= zobrist_enpassant_file_keys[file];
    }
}

#ifdef CHESSENGINE_DEBUG
void Board::DebugAssertValid() const {
    int white_king = 0;
    int black_king = 0;
    for (int i = 0; i < 64; ++i) {
        if (squares_[i] == 'K') {
            white_king += 1;
        } else if (squares_[i] == 'k') {
            black_king += 1;
        }
    }
    if (white_king != 1 || black_king != 1) {
        std::cerr << "DEBUG ASSERT: invalid king count\n";
        std::abort();
    }
    if (side_to_move_ != 'w' && side_to_move_ != 'b') {
        std::cerr << "DEBUG ASSERT: invalid side to move\n";
        std::abort();
    }
    if (en_passant_square_ != -1) {
        int rank = en_passant_square_ / 8;
        if (!(rank == 2 || rank == 5)) {
            std::cerr << "DEBUG ASSERT: invalid en passant square\n";
            std::abort();
        }
    }
    if (castling_rights_ != "-") {
        for (char c : castling_rights_) {
            if (c != 'K' && c != 'Q' && c != 'k' && c != 'q') {
                std::cerr << "DEBUG ASSERT: invalid castling rights\n";
                std::abort();
            }
        }
    }
}
#endif
