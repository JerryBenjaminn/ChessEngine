#pragma once

#include <optional>
#include <string>

class Move {
public:
    Move(std::string from_square, std::string to_square);

    const std::string& from() const;
    const std::string& to() const;

    static std::optional<Move> ParseUci(const std::string& text);

private:
    std::string from_square_;
    std::string to_square_;
};
