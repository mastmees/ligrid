# Directories.
vpath %.cpp .

# Tools.
CC= g++
LD= g++
AR= ar
RM= rm -f

# Flags.
CFLAGS= -Os -g3 -Wall -Wno-strict-aliasing
LDFLAGS= -L/usr/lib
INCLUDEDIRS=

all: capture mixerinfo

capture: capture.o

mixerinfo: mixerinfo.o

%.o : %.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c $< -o $@

clean: 
	$(RM) *.o
	$(RM) *~
	$(RM) core
	$(RM) *.pk
			
bare: clean
	$(RM) capture mixerinfo
