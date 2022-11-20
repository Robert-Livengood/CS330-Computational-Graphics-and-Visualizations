#pragma once
#include "Vertex.h"
//#include <glad/glad.h>        //Uncomment if ShapeData is unused 
//#include <glad/glad.h>		//glad and glew are essentially the same thing so use one and stick to it!
#include <GL/glew.h>

struct ShapeData
{
	ShapeData() :
		vertices(0), numVertices(0),
		indices(0), numIndices(0) {}
	Vertex* vertices;
	GLuint numVertices;
	GLushort* indices;
	GLuint numIndices;
	GLsizeiptr vertexBufferSize() const
	{
		return numVertices * sizeof(Vertex);
	}
	GLsizeiptr indexBufferSize() const
	{
		return numIndices * sizeof(GLushort);
	}
	void cleanup()
	{
		delete[] vertices;
		delete[] indices;
		numVertices = numIndices = 0;
	}
};