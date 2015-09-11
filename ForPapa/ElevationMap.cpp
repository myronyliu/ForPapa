#include "ElevationMap.h"

ElevationMap::ElevationMap(int nx, int ny, float dx, float dy) :
_nx(std::max({ 2, nx })), _ny(std::max({ 2, ny })), _dx(fabs(dx)), _dy(fabs(dy)),
_theta(-M_PI / 6), _phi(0.0f),
_camType(PERSPECTIVE), _fovy(M_PI / 4), _aspect(4.0f / 3),
_near(0.125f), _far(1024.0f),
_left(-400.0f), _right(400.0f), _bottom(-300.0f), _top(300.0f),
_lightDir(glm::vec3(0, 1, 0)), _lightRad(0.5f*glm::vec3(1, 1, 1))
{
    _h = new float[_nx*_ny];

    float s = (float)sqrt(_nx*_nx + _ny*_ny) / 2;
    _camX = glm::vec3(_dy*_ny / 2, s, _dx*_nx*1.7f);

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_pVBO);
    glGenBuffers(1, &_nVBO);
    glGenBuffers(1, &_IBO);
}

ElevationMap::~ElevationMap()
{
    delete[] _h;

    glDeleteVertexArrays(1, &_VAO);
    glDeleteBuffers(1, &_pVBO);
    glDeleteBuffers(1, &_nVBO);
    glDeleteBuffers(1, &_IBO);
}

void ElevationMap::randomize(float hMin, float hMax)
{
    for (unsigned int i = 0; i < _nx; i++)
    {
        for (unsigned int j = 0; j < _ny; j++)
        {
            float alpha = (float)rand() / RAND_MAX;
            _h[_nx*i + j] = alpha*hMin + (1.0f - alpha)*hMax;
        }
    }
    updateBuffers();
}

void ElevationMap::clampCamera()
{
    _theta = (_theta < -M_PI / 2) ? -M_PI / 2 : (_theta > M_PI / 2) ? M_PI / 2 : _theta;
    _phi -= floor(_phi / (2 * M_PI)) * (2 * M_PI);
}

void ElevationMap::panCamera(float dTheta, float dPhi)
{
    _theta += dTheta;
    _phi += dPhi;
    clampCamera();
}

void ElevationMap::shiftCamera(const glm::vec3& dCamX)
{
    _camX += dCamX;
}   

void ElevationMap::moveCamera(const glm::vec3& step)
{
    glm::mat3 R = glm::mat3(glm::rotate(_phi, glm::vec3(0, 1, 0))*glm::rotate(_theta, glm::vec3(1, 0, 0)));

    _camX += R*glm::vec3(step[0], step[1], -step[2]);
}

void ElevationMap::toggleCameraType()
{
    if (_camType == PERSPECTIVE)
        _camType = ORTHOGRAPHIC;
    else if (_camType == ORTHOGRAPHIC)
        _camType = PERSPECTIVE;
    else
        _camType = PERSPECTIVE;
}

void ElevationMap::updateBuffers(int paddingIn) // call this function whenever you modify the elevations
{
    unsigned int nx = _nx;
    unsigned int ny = _ny;

    float* h = _h;

    if (paddingIn != 0)
    {
        unsigned int padding = paddingIn;

        if (paddingIn < 0)
        {
            padding = (unsigned int)fmax(1.0f, sqrt(_nx*_ny) / 8);
        }
        nx = _nx + 2 * padding;
        ny = _ny + 2 * padding;

        h = new float[nx*ny];

        int counter = 0;
        for (unsigned int i = 0; i < padding*nx; i++, counter++)
        {
            h[counter] = 0;
        }
        for (unsigned int i = 0; i < _nx; i++)
        {
            for (unsigned int j = 0; j < padding; j++, counter++)
            {
                h[counter] = 0;
            }
            for (unsigned int j = 0; j < _ny; j++, counter++)
            {
                h[counter] = _h[_nx*i + j];
            }
            for (unsigned int j = 0; j < padding; j++, counter++)
            {
                h[counter] = 0;
            }
        }
        for (unsigned int i = 0; i < padding*nx; i++, counter++)
        {
            h[counter] = 0;
        }
    }

    unsigned int nVerts = nx*ny;
    unsigned int xDivs = nx - 1;
    unsigned int yDivs = ny - 1;
    unsigned int nQuads = xDivs*yDivs;
    _nTriangles = 2 * nQuads;

    glm::vec3* positions = new glm::vec3[nVerts];
    glm::vec3* normals = new glm::vec3[nVerts];
    glm::ivec3* indices = new glm::ivec3[2 * nQuads];

    for (unsigned int i = 0; i < nx; i++) // COMPUTE POSITIONS
    {
        float x = i*_dx;

        for (unsigned int j = 0; j < ny; j++)
        {
            float y = j*_dy;

            positions[i + nx*j] = glm::vec3(y, h[i + nx*j], x);
        }
    }

    if (h != _h)
    {
        delete[] h;
    }

    auto clamp = [](int a, int b, int c)
    {
        if (b < a)
            return a;
        if (b > c)
            return c;
        return b;
    };

    for (unsigned int i = 0; i < nx; i++) // COMPUTE NORMALS
    {
        unsigned int iM = clamp(0, i - 1, nx);
        unsigned int iP = clamp(0, i + 1, nx);

        for (unsigned int j = 0; j < ny; j++)
        {
            unsigned int jM = clamp(0, j - 1, ny);
            unsigned int jP = clamp(0, j + 1, ny);

            glm::vec3 gx = positions[iP + nx*j] - positions[iM + nx*j];
            glm::vec3 gy = positions[i + nx*jP] - positions[i + nx*jM];

            normals[i + nx*j] = glm::normalize(glm::cross(gx, gy));
        }
    }

    for (unsigned int i = 0; i < xDivs; i++) // COMPUTE THE TRIANGLE INDICES
    {
        for (unsigned int j = 0; j < yDivs; j++)
        {
            unsigned int q = xDivs*i + j;

            unsigned int v00 = i + nx*j;
            unsigned int v10 = (i + 1) + nx*j;
            unsigned int v11 = (i + 1) + nx*(j + 1);
            unsigned int v01 = i + nx*(j + 1);

            indices[2 * q + 0] = glm::ivec3(v00, v10, v11);
            indices[2 * q + 1] = glm::ivec3(v11, v01, v00);
        }
    }

    glUseProgram(_shader); // I think you can actually forgo this if you use "layouts" but I prefer grabbing the attrib names instead

    // Tell OpenGL which vertex array object we would like to hold our vertex buffer objects in
    glBindVertexArray(_VAO);

    // Pass in PER-VERTEX attributes as buffer elements of _VAO
    glBindBuffer(GL_ARRAY_BUFFER, _pVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*nVerts, &positions[0], GL_STATIC_DRAW);
    GLint in_position = glGetAttribLocation(_shader, "in_position");
    glEnableVertexAttribArray(in_position);
    glVertexAttribPointer(in_position, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, _nVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*nVerts, &normals[0], GL_STATIC_DRAW);
    GLint in_normal = glGetAttribLocation(_shader, "in_normal");
    glEnableVertexAttribArray(in_normal);
    glVertexAttribPointer(in_normal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // Pass in triangle indices. Each ivec3 holds the indices for one triangle
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * 2 * nQuads, &indices[0], GL_STATIC_DRAW);

    // Unbind everything. Note the order of operations: i.e. unbind the VAO before unbinding the VBOs
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(0);

    delete[] positions;
    delete[] normals;
    delete[] indices;
}

void ElevationMap::draw() const
{
    glUseProgram(_shader);

    // Pass in the MODEL MATRIX
    glm::mat4 MM = glm::translate(_t);
    float angle = glm::length(_w);
    if (angle > 0)
    {
        MM = MM*glm::rotate(angle, _w / angle);
    }
    GLint MMLoc = glGetUniformLocation(_shader, "modelMatrix");
    glUniformMatrix4fv(MMLoc, 1, false, &MM[0][0]);

    // Pass in the camera VIEW MATRIX
    glm::mat4 camMM =
        glm::translate(_camX)*
        glm::rotate(_phi, glm::vec3(0, 1, 0))*
        glm::rotate(_theta, glm::vec3(1, 0, 0));
    glm::mat4 VM = glm::inverse(camMM);
    GLint VMLoc = glGetUniformLocation(_shader, "viewMatrix");
    glUniformMatrix4fv(VMLoc, 1, false, &VM[0][0]);

    // Pass in the PROJECTION MATRIX
    glm::mat4 PM;
    if (_camType == PERSPECTIVE)
        PM = glm::perspective(_fovy, _aspect, _near, _far);
    else if (_camType == ORTHOGRAPHIC)
        PM = glm::ortho(_left, _right, _bottom, _top, _near, _far);
    GLint PMLoc = glGetUniformLocation(_shader, "projectionMatrix");
    glUniformMatrix4fv(PMLoc, 1, false, &PM[0][0]);
    
    // Pass in the LIGHTING parameters
    GLint loc = glGetUniformLocation(_shader, "lightDir");
    glUniform3fv(loc, 1, &_lightDir[0]);
    loc = glGetUniformLocation(_shader, "lightRad");
    glUniform3fv(loc, 1, &_lightRad[0]);

    // Perform the actual drawing
    glBindVertexArray(_VAO);
    glDrawElements(GL_TRIANGLES, 3 * _nTriangles, GL_UNSIGNED_INT, 0);

    // Reset the OpenGL state to default
    glBindVertexArray(0);
    glUseProgram(0);
}