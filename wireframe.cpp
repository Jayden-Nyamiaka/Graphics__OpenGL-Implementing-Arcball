#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits.h>
#include <cmath>
#include <algorithm>

#include "utils.h"
#include "transformation.h"
#include "color.h"
#include "wireframe.h"

using Eigen::Vector4f;

void Wireframe::saveTransformedCopy(string objectName, 
                                    color_rgb_t ambient,
                                    color_rgb_t diffuse,
                                    color_rgb_t specular,
                                    float shininess,
                                    Matrix4f vertexTransformation,
                                    Matrix4f surfNormTransformation) {
    Object objCopy = originals[objectName].copy();

    /* Generates a unique name for the copy */
    int copyNumber = 1;
    string nameAttempt = objectName + "_copy" + to_string(copyNumber);
    while (copies.find(nameAttempt) != copies.end()) {
        copyNumber++;
        nameAttempt = objectName + "_copy" + to_string(copyNumber);
    }
    objCopy.name = nameAttempt;

    objCopy.ambient = ambient;
    objCopy.diffuse = diffuse;
    objCopy.specular = specular;
    objCopy.shininess = shininess;

    /* Transforms all vertexes of the copy */
    for (size_t i = 1; i < objCopy.vertexes.size(); i++) {
        Vector4f point(objCopy.vertexes[i].x, objCopy.vertexes[i].y, objCopy.vertexes[i].z, 1);
        Vector4f result = vertexTransformation * point;
        objCopy.vertexes[i].x = result[0] / result[3];
        objCopy.vertexes[i].y = result[1] / result[3];
        objCopy.vertexes[i].z = result[2] / result[3];
    }

    /* Sets surfNormTransformation to X = (M^-1)^T given that
       M is the scaling, rotation only transformation matrix */
    Matrix4f intermediate = surfNormTransformation.inverse();
    surfNormTransformation = intermediate.transpose();

    /* Transforms all surface normals of the copy */
    for (size_t i = 1; i < objCopy.normals.size(); i++) {
        Vector4f point(objCopy.normals[i].x, objCopy.normals[i].y, objCopy.normals[i].z, 1);
        Vector4f result4d = surfNormTransformation * point;
        Vector3f surfNorm(result4d[0], result4d[1], result4d[2]);
        surfNorm.normalize(); // ensures surface normals are unit vectors
        objCopy.normals[i].x = surfNorm[0];
        objCopy.normals[i].y = surfNorm[1];
        objCopy.normals[i].z = surfNorm[2];
    }
    
    copies.insert({objCopy.name, objCopy});
}


void Wireframe::processFormatFile(string filename) {
    if (filename.find(".txt") == -1) {
        throw invalid_argument("File " + filename + " needs to be a .txt file.");
    }

    string buffer;
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    if (file.fail()) {
        throw invalid_argument("Could not read format file '" + filename + "'.");
    }

    /* Saves filename to Wireframe except w/o the '.txt' suffix */
    filename.erase(filename.find('.'));
    file_name = filename;

    /* Saves the directory where scene file and its obj files are stored */
    string directory = filename;
    directory.erase(directory.find_last_of('/') + 1);

    vector<string> line;

    /* Reads in camera and perspective parameters */
    while (getline(file, buffer)) {
        line.clear();
        splitBySpace(buffer, line);

        if (line.size() == 0) {
            break;
        } else if (line[0] == "position") {
            cam_pos = initVertex(stod(line[1]), stod(line[2]), stod(line[3]));
        } else if (line[0] == "orientation") {
            cam_orien = initVertex(stod(line[1]), stod(line[2]), stod(line[3]));
            cam_angle = stod(line[4]);
        } else if (line[0] == "near") {
            perspec.near = stod(line[1]);
        } else if (line[0] == "far") {
            perspec.far = stod(line[1]);
        } else if (line[0] == "left") {
            perspec.left = stod(line[1]);
        } else if (line[0] == "right") {
            perspec.right = stod(line[1]);
        } else if (line[0] == "top") {
            perspec.top = stod(line[1]);
        } else if (line[0] == "bottom") {
            perspec.bottom = stod(line[1]);
        }
    }

    /* Reads in camera and perspective parameters */
    while (getline(file, buffer)) {
        line.clear();
        splitBySpace(buffer, line);

        if (line.size() == 0) {
            break;
        }

        light_t light;
        light.pos = initVertex(stod(line[1]), stod(line[2]), stod(line[3]));
        light.color = initColor(stod(line[5]), stod(line[6]), stod(line[7]));
        light.atten = stod(line[9]);
        lights.push_back(light);
    }

    /* Reads in all objects storing them in object maps */
    while (getline(file, buffer)) {
        line.clear();
        splitBySpace(buffer, line);

        if (line.size() == 0) {
            break;
        }

        if (line[0] == "objects:") {
            continue;
        }

        Object obj = Object(directory + line[1]);
        obj.name = line[0];
        originals.insert({obj.name, obj});
    }

    /* Reads in all objects to transform as copies along with other params */
    string objectName = "";
    color_rgb_t ambient;
    color_rgb_t diffuse;
    color_rgb_t specular;
    float shininess;
    Matrix4f vertexTransformation;
    Matrix4f surfNormTransformation;
    bool first_run = true;
    while (getline(file, buffer)) {
        line.clear();
        splitBySpace(buffer, line);

        if (objectName.empty()) {
            objectName = line[0];
            continue;
        }

        if (line.size() == 0) {
            saveTransformedCopy(objectName, ambient, diffuse, specular, 
                shininess, vertexTransformation, surfNormTransformation);
            objectName = "";
            first_run = true;
            continue;
        }

        if (line[0] == "ambient") {
            ambient = initColor(stod(line[1]), stod(line[2]), stod(line[3]));
            continue;
        } else if (line[0] == "diffuse") {
            diffuse = initColor(stod(line[1]), stod(line[2]), stod(line[3]));
            continue;
        } else if (line[0] == "specular") {
            specular = initColor(stod(line[1]), stod(line[2]), stod(line[3]));
            continue;
        } else if (line[0] == "shininess") {
            shininess = stod(line[1]);
            continue;
        }

        /* Processes one line of the file as a transformation matrix */
        Matrix4f curr;
        if (line[0][0] == 't') {
            curr = translation(stod(line[1]), stod(line[2]), stod(line[3]));
        } else if (line[0][0] == 'r') {
            curr = rotation(stod(line[1]), stod(line[2]), stod(line[3]), 
                                                        stod(line[4]));;
        } else {
            curr = scaling(stod(line[1]), stod(line[2]), stod(line[3]));
        }

        if (first_run) {
            if (line[0][0] == 't') {
                surfNormTransformation = getIdentity4d();
            } else {
                surfNormTransformation = curr;
            }
            vertexTransformation = curr;
            first_run = false;
            continue;
        }

        if (line[0][0] != 't') {
            surfNormTransformation = curr * surfNormTransformation;
        }
        vertexTransformation = curr * vertexTransformation;
    }
    saveTransformedCopy(objectName, ambient, diffuse, specular, 
        shininess, vertexTransformation, surfNormTransformation);
    file.close();
}


void Wireframe::printScene() {
    cout << "Filename: " << file_name << endl;
    cout << "Resolution " << xres << " by " << yres << endl;
    cout << "Shading Mode " << shading_mode << endl << endl;

    cout << "camera:" << endl;
    cout << "position (" << cam_pos.x << ", " << cam_pos.y << ", " << cam_pos.z << ")" << endl; 
    cout << "orientation (" << cam_orien.x << ", " << cam_orien.y << ", " << cam_orien.z << ") with angle " << cam_angle << endl; 
    cout << "near " << perspec.near << endl;
    cout << "far " << perspec.far << endl;
    cout << "left " << perspec.left << endl;
    cout << "right " << perspec.right << endl;
    cout << "top " << perspec.top << endl;
    cout << "bottom " << perspec.bottom << endl << endl;

    for (size_t i = 0; i < lights.size(); i++) {
        light_t l = lights[i];
        cout << "light ";
        cout << l.pos.x << " " << l.pos.y << " " << l.pos.z << " , ";
        cout << l.color.r << " " << l.color.g << " " << l.color.b << " , ";
        cout << l.atten << endl;
    }

    cout << endl << "All Original Objects" << endl;
    for (map<string, Object>::iterator iter = originals.begin(); 
                                    iter != originals.end(); iter++) {
        iter->second.printContents();
    }

    cout << endl << "All Object Copies" << endl;
    for (map<string, Object>::iterator iter = copies.begin(); 
                                    iter != copies.end(); iter++) {
        iter->second.printContents();
        iter->second.printPixels();
    }

    if (grid == NULL) {
        cout << endl << "Pixel Grid is empty." << endl;
    } else {
        cout << "\n\n\nPixel Grid:\n";
        for (int y = 0; y < yres; y++) {
            for (int x = 0; x < xres; x++) {
                cout << "(" << y << ", " << x << "): " << toString(grid[y][x]) << endl;
            }
        }
    }

    if (depthBuffer == NULL) {
        cout << endl << "Depth Buffer is empty." << endl;
    } else {
        cout << "\n\n\nDepth Buffer:\n";
        for (int y = 0; y < yres; y++) {
            for (int x = 0; x < xres; x++) {
                cout << "(" << y << ", " << x << "): " << depthBuffer[y][x] << endl;
            }
        }
    }
}


void Wireframe::computeTransforms() {
    Matrix4f m_TcTr = translation(cam_pos.x, cam_pos.y, cam_pos.z) *
                 rotation(cam_orien.x, cam_orien.y, cam_orien.z, cam_angle);
    cam_space_transform = m_TcTr.inverse();

    float div_r1 = perspec.right - perspec.left;
    float div_r2 = perspec.top - perspec.bottom;
    float div_r3 = perspec.far - perspec.near;
    perspec_proj_transform << 
        (2 * perspec.near) / div_r1, 0, (perspec.right + perspec.left) / div_r1, 0,
        0, (2 * perspec.near) / div_r2, (perspec.top + perspec.bottom) / div_r2, 0,
        0, 0, -(perspec.far + perspec.near) / div_r3, -2 * perspec.far * perspec.near / div_r3,
        0, 0, -1, 0;
    
    homogenousNDC_transform = perspec_proj_transform * cam_space_transform;
    return;
}


void Wireframe::applyTransforms(Object &copy) {
    for (size_t i = 1; i < copy.vertexes.size(); i++) {
        Vector4f point(copy.vertexes[i].x, copy.vertexes[i].y, copy.vertexes[i].z, 1);
        Vector4f result = homogenousNDC_transform * point;
        
        // Saves Word Space vertex for Phong Shading
        if (shading_mode == 1) {
            copy.vertexesWorld[i] = copy.vertexes[i];
        }

        copy.vertexes[i].x = result[0] / result[3];
        copy.vertexes[i].y = result[1] / result[3];
        copy.vertexes[i].z = result[2] / result[3];

        /* 
            * Mapping: [x, y] --> 
            * [0.5 * xres ((x - left) / (right - left) + 0.5), 
            *  0.5 * yres ((top - y) / (top - bottom) + 0.5)]
        */
        int grid_x = round(0.5 * xres * ((copy.vertexes[i].x - perspec.left) / 
                (perspec.right - perspec.left) + 0.5) );

        int grid_y = round(0.5 * yres * ((perspec.top - copy.vertexes[i].y) / 
                (perspec.top - perspec.bottom) + 0.5) );

        copy.pixels.push_back(initGridVertex(grid_x, grid_y));
    }
    
}


bool Wireframe::pointInBoundGrid(int y, int x) {
    if (y < 0 || y >= yres || x < 0 || x >= xres) {
        return false;
    }
    return true;
}


void Wireframe::plotPoint(int y, int x, color_rgb_t color) {
    if (pointInBoundGrid(y, x)) {
        grid[y][x] = color;
    }
}


color_rgb_t Wireframe::pointLighting(Vector3f point, Vector3f normal, Object &obj) {
    /* Converts parameters to Vector3f for calculations */
    Vector3f cameraPos(cam_pos.x, cam_pos.y, cam_pos.z);
    Vector3f cameraDir = cameraPos - point;
    cameraDir.normalize();

    /* Defines Color Component Sums for Diffuse & Specular Light Reflection */
    color_rgb_t diffuseTotal = initColor(0, 0, 0);
    color_rgb_t specularTotal = initColor(0, 0, 0);

    for (size_t i = 0; i < lights.size(); i++) {
        Vector3f lightPos(lights[i].pos.x, lights[i].pos.y, lights[i].pos.z);
        Vector3f lightDir = lightPos - point;
        lightDir.normalize();

        // Compute Attenuation Factor
        float xDif = (point[0] - lightPos[0]);
        float yDif = (point[1] - lightPos[1]);
        float zDif = (point[2] - lightPos[2]);
        float distPointToLightSquared = xDif*xDif + yDif*yDif + zDif*zDif;
        float attenFactor = 
            1.0 / (1.0 + lights[i].atten * distPointToLightSquared);

        // Computes Diffuse Reflection Component
        color_rgb_t diffuseComp = scaleColor(lights[i].color, 
                            max((float)0, attenFactor * normal.dot(lightDir)));
        diffuseTotal = addColors(diffuseTotal, diffuseComp);

        // Computes Diffuse Reflection Component
        Vector3f sumDir = cameraDir + lightDir;
        sumDir.normalize();
        float specularFactor = pow(max((float)0, normal.dot(sumDir)), obj.shininess);
        color_rgb_t specularComp = 
            scaleColor(lights[i].color, attenFactor * specularFactor);
        specularTotal = addColors(specularTotal, specularComp);
    }

    // Sums Ambient, Diffuse, & Specular keeping rgb values within [0, 1]
    float red = min((float)1, obj.ambient.r + 
                              obj.diffuse.r * diffuseTotal.r + 
                              obj.specular.r * specularTotal.r);
    float green = min((float)1, obj.ambient.g + 
                                obj.diffuse.g * diffuseTotal.g + 
                                obj.specular.g * specularTotal.g);
    float blue = min((float)1, obj.ambient.b + 
                               obj.diffuse.b * diffuseTotal.b + 
                               obj.specular.b * specularTotal.b);
    return initColor(red, green, blue);
}


void Wireframe::shadeGouraud(int pixel_y, int pixel_x,
                             float alpha, float beta, float gamma,
                             color_rgb_t c1, color_rgb_t c2, color_rgb_t c3) {
    /* Interpolates color of point by weighing colors of each triangle vertex
       by the component subtriangle area the point makes with the vertexes */
    float red = min((float)1, alpha * c1.r + beta * c2.r + gamma * c3.r);
    float green = min((float)1, alpha * c1.g + beta * c2.g + gamma * c3.g);
    float blue = min((float)1, alpha * c1.b + beta * c2.b + gamma * c3.b);
    plotPoint(pixel_y, pixel_x, initColor(red, green, blue));
}


void Wireframe::shadePhong(int pixel_y, int pixel_x, Object &copy,
                           float alpha, float beta, float gamma,
                           Vector3f v1, Vector3f v2, Vector3f v3,
                           Vector3f sn1, Vector3f sn2, Vector3f sn3) {
    // Finds the pixel point and surface normal in World Space
    Vector3f v_point = alpha * v1 + beta * v2 + gamma * v3;
    Vector3f sn_point = alpha * sn1 + beta * sn2 + gamma * sn3;

    // Shades the color at that pixel by computing the color directly
    color_rgb_t color = pointLighting(v_point, sn_point, copy);
    plotPoint(pixel_y, pixel_x, color);
}


float Wireframe::barycentricFunction(int vp_x, int vp_y, int vi_x, int vi_y, int vj_x, int vj_y) {
    return (vi_y - vj_y) * vp_x + (vj_x - vi_x) * vp_y + vi_x * vj_y - vj_x * vi_y;
}


void Wireframe::computeColorsByFaceGouraud(Object &copy) {
    for (int face_idx = 0; face_idx < copy.faces.size(); face_idx++) {
        face_t face = copy.faces[face_idx];

        // Prints percent complete every PROGRESS_INTERVAL faces if progress printing is set on
        if (printPlotProgress && face_idx % PROGRESS_INTERVAL == 0) {
            cerr << "\tColoring " << round(face_idx * 100.0 / copy.faces.size()) << "%" << endl;
        }

        /* Gets the vertexes and surface normals of the triangle's points 
            as Vector3f's to ease computations */
        vertex_t vtxW1 = copy.vertexes[face.v1];
        vertex_t vtxW2 = copy.vertexes[face.v2];
        vertex_t vtxW3 = copy.vertexes[face.v3];
        surf_norm_t snW1 = copy.normals[face.sn1];
        surf_norm_t snW2 = copy.normals[face.sn2];
        surf_norm_t snW3 = copy.normals[face.sn3];
        Vector3f vWorld1(vtxW1.x, vtxW1.y, vtxW1.z);
        Vector3f vWorld2(vtxW2.x, vtxW2.y, vtxW2.z);
        Vector3f vWorld3(vtxW3.x, vtxW3.y, vtxW3.z);
        Vector3f snWorld1(snW1.x, snW1.y, snW1.z);
        Vector3f snWorld2(snW2.x, snW2.y, snW2.z);
        Vector3f snWorld3(snW3.x, snW3.y, snW3.z);

        copy.colorByFace[face_idx][0] = pointLighting(vWorld1, snWorld1, copy);
        copy.colorByFace[face_idx][1] = pointLighting(vWorld2, snWorld2, copy);
        copy.colorByFace[face_idx][2] = pointLighting(vWorld3, snWorld3, copy);
    }
}


void Wireframe::rasterColoredTriangles(Object &copy) {
    for (int face_idx = 0; face_idx < copy.faces.size(); face_idx++) {
        face_t face = copy.faces[face_idx];

        // Prints percent complete every PROGRESS_INTERVAL faces if progress printing is set on
        if (printPlotProgress && face_idx % PROGRESS_INTERVAL == 0) {
            cerr << "\tRasterizing " << round(face_idx * 100.0 / copy.faces.size()) << "%" << endl;
        }
                
        // Gets NDC vertexes of triangle face as Vector3f's to ease computation
        vertex_t v1 = copy.vertexes[face.v1];
        vertex_t v2 = copy.vertexes[face.v2];
        vertex_t v3 = copy.vertexes[face.v3];
        Vector3f vNDC1(v1.x, v1.y, v1.z);
        Vector3f vNDC2(v2.x, v2.y, v2.z);
        Vector3f vNDC3(v3.x, v3.y, v3.z);

        // Ensures face faces towards camera [Backface Culling Optimization]
        Vector3f cross = (vNDC3 - vNDC2).cross(vNDC1 - vNDC2);
        if (cross[2] < 0) {
            continue;
        }

        // Gets pixel screen coordinates for triangle vertexes
        grid_vertex_t pxl1 = copy.pixels[face.v1];
        grid_vertex_t pxl2 = copy.pixels[face.v2];
        grid_vertex_t pxl3 = copy.pixels[face.v3];

        // Defines the parallelogram that bounds all the points on the face
        int xmin = min({pxl1.x, pxl2.x, pxl3.x});
        int xmax = max({pxl1.x, pxl2.x, pxl3.x});
        int ymin = min({pxl1.y, pxl2.y, pxl3.y});
        int ymax = max({pxl1.y, pxl2.y, pxl3.y});

        // Computes color for and shades every pixel of the given object
        for (int x = xmin; x <= xmax; x++) {
            // Ensures x coord is in Pixel Grid (and thus also in NDC Cube)
            if (x < 0 || x >= xres) {
                continue;
            }
            for (int y = ymin; y <= ymax; y++) {
                // Ensures y coord is in Pixel Grid (and thus also in NDC Cube)
                if (y < 0 || y >= yres) {
                    continue;
                }

                /* Computes parameters for Barycentric Coordinates */
                // Computes denominators first to ensure there's no division by 0
                float alpha = barycentricFunction(pxl1.x, pxl1.y, pxl2.x, pxl2.y, pxl3.x, pxl3.y);
                float beta = barycentricFunction(pxl2.x, pxl2.y, pxl1.x, pxl1.y, pxl3.x, pxl3.y);
                float gamma = barycentricFunction(pxl3.x, pxl3.y, pxl1.x, pxl1.y, pxl2.x, pxl2.y);
                if (alpha == 0 || beta == 0 || gamma == 0) {
                    continue;
                }
                // Finishes computation by dividing numerator by ensured non-zero denominator
                alpha = barycentricFunction(x, y, pxl2.x, pxl2.y, pxl3.x, pxl3.y) / alpha;
                beta = barycentricFunction(x, y, pxl1.x, pxl1.y, pxl3.x, pxl3.y) / beta;
                gamma = barycentricFunction(x, y, pxl1.x, pxl1.y, pxl2.x, pxl2.y) / gamma;

                // Ensures point is within Barycentric Triangle
                if (alpha < 0 || alpha > 1 || beta < 0 || beta > 1 || gamma < 0 || gamma > 1) {
                    continue;
                }

                // Computes the point in NDC Coordinates
                float pointNDCz = alpha * vNDC1[2] + beta * vNDC2[2] + gamma * vNDC3[2];
                
                // Ensures z coord is in NDC Cube
                if (pointNDCz < -perspec.near) {
                    continue;
                }

                /* Ensures there's no point plotted in front [Depth Buffering Optimization] */
                if (pointNDCz >= depthBuffer[y][x]) {
                    continue;
                }

                // Records this z value as nearest z value computed for this pixel
                depthBuffer[y][x] = pointNDCz;

                // Colors pixel using Gouraud or Phong Shading depending on mode
                if (shading_mode == 0) {
                    shadeGouraud(y, x, alpha, beta, gamma, 
                                 copy.colorByFace[face_idx][0], 
                                 copy.colorByFace[face_idx][1],
                                 copy.colorByFace[face_idx][2]);
                } else {
                    vertex_t vtxW1 = copy.vertexesWorld[face.v1];
                    vertex_t vtxW2 = copy.vertexesWorld[face.v2];
                    vertex_t vtxW3 = copy.vertexesWorld[face.v3];
                    surf_norm_t snW1 = copy.normals[face.sn1];
                    surf_norm_t snW2 = copy.normals[face.sn2];
                    surf_norm_t snW3 = copy.normals[face.sn3];
                    Vector3f vWorld1(vtxW1.x, vtxW1.y, vtxW1.z);
                    Vector3f vWorld2(vtxW2.x, vtxW2.y, vtxW2.z);
                    Vector3f vWorld3(vtxW3.x, vtxW3.y, vtxW3.z);
                    Vector3f snWorld1(snW1.x, snW1.y, snW1.z);
                    Vector3f snWorld2(snW2.x, snW2.y, snW2.z);
                    Vector3f snWorld3(snW3.x, snW3.y, snW3.z);
                    shadePhong(y, x, copy, alpha, beta, gamma, 
                               vWorld1, vWorld2, vWorld3, snWorld1, snWorld2, snWorld3);
                }
            }
        }
    }
}

void Wireframe::plot(bool shouldPrintProgress) {
    printPlotProgress = shouldPrintProgress;

    // Allocates data for Pixel Grid and colors all pixels black
    grid = (color_rgb_t **) malloc(yres * sizeof(color_rgb_t *));
    for (int y = 0; y < yres; y++) {
        grid[y] = (color_rgb_t *) malloc(xres * sizeof(color_rgb_t));
        for (int x = 0; x < xres; x++) {
            grid[y][x] = initColor(0, 0, 0);
        }
    }

    // Allocates data for Depth Buffer and sets all z values to max float value
    depthBuffer = (float **) malloc(yres * sizeof(float *));
    for (int y = 0; y < yres; y++) {
        depthBuffer[y] = (float *) malloc(xres * sizeof(float));
        for (int x = 0; x < xres; x++) {
            depthBuffer[y][x] = numeric_limits<float>::max();
        }
    }

    computeTransforms();
    for (map<string, Object>::iterator obj_iter = copies.begin(); 
                                    obj_iter != copies.end(); obj_iter++) {
        Object &copy = copies[obj_iter->first];                                
        if (printPlotProgress) {
            cerr << "Track " << copy.name << "..." <<  endl;
        }
        if (shading_mode == 0) {  
            // Allocates data for ColorsByFace Gouraud field with Dim: faces.size() by 3
            copy.colorByFace = (color_rgb_t **) malloc(copy.faces.size() * sizeof(color_rgb_t *));
            for (int face_idx = 0; face_idx < copy.faces.size(); face_idx++) {
                copy.colorByFace[face_idx] = (color_rgb_t *) malloc(3 * sizeof(color_rgb_t));
            }
            computeColorsByFaceGouraud(copy);
            if (printPlotProgress) {
                cerr << "\tColoring DONE." << endl;
            }
            applyTransforms(copy);
            rasterColoredTriangles(copy);
            if (printPlotProgress) {
                cerr << "\tRasterizing DONE." << endl << endl;
            }
        } else {
            // Allocates data for vertexesWorld Phong field with size = vertexes.size()
            copy.vertexesWorld = (vertex_t *) malloc(copy.vertexes.size() * sizeof(vertex_t));
            applyTransforms(copy);
            rasterColoredTriangles(copy);
            if (printPlotProgress) {
                cerr << "\tRasterizing DONE." << endl << endl;
            }
        }
    }
}


void Wireframe::output(bool printToStd) {
    ofstream ppm;
    string filename = file_name;
    if (shading_mode == 0) {
        filename += "_Gouraud";
    } else {
        filename += "_Phong";
    }
    filename += ".ppm";
    ppm.open(filename.c_str(), ios::out);
    if (!ppm) {
        string msg = "Could not create '" + filename + "'.";
        throw runtime_error(msg);
    }
    
    ppm << "P3\n" << xres << " " << yres << "\n255\n";
    if (printToStd) {
        cout << "P3\n" << xres << " " << yres << "\n255\n";
    }

    for (int y = 0; y < yres; y++) {
        for (int x = 0; x < xres; x++) {
            string colorStr = toString(grid[y][x]);
            ppm << colorStr << endl;
            if (printToStd) {
                cout << colorStr << endl;
            }
        }
    }

    ppm.close();
}


void Wireframe::destruct() {
    for (int y = 0; y < yres; y++) {
        free(grid[y]);
    }
    free(grid);

    for (int y = 0; y < yres; y++) {
        free(depthBuffer[y]);
    }
    free(depthBuffer);

    for (map<string, Object>::iterator obj_iter = copies.begin(); 
                                    obj_iter != copies.end(); obj_iter++) {
        Object &copy = copies[obj_iter->first];
        copy.destruct();
    }
}


void usage(void) {
    cerr << "Enter input in the form: scene_description_file.txt xres yres mode\n\t"
            "xres, yres must be positive integers\n\t"
            "mode must be either 0 or 1. 0: Gouraud Shading; 1: Phong Shading\n";
    exit(1);
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        usage();
    }

    try {
        Wireframe pipeline;
        pipeline.xres = stoi(argv[2]);
        pipeline.yres = stoi(argv[3]);
        pipeline.shading_mode = stoi(argv[4]);
        pipeline.processFormatFile(argv[1]);
        if (pipeline.xres <= 0 || pipeline.yres <= 0 ||
            (pipeline.shading_mode != 0 && pipeline.shading_mode != 1)) {
            usage();
        }
        pipeline.plot(true);
        pipeline.output(false);
        pipeline.destruct();
    } catch (const char *msg) {
        cerr << msg << endl;
    }

    return 0;
}