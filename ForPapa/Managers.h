#ifndef _MANAGERS_H_
#define _MANAGERS_H_

#include "stdafx.h"

// Adopted from http://in2gpu.com/2015/02/25/create-a-game-engine-part-i-shader-manager/

namespace Managers
{
    class ShaderManager
    {

    public:

        ShaderManager();
        ~ShaderManager();

        void CreateProgram(const std::string& shaderName,
            const std::string& VertexShaderFilename,
            const std::string& FragmentShaderFilename);

        static const GLuint GetShader(const std::string&);

        static const std::map<std::string, GLuint> GetShaders() {
            return _programs;
        }

    private:
        std::string ReadShader(const std::string& filename);
        GLuint CreateShader(GLenum shaderType,
            const std::string& source,
            const std::string& shaderName);

        static std::map<std::string, GLuint> _programs;
    };
}

#endif