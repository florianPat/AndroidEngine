#include "Shader.h"
#include "Ifstream.h"
#include "Utils.h"
#include "GLUtils.h"

String loadShader(const String& filename)
{
	Ifstream file;
	file.open(filename);
	assert(file);

	String result = LongString((uint32_t) file.getSize());
	while (!file.eof())
	{
		LongString line;
		file.getline(line);

		result += line + '\n';
	}

	return result;
}

void checkShaderError(GLuint shader, GLuint flag, bool isProgram, const String& errorMessage)
{
	GLint success = 0;
	GLchar error[1024] = {};

	if (isProgram)
	{
		CallGL(glGetProgramiv(shader, flag, &success));
	}
	else
	{
		CallGL(glGetShaderiv(shader, flag, &success));
	}

	if (success == GL_FALSE)
	{
		if (isProgram)
		{
			CallGL(glGetProgramInfoLog(shader, sizeof(error), nullptr, error));
		}
		else
		{
			CallGL(glGetShaderInfoLog(shader, sizeof(error), nullptr, error));
		}

		utils::logFBreak("", errorMessage.c_str());
		utils::logFBreak("%s", error);
		InvalidCodePath;
	}
}

GLuint createShader(const String& text, GLenum shaderType)
{
	GLuint shader = 0;
	CallGL(shader = glCreateShader(shaderType));

	if (shader == 0)
	{
		utils::logBreak("glCreateShader failed!");
	}

	const GLchar* shaderSourceStrings[1];
	GLint shaderSourceStringLengths[1];

	shaderSourceStringLengths[0] = text.length();
	shaderSourceStrings[0] = text.c_str();

	CallGL(glShaderSource(shader, 1, shaderSourceStrings, shaderSourceStringLengths));
	CallGL(glCompileShader(shader));

	checkShaderError(shader, GL_COMPILE_STATUS, false, "Error: Shader compilation failed!");

	return shader;
}

Shader::Shader(const String & filename, const Vector<ShortString>& attribLocs) : uniformCache()
{
	//NOTE: Because I add an extension, I need to make sure that it fits in a ShortString if it is one
	assert(filename.size() > 16 || filename.size() <= 12);

	GLuint shaders[NUM_SHADERS];

	CallGL(program = glCreateProgram());

	shaders[0] = createShader(loadShader(filename + ".vs"), GL_VERTEX_SHADER);
	shaders[1] = createShader(loadShader(filename + ".fs"), GL_FRAGMENT_SHADER);

	for (uint32_t i = 0; i < NUM_SHADERS; ++i)
	{
		CallGL(glAttachShader(program, shaders[i]));
	}

	for (uint32_t i = 0; i < attribLocs.size(); ++i)
	{
		CallGL(glBindAttribLocation(program, i, attribLocs[i].c_str()));
	}

	CallGL(glLinkProgram(program));
	checkShaderError(program, GL_LINK_STATUS, true, "Error: Shader program linking failed");

	CallGL(glValidateProgram(program));
	checkShaderError(program, GL_VALIDATE_STATUS, true, "Error: Shader program validating failed!");

	for (uint32_t i = 0; i < NUM_SHADERS; ++i)
	{
		CallGL(glDetachShader(program, shaders[i]));
		CallGL(glDeleteShader(shaders[i]));
	}

	bind();
}

Shader::Shader(Shader && other) : program(std::exchange(other.program, 0)), uniformCache(std::move(other.uniformCache))
{
}

Shader & Shader::operator=(Shader && rhs)
{
	this->~Shader();

	program = std::exchange(rhs.program, 0);
	uniformCache = std::move(rhs.uniformCache);

	return *this;
}

Shader::operator bool() const
{
	return (program != -1);
}

Shader::~Shader()
{
	CallGL(glDeleteProgram(program));
	program = -1;
}

void Shader::bind() const
{
	CallGL(glUseProgram(program));
}

void Shader::unbind() const
{
	CallGL(glUseProgram(0));
}

void Shader::setUniform4f(const ShortString & var, float v0, float v1, float v2, float v3)
{
	int32_t loc = getUniformLoc(var);

	CallGL(glUniform4f(loc, v0, v1, v2, v3));
}

void Shader::setUniformMat4f(const ShortString & var, const Mat4x4 & mat)
{
	int32_t loc = getUniformLoc(var);

	CallGL(glUniformMatrix4fv(loc, 1, GL_FALSE, mat.matrix));
}

int32_t Shader::getUniformLoc(const ShortString & var)
{
	auto it = uniformCache.find(var);
	if (it != uniformCache.end())
	{
		return it->second;
	}
	else
	{
		GLint loc = 0;
		CallGL(loc = glGetUniformLocation(program, var.c_str()));
		assert(loc != -1);
		uniformCache.emplace(std::make_pair<>(var, loc));
		return loc;
	}
}
