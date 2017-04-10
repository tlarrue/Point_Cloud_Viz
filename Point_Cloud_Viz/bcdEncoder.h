#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

using namespace std; 

class bcdEncoder {
public:
	static string writeBinaryFile(string textFile);
	static void readBinaryFile(string binaryFile, int numElements, vector<GLfloat> &vec);
};

inline bool exists(string& name);