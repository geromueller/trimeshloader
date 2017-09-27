OBJ = test.o
CFLAGS=-I../include -ansi
LDFLAGS=-L../lib -ltrimeshloader

ifeq ($(MAKE),mingw32-make)
	RM = del /F
	CC := gcc
	EXT=.exe
else
	RM = rm -f
	EXT=
endif

TEST_BIN=test$(EXT)
TEST_OBJ=test.o

TL3DSINFO_BIN=tl3dsinfo$(EXT)
TL3DSINFO_OBJ=tl3dsinfo.o

TLOBJINFO_BIN=tlobjinfo$(EXT)
TLOBJINFO_OBJ=tlobjinfo.o

all: $(TEST_BIN) $(TL3DSINFO_BIN) $(TLOBJINFO_BIN)

$(TEST_BIN): $(TEST_OBJ)
	gcc -o $@ $(TEST_OBJ) $(LDFLAGS)

$(TL3DSINFO_BIN): $(TL3DSINFO_OBJ)
	        gcc -o $@ $(TL3DSINFO_OBJ) $(LDFLAGS)

$(TLOBJINFO_BIN): $(TLOBJINFO_OBJ)
	        gcc -o $@ $(TLOBJINFO_OBJ) $(LDFLAGS)

clean:
	-@$(RM) *.o
	-@$(RM) $(TEST_BIN)
	-@$(RM) $(TL3DSINFO_BIN)
	-@$(RM) $(TLOBJINFO_BIN)

