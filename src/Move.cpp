#include "Move.h"

Move::Move(std::string from_square, std::string to_square)
    : from_square_(std::move(from_square)), to_square_(std::move(to_square)) {}

const std::string& Move::from() const {
    return from_square_;
}

const std::string& Move::to() const {
    return to_square_;
}

static bool IsFile(char c) {
    return c >= 'a' && c <= 'h';
}

static bool IsRank(char c) {
    return c >= '1' && c <= '8';
}

std::optional<Move> Move::ParseUci(const std::string& text) {
    if (text.size() != 4) {
        return std::nullopt;
    }

    if (!IsFile(text[0]) || !IsRank(text[1]) || !IsFile(text[2]) || !IsRank(text[3])) {
        return std::nullopt;
    }

    return Move(text.substr(0, 2), text.substr(2, 2));
}
