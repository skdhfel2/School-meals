CC = gcc
CFLAGS = -I../include -I"C:/msys64/mingw64/include"
LDFLAGS = -L../lib -L"C:/msys64/mingw64/x86_64-w64-mingw32/lib"


ifeq ($(OS),Windows_NT)
    LIBS = -lws2_32 -lcurl -ljson-c
else
    LIBS = -pthread -lcurl -ljson-c
endif

SRCS = server.c db_handler.c network.c server_handler.c neis_api.c
OBJS = $(SRCS:.c=.o)
SQLITE_SRC = ../lib/sqlite3.c
SQLITE_OBJ = ../lib/sqlite3.o
TARGET = server

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) $(SQLITE_OBJ)
	@echo " Linking $(TARGET)..."
	$(CC) $(OBJS) $(SQLITE_OBJ) -o $(TARGET) $(LDFLAGS) -Wl,--exclude-libs,libmingw32.a $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SQLITE_OBJ): $(SQLITE_SRC)
	$(CC) $(CFLAGS) -c $(SQLITE_SRC) -o $(SQLITE_OBJ)

clean:
	del /Q *.o $(TARGET) 2>nul || rm -f *.o $(TARGET) $(SQLITE_OBJ)
