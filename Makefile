#### GTThread Library Makefile

CFLAGS  = -Wall 
LFLAGS  =
CC      = gcc
RM      = /bin/rm -rf
AR      = ar rc
RANLIB  = ranlib

LIBRARY = gtthread.a

LIB_SRC = gtthread.c gtthread_mutex.c

LIB_OBJ = $(patsubst %.c,%.o,$(LIB_SRC))

APP_SRC = Dining_Philosophers.c

APP_OBJ = $(patsubst %.c,%.o,$(APP_SRC))

BIN_NAME = Dining_Philosophers

# pattern rule for object files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

all: $(LIBRARY)
	$(CC) $(LFLAGS) -o $(BIN_NAME) $(APP_SRC) $(LIBRARY)

$(LIBRARY): $(LIB_OBJ)
	$(AR) $(LIBRARY) $(LIB_OBJ)
	$(RANLIB) $(LIBRARY)

clean:
	$(RM) $(LIBRARY) $(LIB_OBJ)

.PHONY: depend
depend:
	$(CFLAGS) -- $(LIB_SRC)  2>/dev/null
