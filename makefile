gndx: main.c
	gcc main.c -I$(groan) -L$(groan) -D_POSIX_C_SOURCE=200809L -o gndx -lgroan -lm -std=c99 -pedantic -Wall -Wextra -O3 -march=native

install: gndx
	cp gndx ${HOME}/.local/bin
