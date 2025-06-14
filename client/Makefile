CC = gcc
CFLAGS = -I. -I../include -Wall
LDFLAGS = -lws2_32

SRCS = client.c network.c menu.c handlers.c menu_general.c menu_parent.c input_util.c
OBJS = $(SRCS:.c=.o)
TARGET = client.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# OS별 명령어 설정
ifeq ($(OS),Windows_NT)
    RM = del /Q /F
    EXT = .exe
else
    RM = rm -f
    EXT =
endif

clean:
	-$(RM) *.o *$(EXT)
