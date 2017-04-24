#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <iterator>
#include <iomanip>

//#include <iomanip>
//#include <boost/spirit/include/qi.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

//namespace qi = boost::spirit::qi;

const std::string POS_STR("positions");
const std::string COL_STR("colors");

/*
Class to read point cloud data into memory
*/
class pcReader {

public:

	struct pcInfo {
		int points; // total number of points
		std::string textFile; // absolute path of ascii data
		std::map<std::string, int> attrSizes; //attribute name, number of coordinates
		std::map<std::string, std::string> binFiles; // attribute name, absolute path of binary file
		std::map<std::string, int> dataTypes; // attribute name, OpenGL datatype [float=4, half=2, glubyte=1]
		std::vector<char> format; // coordinate names (should be length of width)
	};

	static pcInfo parseText(std::string textFile, bool overwriteBinaries);

	//static void writeBinary(pcInfo* info, std::string attribute, bool overwrite);
	static void writeColorBinary(pcInfo* info, bool overwrite);
	static void writePositionBinary(pcInfo* info, bool overwrite);

	static void readColors(pcInfo* info, std::vector<GLubyte> &vec);
	static void readColors(pcInfo* info, std::vector<GLint> &vec);
	static void readPositions(pcInfo* info, std::vector<GLfloat> &vec);
	static void readPositions(pcInfo* info, std::vector<GLhalf> &vec);

};

inline bool exists(std::string& name);
inline bool isPositionCoord(char x);
inline bool isColorCoord(char x);
inline std::string getBinaryFile(std::string textFile, std::string attribute);