###############################################################################
# CS/CNS 171 Fall 2015
#
# This is a template Makefile for OpenGL programs. Edit it however you find
# convenient.
# 
# The current version of this file should compile OpenGL programs just fine on
# Debian-based Linux operating systems.
#
# If you run Mac OS or other distributions of Linux, then you may have to
# fool around with the 'INCLUDE' and 'LIBDIR' lines below before the Makefile
# can compile the OpenGL parts successfully.
###############################################################################
CC = g++
FLAGS = -w -std=c++17 -g -o

INCLUDE = -I/usr/X11R6/include -I/usr/include/GL -I/usr/include -I ./
LIBDIR = -L/usr/X11R6/lib -L/usr/local/lib
LIBS = -lGLEW -lGL -lGLU -lglut -lm

opengl: opengl.cpp
	$(CC) $(FLAGS) opengl $(INCLUDE) $(LIBDIR) opengl.cpp $(LIBS)

opengl_matrix: opengl_matrix.cpp
	$(CC) $(FLAGS) opengl_matrix $(INCLUDE) $(LIBDIR) opengl_matrix.cpp $(LIBS)

clean:
	rm -f *.o $(EXENAME)

.PHONY: all clean
