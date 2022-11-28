CXX = g++
FLAGS = -std=c++17 -I ./ -march=native -O3 -w
SOURCES = *.h *.cpp
GENERATED_PPMS = $(wildcard **/*.ppm) $(wildcard *.ppm)
GENERATED_PNGS = $(wildcard **/*-generated.png) $(wildcard *-generated.png)

EXENAME = wireframe
 
all: $(SOURCES)
	$(CXX) $(FLAGS) -o $(EXENAME) $(SOURCES)

generate_pngs:
	python3 ppm3-to-png.py

clean:
	rm -f *.o $(EXENAME) $(GENERATED_PPMS) $(GENERATED_PNGS) garbage
 
.PHONY: all generate_pngs clean

test: $(EXENAME)
	./$(EXENAME) data/scene_cube3.txt 800 800 0
	./$(EXENAME) data/scene_cube3.txt 800 800 1

run: $(EXENAME)
	./$(EXENAME) data/scene_armadillo.txt 800 800 0
	./$(EXENAME) data/scene_armadillo.txt 800 800 1
	./$(EXENAME) data/scene_bunny1.txt 800 800 0
	./$(EXENAME) data/scene_bunny1.txt 800 800 1
	./$(EXENAME) data/scene_bunny1_attenuated.txt 800 800 0
	./$(EXENAME) data/scene_bunny1_attenuated.txt 800 800 1
	./$(EXENAME) data/scene_bunny2.txt 800 800 0
	./$(EXENAME) data/scene_bunny2.txt 800 800 1
	./$(EXENAME) data/scene_cube1.txt 800 800 0
	./$(EXENAME) data/scene_cube1.txt 800 800 1
	./$(EXENAME) data/scene_cube2.txt 800 800 0
	./$(EXENAME) data/scene_cube2.txt 800 800 1
	./$(EXENAME) data/scene_cube3.txt 800 800 0
	./$(EXENAME) data/scene_cube3.txt 800 800 1
	./$(EXENAME) data/scene_fourCubes.txt 800 800 0
	./$(EXENAME) data/scene_fourCubes.txt 800 800 1
	./$(EXENAME) data/scene_kitten.txt 800 800 0
	./$(EXENAME) data/scene_kitten.txt 800 800 1
	./$(EXENAME) data/scene_kitten_attenuated.txt 800 800 0
	./$(EXENAME) data/scene_kitten_attenuated.txt 800 800 1
	./$(EXENAME) data/scene_sphere.txt 800 800 0
	./$(EXENAME) data/scene_sphere.txt 800 800 1

run_edge: $(EXENAME)
	./$(EXENAME) data/scene_bunny_closeup.txt 800 800
	./$(EXENAME) data/scene_bunny1.txt 700 -400
	./$(EXENAME) data/scene_bunny2.txt 400 400
	./$(EXENAME) data/scene_cube1.txt -1000 100
	./$(EXENAME) data/scene_cube2.txt 100 0
	./$(EXENAME) data/scene_face1.txt 2
	./$(EXENAME) data/scene_face2.txt 
	./$(EXENAME) data/scene_fourCubes.txt 80 34

run_error: $(EXENAME)
	./$(EXENAME) data/scene_bunny_closeup.txt 3+4 50
	./$(EXENAME) data/scene_bunny1.txt 700 400
	./$(EXENAME) data/scene_bunny2.txt -400 -400
	./$(EXENAME) data/scene_cube1.txt -1000 100 20
	./$(EXENAME) data/scene_cube2.txt 100 0
	./$(EXENAME) data/scene_face1.txt 2
	./$(EXENAME) data/scene_face2.txt 
	./$(EXENAME) data/scene_fourCubes.txt 80 3+4
  
run_single: $(EXENAME)
	./$(EXENAME) data/scene_bunny1.txt 800 800


 
 

