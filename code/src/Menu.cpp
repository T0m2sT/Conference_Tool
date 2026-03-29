#include <iostream>
#include <limits>
#include <termios.h>
#include <unistd.h>
#include "Menu.h"

// ANSI escape code constants
const std::string Menu::CLEAR_SCREEN    = "\033[2J\033[H";
const std::string Menu::HIGHLIGHT_ON    = "\033[7m";
const std::string Menu::HIGHLIGHT_OFF   = "\033[0m";
const std::string Menu::CURSOR_HIDE     = "\033[?25l";
const std::string Menu::CURSOR_SHOW     = "\033[?25h";
const std::string Menu::CURSOR_SAVE     = "\033[s";
const std::string Menu::CURSOR_RESTORE  = "\033[u";

const int Menu::BOX_INNER = 66;
const std::string Menu::BOX_TOP    = "╔════════════════════════════════════════════════════════════════════╗";
const std::string Menu::BOX_MID    = "╠════════════════════════════════════════════════════════════════════╣";
const std::string Menu::BOX_BOTTOM = "╚════════════════════════════════════════════════════════════════════╝";

std::string Menu::cursorToRow(int row) {
    return "\033[" + std::to_string(row) + ";1H";
}

std::string Menu::boxLine(const std::string &text) {
    std::string padded = text;
    if ((int)padded.size() > BOX_INNER) padded = padded.substr(0, BOX_INNER);
    while ((int)padded.size() < BOX_INNER) padded += " ";
    return "║ " + padded + " ║";
}

std::string Menu::boxLineArrow(const std::string &text) {
    std::string padded = text;
    while ((int)padded.size() < BOX_INNER + 4) padded += " ";
    return "║ " + padded + " ║";
}

std::string Menu::menuLine(const std::string &text, bool highlighted) {
    std::string padded = text;
    while ((int)padded.size() < BOX_INNER - 3) padded += " ";
    if (highlighted)
        return "║  " + HIGHLIGHT_ON + " " + padded + HIGHLIGHT_OFF + "  ║";
    return "║   " + padded + "  ║";
}

void Menu::displayInBox(const std::string &subtitle, const std::vector<std::string> &lines) {
    std::cout << CLEAR_SCREEN;
    std::cout << "\n";
    std::cout << BOX_TOP << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("                Conference Review Assignment Tool") << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << BOX_MID << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("  " + subtitle) << "\n";
    std::cout << boxLine("") << "\n";
    for (const auto &line : lines) {
        std::cout << boxLine("  " + line) << "\n";
    }
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("  Press Enter to go back...") << "\n";
    std::cout << BOX_BOTTOM << std::endl;

    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    tcflush(STDIN_FILENO, TCIFLUSH);

    while (getchar() != '\n') {}
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

std::string Menu::promptInBox(const std::string &subtitle, const std::string &prompt) {
    std::cout << CLEAR_SCREEN;
    std::cout << "\n";
    std::cout << BOX_TOP << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("                Conference Review Assignment Tool") << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << BOX_MID << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("  " + subtitle) << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("  " + prompt) << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << BOX_BOTTOM << "\n";
    std::cout << std::flush;
    int promptCol = 4 + (int)prompt.size() + 1;
    std::cout << "\033[3A\033[" << promptCol << "G";
    std::string input;
    std::cin >> input;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return input;
}

int Menu::arrowMenu(const std::vector<std::string> &options) {
    int selected = 0;
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cout << CLEAR_SCREEN;
    std::cout << "\n";
    std::cout << BOX_TOP << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << boxLine("                Conference Review Assignment Tool") << "\n";
    std::cout << boxLine("") << "\n";
    std::cout << BOX_MID << "\n";
    std::cout << boxLine("") << "\n";
    int firstOptionRow = 8;
    for (int i = 0; i < (int)options.size(); i++) {
        std::cout << menuLine(options[i], i == selected) << "\n";
    }
    std::cout << boxLine("") << "\n";
    std::cout << boxLineArrow("  \u2191/\u2193 arrows, Enter to select") << "\n";
    std::cout << BOX_BOTTOM << "\n";
    std::cout << CURSOR_HIDE << std::flush;

    while (true) {
        char c = getchar();
        if (c == '\n') break;
        if (c == '\033') {
            getchar();
            char arrow = getchar();
            int oldSelected = selected;
            if (arrow == 'A' && selected > 0) selected--;
            else if (arrow == 'A') selected = (int)options.size()-1;
            if (arrow == 'B' && selected < (int)options.size() - 1) selected++;
            else if (arrow == 'B') selected = 0;

            if (oldSelected != selected) {
                std::cout << CURSOR_SAVE;
                std::cout << cursorToRow(firstOptionRow + oldSelected);
                std::cout << menuLine(options[oldSelected], false);
                std::cout << cursorToRow(firstOptionRow + selected);
                std::cout << menuLine(options[selected], true);
                std::cout << CURSOR_RESTORE << std::flush;
            }
        }
    }

    std::cout << CURSOR_SHOW;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return selected;
}
