#pragma once

#include <optional>
#include <string>

// Square indexing: 0..63 where a1=0, b1=1, ..., h1=7, a2=8, ..., h8=63.
std::optional<int> SquareFromString(const std::string& text);
std::optional<std::string> SquareToString(int index);

class Move {
public:
    Move(int from_square, int to_square, std::optional<char> promotion = std::nullopt);

    int from() const;
    int to() const;
    std::optional<char> promotion() const;

    static std::optional<Move> ParseUci(const std::string& text);
    std::string ToUci() const;

private:
    int from_square_;
    int to_square_;
    std::optional<char> promotion_;
};
