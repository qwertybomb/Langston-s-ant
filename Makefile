CC = clang

#remove -lmingw32 -lSDL2Main if not on windows
LIBS = -lmingw32 -lSDL2Main -lSDL2

INCLUDE = -I/usr/include/SDL2

#remove -mwindows if not on windows
FLAGS =  -s -mwindows -Ofast -march=native -std=gnu11 -Wall -Wextra -Werror

EXE = main

all: ant.c
	$(CC) $(INCLUDE) $(FLAGS) -o $(EXE) ant.c $(LIBS)

#change del to rm and remove the .exe if not on windows
clean:
	del $(EXE).exe