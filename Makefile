
NAME=fillwan
DEPS= 

CC=g++
LD=ld

CPPFLAGS= -Wall -std=c++20 -O
LINKFLAGS=

PREFIX=/usr/bin
SOURCEDIR=src
BUILDDIR=obj
TARGETDIR=bin

SOURCES= $(SOURCEDIR)/processing.cpp $(SOURCEDIR)/main.cpp
OBJECTS= $(BUILDDIR)/processing.o $(BUILDDIR)/main.o


# Building
# ###################################################################

all: build

build: compile link

link: compile
	$(CC) -o $(TARGETDIR)/$(NAME) $(LINKFLAGS) $(OBJECTS)

compile: $(OBJECTS)

$(OBJECTS): $(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp | makedir
	$(CC) $(CPPFLAGS) -c $< -o $@

makedir:
	mkdir -p $(BUILDDIR)
	mkdir -p $(TARGETDIR)


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
	upx --lzma $(TARGETDIR)/$(NAME)

decompress:
	upx -d $(TARGETDIR)/$(NAME)

