
NAME=fillwan
DEPS= 

CC=g++
LD=ld

CPPFLAGS= -Wall -std=c++20 -O1
LINKFLAGS= -s

PREFIX=/usr/bin
SOURCEDIR=src
BUILDDIR=obj
TARGETDIR=bin

SOURCES= $(SOURCEDIR)/processing.cpp $(SOURCEDIR)/main.cpp
HEADERS= $(SOURCEDIR)/definitions.hpp $(SOURCEDIR)/words_de.hpp $(SOURCEDIR)/words_en.hpp 
OBJECTS= $(BUILDDIR)/processing.o $(BUILDDIR)/main.o


# Building
# ###################################################################

all: build

build: compile link

link: compile
	$(CC) -o $(TARGETDIR)/$(NAME) $(LINKFLAGS) $(OBJECTS)

compile: $(OBJECTS)

# currently rebuilds everything if a header has changed
$(OBJECTS): $(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp $(HEADERS) | makedir
	$(CC) $(CPPFLAGS) -c $< -o $@

makedir:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGETDIR)


# Misc
# ###################################################################

clean: 
	rm -f $(OBJECTS)
	rm -f $(TARGETDIR)/$(NAME)

install: build
	install $(TARGETDIR)/$(NAME) $(PREFIX)/$(NAME)

uninstall:
	rm $(PREFIX)/$(NAME)

compress:
	strip -s -R .comment $(TARGETDIR)/$(NAME)
	upx --lzma -qq $(TARGETDIR)/$(NAME)

decompress:
	upx -d -qq $(TARGETDIR)/$(NAME)

