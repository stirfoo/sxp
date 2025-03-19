/*
  main.cpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#include "sxp.hpp"

int main(int argc, char** argv) {
    rt::init();
    if (argc > 1)
        rt::loadFile(argv[1]);
    else
        rt::loadFile("sxpsrc/repl.sxp");
    return 0;
}
