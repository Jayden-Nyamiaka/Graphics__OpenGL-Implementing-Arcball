#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H
 
#include <Eigen/Dense>
#include <string>
#include <stdexcept>

using Eigen::Matrix4f;

using namespace std;

Matrix4f translation(float tx, float ty, float tz);

/** 
 * Rotation about vector v = (rx, ry, rz) 
 * counterclockwise by angle rad in radians
 * 
 * Normalizes the given vector if it's not already a unit vector
  */
Matrix4f rotation(float rx, float ry, float rz, float rad);

Matrix4f scaling(float sx, float sy, float sz);

Matrix4f getIdentity4d();

[[deprecated("Matrix products from files is better handled in Wireframe::processFormatFile.")]]
/**
 * Computes the product of all transformations from 
 * a given .obj file in the form of a 4D Matrix.
 * 
 * @throws invalid_argument if the file cannot be read
 * @return the 4D matrix result
 */
Matrix4f computeGrandProduct(string filename);

#endif