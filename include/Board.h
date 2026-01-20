#pragma once

#include <array>
#include <cstdint>
#include <string>

// Square indexing: 0..63 where a1=0, b1=1, ..., h1=7, a2=8, ..., h8=63.
class Board {
public:
    Board();

    bool LoadFen(const std::string& fen);
    std::string ToAscii() const;

    char PieceAt(int index) const;
    char SideToMove() const;
    void SetPieceAt(int index, char piece);
    void SetSideToMove(char side);
    int EnPassantSquare() const;
    void SetEnPassantSquare(int square);
    const std::string& CastlingRights() const;
    void SetCastlingRights(const std::string& rights);
    int HalfmoveClock() const;
    void SetHalfmoveClock(int halfmove);
    uint64_t Hash() const;
    void RecomputeHash();
#ifdef CHESSENGINE_DEBUG
    void DebugAssertValid() const;
#endif

private:
    std::array<char, 64> squares_;
    char side_to_move_;
    std::string castling_rights_;
    int en_passant_square_;
    int halfmove_clock_;
    uint64_t hash_;
};
