## Technical Overview

### Project Summary
This project is a console-based chess engine that lets a human play as White against a simple AI playing Black. It renders the board to the console and supports move input in UCI format.

Key features implemented:
- Legal move generation with king safety filtering.
- Special moves: castling, en passant, and promotions.
- FEN loading for positions.
- Zobrist hashing and a transposition table (integrated in search).
- Iterative deepening search with a fixed time budget.
- Opening book for the first few plies (optional).

### Build & Run
- Build:
  - `cmake -S . -B build`
  - `cmake --build build`
- Tests:
  - `ctest --test-dir build`
- Run the game:
  - `build/chessengine.exe` (Windows) or `./build/chessengine` (other)
- Windows Unicode note:
  - For proper Unicode piece display, you may need `chcp 65001` in the console.

### Codebase Structure
- `include/`: Public headers for core modules.
- `src/`: Implementations of engine modules and the console UI.
- `tests/`: Unit tests and perft checks.

Important modules/classes:
- `Board` (`include/Board.h`, `src/Board.cpp`): game state and FEN loading.
- `Move` (`include/Move.h`, `src/Move.cpp`): UCI move parsing/formatting.
- `MoveGen` (`include/MoveGen.h`, `src/MoveGen.cpp`): move generation and legality.
- `Search` (`include/Search.h`, `src/Search.cpp`): evaluation and alpha-beta search.
- `TranspositionTable` (`include/TranspositionTable.h`, `src/TranspositionTable.cpp`): TT with bounds.
- `OpeningBook` (`include/OpeningBook.h`, `src/OpeningBook.cpp`): hardcoded opening lines.
- `ConsoleRenderer` (`include/ConsoleRenderer.h`, `src/ConsoleRenderer.cpp`): console output.
- `src/main.cpp`: human vs AI loop and commands.

### Board Representation
- Square indexing is 0..63 where a1=0, b1=1, ..., h1=7, a2=8, ..., h8=63.
- Pieces are stored as characters:
  - White: `P N B R Q K`
  - Black: `p n b r q k`
  - Empty: `.` internally; rendering uses Unicode symbols.
- Stored state:
  - Side to move (`'w'`/`'b'`)
  - Castling rights string (e.g., `KQkq` or `-`)
  - En passant target square (`-1` if none)
  - Zobrist hash (`uint64_t`)
- FEN support:
  - `Board::LoadFen` parses piece placement, side to move, castling rights, and en passant.

### Move Representation
- Moves use UCI format (`e2e4`, `e7e8q`).
- Promotion is encoded as a trailing `q/r/b/n`.
- `Move::ParseUci` validates and produces a move with optional promotion.
- `Move::ToUci` formats back to UCI.

### Move Generation & Legality
- Pseudo-legal moves are generated for all pieces:
  - Pawns (single/double push, captures, promotions, en passant)
  - Knights, bishops, rooks, queens, kings
- Legal moves are filtered by applying a move and verifying king safety.
- Special move handling:
  - Castling checks: empty path, not in check, and no attacked transit squares.
  - En passant captures remove the pawn behind the target square.
  - Promotions replace the pawn with the promoted piece.
- Make/undo strategy:
  - `ApplyMove` and `UndoMoveApply` update pieces, en passant, castling rights, and hash.

### Search / AI
- Negamax with alpha-beta pruning.
- Iterative deepening with a fixed 300ms budget per AI move.
- Quiescence search at depth 0 (capture moves only).
- Move ordering:
  - Promotions first, then MVV-LVA captures, then quiet moves.
  - TT best move is searched first when available.
- Transposition table:
  - Zobrist hashing with bounds `EXACT`, `LOWER`, `UPPER`.
  - Integrated into main search and quiescence.
- Evaluation:
  - Material + piece-square tables (PST) + development bonus.
  - Mate/stalemate scoring uses ply to prefer faster mates.

### Validation
- Perft tests are included with known reference counts:
  - Start position (no castling rights): depths 1–3.
  - Additional micro-positions for promotions, en passant, and castling.

### Known Limitations / Future Work
- No UCI/XBoard protocol support.
- No GUI; console only.
- No endgame tablebases.
- No repetition/50-move draw detection.
- No opening learning or external book files.
