CC=gcc
# Get ncurses flags using pkg-config
# NCURSES_CFLAGS will contain -I/path/to/ncurses/include
# NCURSES_LIBS will contain -L/path/to/ncurses/lib -lncurses
NCURSES_CFLAGS := $(shell pkg-config --cflags ncurses)
NCURSES_LIBS := $(shell pkg-config --libs ncurses)

# CFLAGS for compilation: general warnings + ncurses include paths
CFLAGS=-Wall -Wextra $(NCURSES_CFLAGS)
# LDFLAGS for linking: ncurses library paths and names
LDFLAGS=$(NCURSES_LIBS)

GAME_OBJS=obj/game/main.o obj/game/sudoku.o obj/game/interface.o obj/game/misc.o
GAME_BIN=sudoku.out
SOLVER_OBJS=obj/solver/main.o obj/solver/sudoku.o obj/solver/misc.o
SOLVER_BIN=solver.out
#-------------------------------------------------------------------------------
.PHONY: all run clean
all: $(GAME_BIN) $(SOLVER_BIN)
run: $(GAME_BIN)
	./$<
clean:
	rm -f $(SOLVER_BIN) $(GAME_BIN)
	rm -f $(GAME_OBJS) $(SOLVER_OBJS)
	rm -rf obj
#-------------------------------------------------------------------------------
# Game
$(GAME_BIN): $(GAME_OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $(GAME_OBJS) $(LDFLAGS)
$(GAME_OBJS): obj/game/%.o : src/game/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
#-------------------------------------------------------------------------------
# Solver
$(SOLVER_BIN): $(SOLVER_OBJS)
	$(CC) -o $@ $(SOLVER_OBJS)
$(SOLVER_OBJS): obj/solver/%.o : src/solver/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
