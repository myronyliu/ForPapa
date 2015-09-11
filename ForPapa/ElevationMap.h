#ifndef _ELEVATIONMAP_H_
#define _ELEVATIONMAP_H_

#include "stdafx.h"

class ElevationMap // I'm just going to lump everything in here (camera and all) for convenience
{
public:

    GLuint _shader;

    enum { PERSPECTIVE = 0, ORTHOGRAPHIC };

    int _camType;

    float _fovy; // perpective params
    float _aspect;

    float _left; // orthographic params
    float _right;
    float _bottom;
    float _top;

    ////////////////////////////////
    //// BEGIN MEMBER FUNCTIONS ////
    ////////////////////////////////

	ElevationMap(int nx, int ny, float dx = 10.0f, float dy = 10.0f);
    ~ElevationMap();

    bool load(const char* fileName); // I don't know the file format, so you can fill this in
    void randomize(float hMin, float hMax);
    void updateBuffers(int padding = -1);
    void draw() const;

    void panCamera(float dTheta, float dPhi);
    void shiftCamera(const glm::vec3& dCamX); // shifts according to GLOBAL axes
    void moveCamera(const glm::vec3& step); // moves according to the camera's own axes
    void toggleCameraType();
private:
    unsigned int _nx;
    unsigned int _ny;
    float _dx;
    float _dy;
    float* _h;
    glm::vec3 _w; // Some Euclidean transform parameters to make your life easier (in case (0,1,0) isn't the up direction)
    glm::vec3 _t;

    GLuint _VAO;
    GLuint _pVBO;
    GLuint _nVBO;
    GLuint _IBO;

    ///////////////////////////////////////////////////////////
    //// modified spherical coordinates for camera control ////
    ///////////////////////////////////////////////////////////

    // _theta corresponds to rotation about the GLOBAL NEGATIVE x-axis
    // _phi is just rotation about the GLOBAL y-axis
    // The default view [theta,phi] = [0,0] looks along the z-axis with y-axis pointing up

    float _theta;
    float _phi;
    glm::vec3 _camX; // The position of the camera

    float _near; // clipping params
    float _far;

    void clampCamera();

    ////////////////////////////////
    //// point light parameters ////
    ////////////////////////////////

    glm::vec3 _lightDir; // the (normalized) vector pointing to the lightSource [0,1,0] for "high noon sunlight"
    glm::vec3 _lightRad; // the radiance of the light (wattage doesn't mean much for infinitely distant lights)

    unsigned int _nTriangles;
};

#endif