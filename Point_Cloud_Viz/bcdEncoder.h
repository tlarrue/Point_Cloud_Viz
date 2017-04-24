#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

using namespace std; 

struct pcBinFiles {
	string posFileName;
	string colFileName;
	int posFileSize;
	int colFileSize;
};

class bcdEncoder {
public:
	static pcBinFiles writeBinaryFiles(string textFile, bool overwrite);
	static void readBinaryFile(string binaryFile, int numElements, vector<GLfloat> &vec);
};

inline bool exists(string& name);