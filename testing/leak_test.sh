#!/bin/bash
make && clear && valgrind --track-origins=yes --verbose --show-leak-kinds=all --leak-check=full ./overseer.out 1234
