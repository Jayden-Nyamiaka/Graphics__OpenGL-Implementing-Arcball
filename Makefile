CC = g++
FLAGS = -w -std=c++17 -g -o

INCLUDE = -I/usr/X11R6/include -I/usr/include/GL -I/usr/include -I ./
LIBDIR = -L/usr/X11R6/lib -L/usr/local/lib
LIBS = -lGLEW -lGL -lGLU -lglut -lm

opengl: opengl.cpp
	$(CC) $(FLAGS) opengl $(INCLUDE) $(LIBDIR) opengl.cpp $(LIBS)

opengl_matrix: opengl_matrix.cpp
	$(CC) $(FLAGS) opengl_matrix $(INCLUDE) $(LIBDIR) opengl_matrix.cpp $(LIBS)

demo: opengl_demo.cpp
	$(CC) $(FLAGS) demo $(INCLUDE) $(LIBDIR) opengl_demo.cpp $(LIBS)

clean:
	rm -f *.o opengl opengl_matrix demo

all: clean opengl

.PHONY: all clean
