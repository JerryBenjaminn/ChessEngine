#include "ConsoleRenderer.h"

#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
bool IsWhitePiece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

#ifdef _WIN32
wchar_t PieceToWideSymbol(char piece) {
    switch (piece) {
        case 'P':
            return L'♙';
        case 'R':
            return L'♖';
        case 'N':
            return L'♘';
        case 'B':
            return L'♗';
        case 'Q':
            return L'♕';
        case 'K':
            return L'♔';
        case 'p':
            return L'♟';
        case 'r':
            return L'♜';
        case 'n':
            return L'♞';
        case 'b':
            return L'♝';
        case 'q':
            return L'♛';
        case 'k':
            return L'♚';
        default:
            return L' ';
    }
}

void WriteWide(HANDLE handle, const wchar_t* text, DWORD length) {
    DWORD written = 0;
    WriteConsoleW(handle, text, length, &written, nullptr);
}

void WriteWideString(HANDLE handle, const std::wstring& text) {
    WriteWide(handle, text.c_str(), static_cast<DWORD>(text.size()));
}
#endif
}  // namespace

void RenderBoard(const Board& board, bool useColor) {
#ifdef _WIN32
    if (!useColor) {
        std::cout << board.ToAscii() << '\n';
        return;
    }

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    WORD original_attributes = 0;
    if (GetConsoleScreenBufferInfo(handle, &info)) {
        original_attributes = info.wAttributes;
    }

    const WORD light_bg = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
    const WORD dark_bg = BACKGROUND_BLUE;
    for (int rank = 7; rank >= 0; --rank) {
        SetConsoleTextAttribute(handle, original_attributes);
        std::wstring rank_label;
        rank_label.push_back(static_cast<wchar_t>(L'0' + (rank + 1)));
        rank_label.push_back(L' ');
        WriteWideString(handle, rank_label);

        for (int file = 0; file < 8; ++file) {
            int index = rank * 8 + file;
            char piece = board.PieceAt(index);
            bool is_light = ((rank + file) % 2) == 0;
            WORD bg = is_light ? light_bg : dark_bg;
            SetConsoleTextAttribute(handle, bg);

            wchar_t buffer[2];
            if (piece != '.') {
                buffer[0] = PieceToWideSymbol(piece);
                buffer[1] = L' ';
            } else {
                buffer[0] = L' ';
                buffer[1] = L' ';
            }
            WriteWide(handle, buffer, 2);
        }
        if (rank > 0) {
            SetConsoleTextAttribute(handle, original_attributes);
            WriteWide(handle, L"\n", 1);
        }
    }
    SetConsoleTextAttribute(handle, original_attributes);
    WriteWideString(handle, L"\n  a b c d e f g h\n");
#else
    (void)useColor;
    std::cout << board.ToAscii() << '\n';
#endif
}
