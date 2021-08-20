NAME = libirc.a

# Compiler and linker
CC = clang
LD = clang
AR = ar

# Paths
include srcs.mk
INCDIR = include
LIBDIR = lib

OBJDIR = obj
BINDIR = .

# Library dependencies
LIBS = $(addprefix $(LIBDIR)/, libhashmap/libhashmap.a libbase64/libbase64.a)

LOCALLIBDIRS = $(dir $(LIBS))
SYSLIBINCS =
SYSLIBDIRS =
LIBINCS = $(addsuffix $(INCDIR), $(LOCALLIBDIRS)) $(SYSLIBINCS)
LIBDIRS = $(LOCALLIBDIRS) $(SYSLIBDIRS)
LIBARS = $(notdir $(LIBS))

# Include directories
SYSINCS = /usr/include /usr/local/include

ifeq ($(shell find -L $(SYSINCS) -maxdepth 1 -type d -name openssl -print -quit 2>/dev/null), )
        BREW = $(shell dirname $(dir $(shell which brew)))

        ifneq ($(BREW), )
			USRINC = $(shell find -L $(BREW)/include -maxdepth 1 -type d -name openssl -print -quit)
			USRLIB = $(shell find -L $(BREW)/lib -maxdepth 1 -type f -iname "libssl*" -print -quit)
			ifeq ($(USRINC), )
				USRINC = $(shell find -L $(BREW)/opt/openssl/include -maxdepth 1 -type d -name "openssl" -print -quit)
				USRLIB = $(shell find -L $(BREW)/opt/openssl/lib -maxdepth 1 -type f -iname "libssl*")
	    	endif
		endif

        ifeq ($(USRINC), )
$(error Could not find OpenSSL headers!)
        endif

        ifeq ($(USRLIB), )
$(error Could not find OpenSSL library!)
        endif

        SYSLIBINCS += $(dir $(USRINC))
        SYSLIBDIRS += $(dir $(USRLIB))
		LIBARS += libcrypto.a libssl.a
endif

INCS = $(LIBINCS) $(INCDIR)

# Objects
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:.o=.d)

# Tests
BOTSRCDIR = test
BOTSRCS = $(addprefix $(BOTSRCDIR)/, ch0.c ch1.c ch2.c ch3.c)
BOTOBJS = $(BOTSRCS:$(BOTSRCDIR)/%.c=$(OBJDIR)/%.o)
BOTDEPS = $(BOTOBJS:.o=.d)

# Flags
CFLAGS = -Wall -Wextra -Werror $(INCS:%=-I%)
DFLAGS = -MT $@ -MMD -MP -MF $(OBJDIR)/$*.d
LDFLAGS = $(LIBDIRS:%=-L%)
ARFLAGS = -rcTs
LDLIBS = $(LIBARS:lib%.a=-l%)

# Compiling commands
COMPILE.c = $(CC) $(DFLAGS) $(CFLAGS) -c
COMPILE.o = $(LD) $(LDFLAGS)
ARCHIVE.o = $(AR) $(ARFLAGS)

all: $(BINDIR)/$(NAME)

# Directories
$(OBJDIR) $(BINDIR):
	@echo "MK $@"
	mkdir -p "$@"

# Libraries
$(LIBS): %.a: FORCE
	make -C $(dir $@) NAME=$(@F)

# Objects
$(BOTOBJS): $(OBJDIR)/%.o: $(BOTSRCDIR)/%.c $(OBJDIR)/%.d | $(OBJDIR)
	@mkdir -p '$(@D)'
	@echo "CC $<"
	$(COMPILE.c) $< -o $@

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)/%.d | $(OBJDIR)
	@mkdir -p '$(@D)'
	@echo "CC $<"
	$(COMPILE.c) $< -o $@

# Dependencies
$(DEPS): $(OBJDIR)/%.d:
include $(wildcard $(DEPS))

$(BOTDEPS): $(OBJDIR)/%.d:
include $(wildcard $(BOTDEPS))

# Library
$(BINDIR)/$(NAME): $(OBJS) $(LIBS) | $(BINDIR)
	@echo "AR $@"

	$(ARCHIVE.o) $@ $^

# Binaries

ch0: BOTSRCS = $(addprefix $(BOTSRCDIR)/, ch0.c)
ch0: $(BINDIR)/$(NAME) $(addprefix $(OBJDIR)/, ch0.o)
	@echo "LD $^"
	$(COMPILE.o) -L$(BINDIR) $(BOTOBJS) -o $@ -lm -lcrypto -lssl $(LDLIBS) -l$(<:lib%.a=%)

ch1: BOTSRCS = $(addprefix $(BOTSRCDIR)/, ch1.c)
ch1: $(BINDIR)/$(NAME) $(addprefix $(OBJDIR)/, ch1.o)
	@echo "LD $^"
	$(COMPILE.o) -L$(BINDIR) $(BOTOBJS) -o $@ -lm -lcrypto -lssl $(LDLIBS) -l$(<:lib%.a=%)

ch2: BOTSRCS = $(addprefix $(BOTSRCDIR)/, ch2.c)
ch2: $(BINDIR)/$(NAME) $(addprefix $(OBJDIR)/, ch2.o)
	@echo "LD $^"
	$(COMPILE.o) -L$(BINDIR) $(BOTOBJS) -o $@ -lm -lcrypto -lssl $(LDLIBS) -l$(<:lib%.a=%)

ch3: BOTSRCS = $(addprefix $(BOTSRCDIR)/, ch3.c)
ch3: $(BINDIR)/$(NAME) $(addprefix $(OBJDIR)/, ch3.o)
	@echo "LD $^"
	$(COMPILE.o) -L$(BINDIR) $(BOTOBJS) -o $@ -lm -lcrypto -lssl -lz $(LDLIBS) -l$(<:lib%.a=%)

debug: CFLAGS += -DDEBUG -g3 -fsanitize=address
debug: LDFLAGS += -g3 -fsanitize=address
debug: re

clean:
	$(foreach dir, $(LOCALLIBDIRS),\
		echo "MK $(addprefix -C , $(dir)) $@" && make -C $(dir) $@ && ):
	
	rm -r "$(OBJDIR)" 2>/dev/null && @echo "RM $(OBJDIR)" || :;

fclean: clean
	$(foreach dir, $(LOCALLIBDIRS),\
		echo "MK $(addprefix -C, $(dir)) $@" && make -C $(dir) $@ && ):

	$(foreach bin, $(BINDIR)/$(NAME) ch0 ch1 ch2 ch3,\
		rm "$(bin)" 2>/dev/null && echo "RM $(bin)"; ):

	@rmdir "$(BINDIR)" 2>/dev/null && echo "RM $(BINDIR)" || :

compile_flags.txt:
	printf '%s\n' $(CFLAGS) > $@

re: fclean all

FORCE: ;

.PHONY: all clean fclean re

# Assign a value to VERBOSE to enable verbose output
$(VERBOSE).SILENT:
