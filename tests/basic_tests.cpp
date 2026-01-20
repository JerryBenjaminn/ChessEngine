#include <cassert>

#include "Move.h"

int main() {
    auto move = Move::ParseUci("e2e4");
    assert(move.has_value());
    assert(move->ToUci() == "e2e4");

    auto a1 = SquareFromString("a1");
    auto h8 = SquareFromString("h8");
    assert(a1.has_value());
    assert(h8.has_value());
    assert(SquareToString(*a1).value() == "a1");
    assert(SquareToString(*h8).value() == "h8");

    auto promo = Move::ParseUci("e7e8q");
    assert(promo.has_value());
    assert(promo->ToUci() == "e7e8q");

    assert(!SquareFromString("e9").has_value());
    assert(!SquareFromString("i2").has_value());
    assert(!Move::ParseUci("e2").has_value());

    return 0;
}
