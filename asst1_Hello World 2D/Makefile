BASE = asst1

all: $(BASE)

OS := $(shell uname -s)

ifeq ($(OS), Linux) # Science Center Linux Boxes
  CPPFLAGS = -I/home/l/i/lib175/usr/glew/include
  LDFLAGS += -L/home/l/i/lib175/usr/glew/lib -L/usr/X11R6/lib
  LIBS += -lGL -lGLU -lglut
endif

ifeq ($(OS), Darwin) # Assume OS X
  CPPFLAGS += -D__MAC__
  # added for Mavericks: 09/08/2014
  CPPFLAGS += -I/usr/include
  LDFLAGS += -framework GLUT -framework OpenGL
    # suppress the warnings about deprecated glu/glut functions on Mavericks
  CXXFLAGS += -Wno-deprecated
endif

ifdef OPT 
  #turn on optimization
  CXXFLAGS += -O2
else 
  #turn on debugging
  CXXFLAGS += -g
endif

CXX = g++ 

OBJ = $(BASE).o ppm.o glsupport.o

$(BASE): $(OBJ)
	@# If you are using an older version of OSX, and this doesn't work, 
	@# you might need hard code the GLEW path, use /usr/local/Cellar/glew/1.13.0/lib/libGLEW.a or /usr/local/lib/libGLEW.a
	@#$(LINK.cpp) -o $@ $^ $(LIBS) /usr/local/lib/libGLEW.a
	$(LINK.cpp) -o $@ $^ $(LIBS) -lGLEW 

clean:
	rm -f $(OBJ) $(BASE)
