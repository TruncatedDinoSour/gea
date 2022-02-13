#!/usr/bin/env bash

set -xe

main() {
    cd src
    ${CXX:-clang++} $(pkg-config --cflags ncurses) -flto -Ofast -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -pedantic -ffunction-sections -fdata-sections -march=native -pipe -o ../gea.elf main.cc -s $(pkg-config --libs ncurses)
    strip --strip-all --remove-section=.note --remove-section=.gnu.version --remove-section=.comment --strip-debug --strip-unneeded ../gea.elf -o ../gea.elf
    cd ..

    if [ "$1" = "-r" ]; then
        ./gea.elf "${@:2}"
    fi
}

main "$@"
