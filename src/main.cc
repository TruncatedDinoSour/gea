#include <curses.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <cstdio>

#include "client.hpp"
#include "curses_keys.h"

bool is_running = true;
const static int line_offset_bottom = 2;
const static int bar_line_offset_column = 2;

const char *mode_to_str(ClientMode client_mode) {
    const char *mode;

    switch (client_mode) {
    case CLIENT_ADD:
        mode = "add";
        break;
    case CLIENT_NORMAL:
        mode = "normal";
        break;
    default:
        mode = "unknown";
        break;
    }

    return mode;
}

void gea_bar(Client client) {
    char *formatted_bar = (char *)NULL;
    if (asprintf(&formatted_bar, "Mode: %s | File: %s",
                 mode_to_str(client.mode), client.filename) < 0) {
        formatted_bar = (char *)"FORMAT ERROR";
    }

    mvaddstr(LINES - line_offset_bottom, bar_line_offset_column, formatted_bar);
    clrtobot();

    move(client.y, client.x);
}

void gea_curses_init(void) {
    initscr();
    cbreak();
    noecho();
    raw();

    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    clear();
}

unsigned char parse_key(int key, Client &client) {
    unsigned char character = '\0';

    switch (client.mode) {
    case CLIENT_NORMAL:
        switch (key) {
        case 'a':
            client.mode = CLIENT_ADD;
            break;

        case 'q':
            is_running = false;
            break;

        case 'p':
            if (client.x + 1 < stdscr->_maxx)
                ++client.x;
            break;
        case 'j':
            if (client.x - 1 >= 0)
                --client.x;
            break;
        case 'o':
            if (client.y - 1 >= 0)
                --client.y;
            break;
        case 'i':
            if (client.y + line_offset_bottom < stdscr->_maxy)
                ++client.y;
            break;
        }

        break;
    case CLIENT_ADD:
        switch (key) {
        case K_CURSES_ALT_ESC:
            client.mode = CLIENT_NORMAL;
            --client.x;
            break;

        case ERR:
            // No input (pass)
            break;

        case KEY_BACKSPACE:
        case 127:
        case K_CURSES_BACKSPACE:
            --client.x;
            mvdelch(client.y, client.x);
            break;

        case KEY_ENTER:
        case K_CURSES_ENTER:
            ++client.y;
            client.x = 0;
            character = '\n';
            break;

        default:
            ++client.x;
            character = key;
            addch(key);
            break;
        }

        break;
    }

    return character;
}

int main(int argc, char *argv[]) {
    gea_curses_init();
    Client c = Client();

    if (argc > 1 && access(argv[1], F_OK) == 0) {
        std::string line;
        std::ifstream input_file(argv[1]);

        while (getline(input_file, line))
            addstr((line + '\n').c_str());

        input_file.close();
        c.filename = argv[1];
    }

    std::vector<unsigned char> final_chars;

    while (is_running) {
        gea_bar(c);
        refresh();

        unsigned char fchar = parse_key(getch(), c);

        if (fchar != '\0')
            final_chars.push_back(fchar);
    }

    endwin();

    if (final_chars.size() >= 1) {
        char yes_no = 'y';

        if (argc <= 1) {
            std::cout << "(unsaved changes, do you want to save them? (Y/n)) ";
            yes_no = getchar();

            if (!yes_no)
                yes_no = 'y';
        }

        if ((char)std::tolower(yes_no) == 'y') {
            std::string output_filename;

            if (argc > 1)
                output_filename = argv[1];
            else {
                std::cout << "(output filename) ";
                std::cin >> output_filename;
            }

            if (output_filename.size() < 1 || output_filename.size() > 255)
                output_filename = "gea_output";

            std::ofstream output(output_filename);

            if (!output) {
                std::cerr << "Failed to open file" << '\n';
                return EXIT_FAILURE;
            }

            for (char character : final_chars)
                output << character;

            output.close();
        }
    }

    return 0;
}
