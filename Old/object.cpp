#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <iostream>

#include "utils.h"

#include "object.h"

vertex_t initVertex(float a, float b, float c) {
    vertex_t v;
    v.x = a;
    v.y = b;
    v.z = c;
    return v;
}

surf_norm_t initSurfNorm(float a, float b, float c) {
    surf_norm_t sn;
    sn.x = a;
    sn.y = b;
    sn.z = c;
    return sn;
}

face_t initFace(int v_1, int sn_1, int v_2, int sn_2, int v_3, int sn_3) {
    face_t f;
    f.v1 = v_1;
    f.sn1 = sn_1;
    f.v2 = v_2;
    f.sn2 = sn_2;
    f.v3 = v_3;
    f.sn3 = sn_3;
    return f;
}

grid_vertex_t initGridVertex(int a, int b) {
    grid_vertex_t v;
    v.x = a;
    v.y = b;
    return v;
}

void Object::init() {
    vertexes.push_back(initVertex(0, 0, 0));
    normals.push_back(initSurfNorm(0, 0, 0));
    pixels.push_back(initGridVertex(0, 0));

    ambient = initColor(0, 0, 0);
    diffuse = initColor(0, 0, 0);
    specular = initColor(0, 0, 0);
    shininess = 0.0;

    colorByFace = NULL;
    vertexesWorld = NULL;
}

Object::Object() {
    init();
}

Object::Object(string filename) {
    init();
    processFile(filename);
}

Object Object::copy() {
    Object copy;
    copy.name = name;
    copy.vertexes = vertexes;
    copy.normals = normals;
    copy.faces = faces;

    copy.ambient = ambient;
    copy.diffuse = diffuse;
    copy.specular = specular;
    copy.shininess = shininess;
    return copy;
}

void Object::processFile(string filename) {
    if (filename.find(".obj") == -1) {
        throw invalid_argument("File " + filename + " needs to be a .obj file.");
    }
    
    string buffer;
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    if (file.fail()) {
        string msg = "Could not read obj file '" + filename + "'.";
        throw invalid_argument(msg);
    }

    if (name.size() == 0) {
        filename.erase(filename.find('.'));
        name = filename;
    }
    
    vector<string> element;
    while (getline(file, buffer)) {
        element.clear();
        splitBySpace(buffer, element);
        if (element[0] == "v") {
            vertex_t v = initVertex(stod(element[1]), stod(element[2]), stod(element[3]));
            vertexes.push_back(v);
            continue;
        } else if (element[0] == "vn") {
            surf_norm_t sn = initSurfNorm(stod(element[1]), stod(element[2]), stod(element[3]));
            normals.push_back(sn);
            continue;
        }

        int v1, sn1, v2, sn2, v3, sn3;
        string element_str;

        element_str = element[1];
        element[1].erase(element[1].find("//"));
        v1 = stoi(element[1]);
        element_str.erase(0, element_str.find("//") + 2);
        sn1 = stoi(element_str);

        element_str = element[2];
        element[2].erase(element[2].find("//"));
        v2 = stoi(element[2]);
        element_str.erase(0, element_str.find("//") + 2);
        sn2 = stoi(element_str);

        element_str = element[3];
        element[3].erase(element[3].find("//"));
        v3 = stoi(element[3]);
        element_str.erase(0, element_str.find("//") + 2);
        sn3 = stoi(element_str);

        face_t f = initFace(v1, sn1, v2, sn2, v3, sn3);
        faces.push_back(f);
    }

    file.close();
}

void Object::printContents() {
    if (name.empty()) {
        cout << "Unnamed Object.\n";
    } else {
        cout << name << ":\n";
    }

    cout << "ambient: " << toString(ambient) << endl;
    cout << "diffuse: " << toString(diffuse) << endl;
    cout << "specular: " << toString(specular) << endl;
    cout << "Phong shininess: " << shininess << endl;

    for (size_t i = 1; i < vertexes.size(); i++) {
        vertex_t v = vertexes[i];
        cout << i << ": " << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (size_t i = 1; i < normals.size(); i++) {
        surf_norm_t sn = normals[i];
        cout << i << ": " << "vn " << sn.x << " " << sn.y << " " << sn.z << "\n";
    }

    for (size_t i = 0; i < faces.size(); i++) {
        face_t f = faces[i];
        cout << i << ": " << "f " << f.v1 << "//" << f.sn1 << " " << f.v2 << "//" << f.sn2
                 << " " << f.v3 << "//" << f.sn3 << endl;
    }

    cout << endl;
}

void Object::printPixels() {
    if (name.empty()) {
        cout << "Unnamed Object.\n";
    } else {
        cout << name << ":\n";
    }

    if (pixels.size() == 1) {
        cout << "Object has no Pixel Grid Representation [pixels is empty].\n";
        return;
    }

    for (size_t i = 1; i < pixels.size(); i++) {
        cout << "(" << pixels[i].x << ", " << pixels[i].y << ")" << endl;
    }

    cout << endl;
}

void Object::destruct() {
    if (colorByFace != NULL) {
        for (int face_idx = 0; face_idx < faces.size(); face_idx++) {
            free(colorByFace[face_idx]);
        }
        free(colorByFace);
    }
    if (vertexesWorld != NULL) {
        free(vertexesWorld);
    }
}
