#include "Move.h"

namespace {
bool IsFile(char c) {
    return c >= 'a' && c <= 'h';
}

bool IsRank(char c) {
    return c >= '1' && c <= '8';
}

bool IsPromotion(char c) {
    return c == 'q' || c == 'r' || c == 'b' || c == 'n';
}
}  // namespace

std::optional<int> SquareFromString(const std::string& text) {
    if (text.size() != 2) {
        return std::nullopt;
    }

    if (!IsFile(text[0]) || !IsRank(text[1])) {
        return std::nullopt;
    }

    int file = text[0] - 'a';
    int rank = text[1] - '1';
    return rank * 8 + file;
}

std::optional<std::string> SquareToString(int index) {
    if (index < 0 || index > 63) {
        return std::nullopt;
    }

    int file = index % 8;
    int rank = index / 8;
    std::string text;
    text.push_back(static_cast<char>('a' + file));
    text.push_back(static_cast<char>('1' + rank));
    return text;
}

Move::Move(int from_square, int to_square, std::optional<char> promotion)
    : from_square_(from_square), to_square_(to_square), promotion_(promotion) {}

int Move::from() const {
    return from_square_;
}

int Move::to() const {
    return to_square_;
}

std::optional<char> Move::promotion() const {
    return promotion_;
}

std::optional<Move> Move::ParseUci(const std::string& text) {
    if (text.size() != 4 && text.size() != 5) {
        return std::nullopt;
    }

    auto from = SquareFromString(text.substr(0, 2));
    auto to = SquareFromString(text.substr(2, 2));
    if (!from || !to) {
        return std::nullopt;
    }

    std::optional<char> promotion;
    if (text.size() == 5) {
        char promo = text[4];
        if (!IsPromotion(promo)) {
            return std::nullopt;
        }
        promotion = promo;
    }

    return Move(*from, *to, promotion);
}

std::string Move::ToUci() const {
    auto from_text = SquareToString(from_square_);
    auto to_text = SquareToString(to_square_);
    if (!from_text || !to_text) {
        return {};
    }

    std::string text = *from_text + *to_text;
    if (promotion_) {
        text.push_back(*promotion_);
    }
    return text;
}
