# you have to build Juce one level up, so that Juce static libs are
# available at ../juce/bin (or symlink)
#
vpath %.cpp .
vpath %.cpp utils

# Tools.
CXX= g++
CC= g++
LD= g++
AR= ar
RM= rm -f

# Flags.
ARCHITECTURES = -arch i386  -mmacosx-version-min=10.5
CFLAGS= -Os -g3 -Wall -Wno-strict-aliasing $(ARCHITECTURES) 
LDFLAGS= -L/usr/lib -lpthread -s -L../juce/bin -ljucedebug -F/System/Library/Frameworks \
	-framework CoreAudio -framework AppKit -framework OpenGL -framework IOKit \
	-framework AGL -framework Carbon $(ARCHITECTURES)
INCLUDEDIRS=

OBJECTS = main.o \
	MainComponent.o \
	powerscopeComponent.o \
	directionscopeComponent.o \
	radarComponent.o \
	scopeComponent.o

all: LiGridUI
	sh create_app_mac.sh LiGridUI LiGridUI
			
LiGridUI: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o LiGridUI

%.o : %.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c $< -o $@

clean:
	$(RM) *.o
	$(RM) *~
	$(RM) core

bare: clean
	$(RM) LiGridUI
	$(RM) -r LigridUI.app

