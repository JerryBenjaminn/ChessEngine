#pragma once

#include <array>
#include <string>

// Square indexing: 0..63 where a1=0, b1=1, ..., h1=7, a2=8, ..., h8=63.
class Board {
public:
    Board();

    bool LoadFen(const std::string& fen);
    std::string ToAscii() const;

    char PieceAt(int index) const;

private:
    std::array<char, 64> squares_;
    char side_to_move_;
    std::string castling_rights_;
    int en_passant_square_;
};
