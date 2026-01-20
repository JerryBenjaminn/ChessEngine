#include <cassert>

#include "Move.h"

int main() {
    auto move = Move::ParseUci("e2e4");
    assert(move.has_value());
    assert(move->from() == "e2");
    assert(move->to() == "e4");
    return 0;
}
