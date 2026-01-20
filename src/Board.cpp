#include "Board.h"

#include <cctype>
#include <sstream>

#include "Move.h"

namespace {
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
      en_passant_square_(-1) {
    squares_.fill('.');
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
    if (!(iss >> board_part >> side_part >> castling_part >> en_passant_part)) {
        return false;
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
