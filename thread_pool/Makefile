SRCS      = $(wildcard *.c)
OBJS     := $(SRCS:.c=.o)
BUILDDIR ?= build
BINDIR   ?= bin

OBJS     := $(addprefix $(BUILDDIR)/, $(OBJS))

CFLAGS = -Werror

debug: CFLAGS += -g
debug: CFLAGS += -DDEBUG
debug: program

release: CFLAGS += -O3
release: program

LOG_WORKS ?= 0

ifneq ($(LOG_WORKS), 0)
	CFLAGS += -DLOG_WORKS
endif

CC = gcc $(CFLAGS)

program: $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) -o $(BINDIR)/$@ $^

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $^ -o $@

clean:
	rm -rf $(BUILDDIR)
