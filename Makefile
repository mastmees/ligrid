# TODO: makefile is very simpleminded, fix to handle dependencies better
# Directories.
vpath %.cpp .
vpath %.cpp utils

# Tools.
CC= g++
LD= g++
AR= ar
RM= rm -f

# Flags.
CFLAGS= -Os -g3 -Wall -Wno-strict-aliasing
LDFLAGS= -L/usr/lib -lpthread
INCLUDEDIRS=

LIGRIDOBJECTS= ligrid.o sampler.o uiconnection.o \
 	clientconnection.o signalscanner.o connectionpool.o \
 	webconnection.o stationcache.o stereoinput.o
                                                                                                             
all: ligrid

kill:
	-killall ligrid

restart: kill
	./ligrid &
	
ligrid: $(LIGRIDOBJECTS)
	$(CXX) $(LDFLAGS) $(LIGRIDOBJECTS) -o $@
		
%.o : %.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c $< -o $@

clean: 
	$(RM) *.o
	$(RM) *~
	$(RM) core
	$(RM) *.pk
			
bare: clean
	$(RM) ligrid
