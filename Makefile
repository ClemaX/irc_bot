NAME = ch0

# Compiler and linker
CC = clang
LD = clang

# Paths
include srcs.mk
INCDIR = include
LIBDIR = lib

OBJDIR = obj
BINDIR = .

# Library dependencies
LIBS = $(addprefix $(LIBDIR)/, libhashmap/libhashmap.a)

LIBDIRS = $(dir $(LIBS))
LIBINCS = $(addsuffix $(INCDIR), $(LIBDIRS))
LIBARS = $(notdir $(LIBS))

# Sources
INCS = $(LIBINCS) $(INCDIR)

OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:.o=.d)

# Flags
CFLAGS = -Wall -Wextra -Werror $(INCS:%=-I%)
DFLAGS = -MT $@ -MMD -MP -MF $(OBJDIR)/$*.d
LDFLAGS = $(LIBDIRS:%=-L%)
LDLIBS = $(LIBARS:lib%.a=-l%) -lcrypto -lssl

# Compiling commands
COMPILE.c = $(CC) $(DFLAGS) $(CFLAGS) -c -g3 -fsanitize=address
COMPILE.o = $(LD) $(LDFLAGS) -g3 -fsanitize=address

all: $(BINDIR)/$(NAME)

# Directories
$(OBJDIR) $(BINDIR):
	@echo "MK $@"
	mkdir -p "$@"

# Libraries
$(LIBS): %.a: FORCE
	make -C $(dir $@) NAME=$(@F)

# Objects
$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)/%.d | $(OBJDIR)
	@mkdir -p '$(@D)'
	@echo "CC $<"
	$(COMPILE.c) $< -o $@

# Dependencies
$(DEPS): $(OBJDIR)/%.d:
-include $(wildcard $(DEPS))

# Binaries
$(BINDIR)/$(NAME): $(OBJS) $(LIBS) | $(BINDIR)
	@echo "LD $@"
	$(COMPILE.o) $^ -o $@ $(LDLIBS)

debug: CFLAGS += -DDEBUG
debug: re

# TODO: Replace make re with distribution folders

clean:
	$(foreach dir, $(LIBDIRS),\
		@echo "MK $(addprefix -C, $(LIBDIRS)) $@" && make -C $(dir) $@ && ):
	@echo "RM $(OBJDIR)"
	rm -rf "$(OBJDIR)"

fclean: clean
	$(foreach dir, $(LIBDIRS),\
		@echo "MK $(addprefix -C, $(LIBDIRS)) $@" && make -C $(dir) $@ && ):
	@echo "RM $(BINDIR)/$(NAME)"
	rm -f "$(BINDIR)/$(NAME)"
	@rmdir "$(BINDIR)" 2>/dev/null && echo "RM $(BINDIR)" || :

re: fclean all

FORCE: ;

.PHONY: all clean fclean re

# Assign a value to VERBOSE to enable verbose output
$(VERBOSE).SILENT:
