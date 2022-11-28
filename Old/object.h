#ifndef OBJECT_H
#define OBJECT_H
 
#include <vector>
#include <string>
#include <stdexcept>

#include "color.h"

using namespace std;

typedef struct vertex {
    float x;
    float y;
    float z;
} vertex_t;

vertex_t initVertex(float a, float b, float c);

typedef struct surf_norm {
    float x;
    float y;
    float z;
} surf_norm_t;

surf_norm_t initSurfNorm(float a, float b, float c);

typedef struct face {
    int v1;
    int sn1;
    int v2;
    int sn2;
    int v3;
    int sn3;
} face_t;

face_t initFace(int v_1, int sn_1, int v_2, int sn_2, int v_3, int sn_3);

typedef struct gridVertex {
    int x;
    int y;
} grid_vertex_t;

grid_vertex_t initGridVertex(int a, int b);

class Object {
    public:
        string name;

        vector<vertex_t> vertexes;
        vector<surf_norm_t> normals;
        vector<face_t> faces;

        color_rgb_t ambient;
        color_rgb_t diffuse;
        color_rgb_t specular;
        float shininess; // Phong exponent p for material shininess

        /* All of Object's vertexes that are inside the 
           perspective cube mapped to a 2D NDC pixel grid */
        vector<grid_vertex_t> pixels;

        // Note: Arrays used for Gouraud and Phong Parameters for Speed Increase

        // Gouraud Parameters
        // Dim: faces.size() by 3, used to compute colors before NDC transform
        color_rgb_t **colorByFace; 

        // Phong Parameters
        // Size is equal to vertexes.size(), used to save World coords before transforming to NDC
        vertex_t *vertexesWorld;


        /**
         * Base constructor that initializes an empty Object.
         */
        Object();

        /**
         * Constructor that populates Object by reading 'filename' .obj file.
         * 
         * @param filename of the .obj file to be processed
         * @throws invalid_argument if it fails to read the file
         */
        Object(string filename);

        /**
         * Returns a deep copy of the object.
         * NOTE: This excludes Shading Params - colorByFace & vertexesWorld.
         * 
         * @returns a deep copy of Object
         */
        Object copy();

        /** 
         * Populates Object by reading the 'filename' .obj file.
         * 
         * If the object has no name, names the object the filename.
         * 
         * @param filename of the .obj file to be processed
         * @throws invalid_argument if it fails to read the file
         */ 
        void processFile(string filename);

        /**
         * Prints contents of the Object in a format similar to what's read in.
         */
        void printContents(); 

        /**
         * Prints out all the Pixel Grid vertexes.
         */
        void printPixels();

        /**
         * Destructs Object by freeing any allocated data.
        */
        void destruct();

    private:
        /* Private helper functions the user doesn't need to know */
        void init();
        void setFileName(string filename);
};

#endif