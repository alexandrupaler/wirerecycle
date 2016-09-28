OBJDIR = obj
BINDIR = bin
CC = g++
CS = gcc
INCLUDES = headers
CFLAGS = -I$(INCLUDES) -lm -g #-m32 #-O3
CXXFLAGS = $(CFLAGS) 

FILES = gatenumbers.cpp cnotcounter.cpp circuitmatrix.cpp cuccaro.cpp trim.cpp causalgraph.cpp circconvert.cpp recyclegate.cpp recycle.cpp fileformats/test.cpp
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(FILES))

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

all:: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BINDIR)/recycle

clean::
	rm -f $(OBJS)
	rm -f $(BINDIR)/recycle
	
$(OBJS): | $(OBJDIR) $(BINDIR)

$(OBJDIR):
	@mkdir -p $@
	
$(BINDIR):
	@mkdir -p $@