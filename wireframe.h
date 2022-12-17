#ifndef WIREFRAME_H
#define WIREFRAME_H

#include <map>
#include "object.h"
#include "transformation.h"

using Eigen::Vector3f;

using namespace std;

// Defines # of object faces to print progress at if progress printing is on
#define PROGRESS_INTERVAL 30000

typedef struct perspective {
    float near;
    float far;
    float left;
    float right;
    float top;
    float bottom;
} perspective_t;

typedef struct point_light {
    vertex_t pos;
    color_rgb_t color;
    float atten; // k, attenuation parameter
} light_t;

class Wireframe {
    public:
        /* File name used to populate Wireframe 
           from processFormatFile with '.txt' removed */
        string file_name;
        /* PPM Resolution */
        int xres, yres;
        /* Shading Mode [0: Gouraud Shading, 1: Phong Shading] */
        int shading_mode;
        /* Camera parameters */
        vertex_t cam_pos;
        vertex_t cam_orien;
        float cam_angle;
        /* Perspective parameters */
        perspective_t perspec;
        /* Vector of all point lights in the scene */
        vector<light_t> lights;
        /* Transformation matrices*/
        Matrix4f cam_space_transform;
        Matrix4f perspec_proj_transform;
        Matrix4f homogenousNDC_transform;
        /* Original read in objects mapped by name */
        map<string, Object> originals;
        /* Copies of the read in objects that are transformed, 
           converted to NDC Coords and mapped to a Pixel Grid */
        map<string, Object> copies;
        /* Cartesian NDC Pixel Grid with the color of each Pixel */
        color_rgb_t** grid;
        /* Depth Buffer Grid used for Depth Buffer Optimization */
        float **depthBuffer;
        /* Flag set during Plot method to decide if plotting progress is printed
           Note: Gouraud Shading has Coloring and Rasterizing Progress while 
                 Phong Shading only has Rasterizing Progress */
        bool printPlotProgress;


        /** 
         * Populates Wireframe properties by reading from format .txt file.
         * 
         * @param filename of the .txt file to be processed
         * @throws invalid_argument if it fails to read the file
         */ 
        void processFormatFile(string filename);
        
        /**
         * Prints everything in the scene (contents of Wireframe Object).
         * 
         * Used mainly for debugging & checking if input's been read correctly.
        */
        void printScene();

        /**
         * Plots and colors the object copies on the Pixel Grid.
         * 
         * Note: This allocates data for grid and depthBuffer (freed in destruct).
         * 
         * @param shouldPrintProgress, sets printPlotProgress
        */
        void plot(bool shouldPrintProgress);

        /**
         * Writes the final output image computed as a PPM to a file.
         * Also prints the final image to standard out if printToStd is true.
         * 
         * @param printToStd boolean that decides if image is also printed
         * @throws invalid_argument if it fails to open the file
        */
        void output(bool printToStd);

        /**
         * Destructs the Wireframe object by freeing any allocated data.
        */
        void destruct();

    private:
        /**
         * Takes an object name and saves a transformed copy with light 
         * parameters to copies maps.
         * 
         * @param objectName, the object name (acts as key in untouched map)
         * @param ambient, the ambient color param for the object copy
         * @param diffuse, the diffuse color param for the object copy
         * @param specular, the specular color param for the object copy
         * @param shininess, the shininess factor param for the object copy
         * @param vertexTransformation, the Matrix to transform all vertexes
         * @param surfNormTransformation, the intermediate Matrix to transform all normals
         *                                (the transpose of the inverse still needs to be taken)
        */
        void saveTransformedCopy(string objectName, 
                                 color_rgb_t ambient,
                                 color_rgb_t diffuse,
                                 color_rgb_t specular,
                                 float shininess,
                                 Matrix4f vertexTransformation,
                                 Matrix4f surfNormTransformation);

        /**
         * Uses the camera and persepective parameters to compute and save 
         * the camera, perspective, and homogenous NDC transformation matrices.
        */
        void computeTransforms();

        /**
         * Applies the homogenous NDC transformations to the given object copy.
         * 
         * Note: Saves a copy of the World Space vertexes to the array 
         *       copy.vertexesWorld for Phong Shading.
         * 
         * @param, the Object to transform
        */
        void applyTransforms(Object &copy);


        /**
         * Returns if the point (y,x) lies within the Pixel Grid.
         * 
         * @param y, the y component of the point (y, x)
         * @param x, the x component of the point (y, x)
         * @returns, true if point lies within Pixel Grid, false otherwise
        */
        bool pointInBoundGrid(int y, int x);

        /**
         * Plots the given point (y, x) if it lies within the Pixel Grid.
         * 
         * @param y, the y component of the point (y, x)
         * @param x, the x component of the point (y, x)
         * @param color, the rgb color of the point 
        */
        void plotPoint(int y, int x, color_rgb_t color);

        /**
         * Computes the color of a point on an Object's surface.
         * 
         * Note: Function must use World Space Coordinates, NOT NDC.
         * 
         * @param point, the vertex point for the Object
         * @param normal, the surface normal of the vertex point
         * @param obj, the Object for which the color is being computed
         * @returns, the color at the given World Space vertex
        */
        color_rgb_t pointLighting(Vector3f point, 
                                  Vector3f normal, 
                                  Object &obj);

        /**
         * Shades a pixel point (y, x) on a triangle face of an Object using Gouraud Shading.
         * AKA Per-Vertex Shading
         * 
         * Note: Function must use World Space Coordinates, NOT NDC.
         * 
         * Gouraud Shading Algorithm:
         * Computes color of (y, x) by summing the color at each vertex of the triangle
         * weighted by the area made between the point and the sides of the triangle.
         * 
         * @param pixel_y, the y component of the pixel being shaded / colored
         * @param pixel_x, the x component of the pixel being shaded / colored
         * @param alpha, the alpha param of triangle's Barycentric Coordinates
         * @param beta, the beta param of triangle's Barycentric Coordinates
         * @param gamma, the gamma param of triangle's Barycentric Coordinates
         * @param c1, the color at the first vertex of the triangle face
         * @param c2, the color at the second vertex of the triangle face
         * @param c3, the color at the third vertex of the triangle face
         */
        void shadeGouraud(int pixel_y, int pixel_x,
                          float alpha, float beta, float gamma,
                          color_rgb_t c1, color_rgb_t c2, color_rgb_t c3);

        /**
         * Shades a pixel point (y, x) on a triangle face of an Object using Phong Shading.
         * AKA Per-Pixel Shading
         * 
         * Note: Function must use World Space Coordinates, NOT NDC.
         * 
         * Phong Shading Algorithm:
         * Computes color of (y, x) by computing the World Space pixel point 
         * and surface normal then computing the color of that pixel directly.
         * 
         * @param pixel_y, the y component of the pixel being shaded / colored
         * @param pixel_x, the x component of the pixel being shaded / colored
         * @param copy, the Object to shade for (passed in to pointLighting)
         * @param alpha, the alpha param of triangle's Barycentric Coordinates
         * @param beta, the beta param of triangle's Barycentric Coordinates
         * @param gamma, the gamma param of triangle's Barycentric Coordinates
         * @param v1, the first vertex of the triangle in World Space
         * @param v2, the second vertex of the triangle in World Space
         * @param v3, the third vertex of the triangle in World Space
         * @param sn1, the surface normal of the first vertex in World Space
         * @param sn2, the surface normal of the second vertex in World Space
         * @param sn3, the surface normal of the third vertex in World Space
         */
        void shadePhong(int pixel_y, int pixel_x, Object &copy,
                          float alpha, float beta, float gamma,
                          Vector3f v1, Vector3f v2, Vector3f v3,
                          Vector3f sn1, Vector3f sn2, Vector3f sn3);

        /**
         * Key function that lets us easily compute the parameters for the 
         * Barycentric Coordinates of a Pixel Grid point on a triangle surface.
         * 
         * @param vp_x, the x component of vertex of the point being computed
         * @param vp_y, the y component of vertex of the point being computed
         * @param vi_x, the x component of vertex i of the triangle
         * @param vi_y, the y component of vertex i of the triangle
         * @param vj_x, the x component of vertex j of the triangle
         * @param vj_y, the y component of vertex j of the triangle
         * @returns, the integer result of the function
        */
        float barycentricFunction(int vp_x, int vp_y, int vi_x, int vi_y, int vj_x, int vj_y);

        /**
         * Saves the color at every vertex of every face for a given object
         * ONLY for Gouraud Shading.
         * 
         * Note: The color is saved to the array copy.colorsByFace for Gouraud Shading.
         * 
         * @param copy, Object to save the colors for
        */
        void computeColorsByFaceGouraud(Object &copy);

        /**
         * Rasters and colors a given object.
         * 
         * @param copy, the Object to raster for
        */
        void rasterColoredTriangles(Object &copy);
};

#endif