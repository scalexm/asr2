CC=clang
LDFLAGS=
CFLAGS=-pthread -Wall -std=c11
LD=clang
BIN=serv

SRCDIR=.
TMPDIR=obj
OBJDIR=$(TMPDIR)

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) -o $@ $^ $(CFLAGS) $(LIB)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(TMPDIR)/%.d: $(SRCDIR)/%.c
	$(CC) -MM -MT $(OBJDIR)/$*.o -o $@ $<

-include $(DEPS)

.PHONY: clean distclean

clean:
	rm -f $(OBJS) $(DEPS)

distclean: clean
	rm -rf $(BIN)

nodir:
	CFLAGS += -DNO_DIRECTORY
