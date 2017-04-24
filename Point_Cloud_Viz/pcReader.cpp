#include "pcReader.h"
#include "ProgressBar.h"


/*
Parses ascii point cloud data into a pcInfo object. 
Writes a binary file for each attribute if they do not exist.
*/
pcReader::pcInfo pcReader::parseText(std::string textFile, bool overwriteBinaries) {

	pcReader::pcInfo info;
	info.textFile = textFile;
	std::string length("POINTS");
	std::string format("FORMAT");

	// find header file
	size_t lastindex = textFile.find_last_of(".");
	std::string rawname = textFile.substr(0, lastindex);
	std::string hdrFile = rawname.append(".hdr.txt");

	// loop through lines of header file & parse fields into pcInfo struct
	if (exists(hdrFile)) {

		std::ifstream in(hdrFile);
		std::string line;

		while (std::getline(in, line)) {

			std::istringstream iss(line);
			std::string fieldName, fieldValue;

			if (!(iss >> fieldName)) {
				throw std::runtime_error("Error reading header\n");
			}

			size_t lastindex = fieldName.find_last_of(":");
			std::string rawname = fieldName.substr(0, lastindex);

			if (!length.compare(0, 6, rawname, 0, 6)) {
				
				iss >> fieldValue; 
				info.points = atoi(fieldValue.c_str());
			}

			if (!format.compare(0, 6, rawname, 0, 6)) {

				char attr;
				while (iss >> attr) {
					info.format.push_back(attr);
				}

			}
		}
	}
	else {
		throw std::runtime_error("Header file does not exist.\n");
	}

	// count number of coordinates per attribute
	int positions = 0, colors = 0; // attribute types

	for (char & i : info.format) {
		if (isPositionCoord(i)) {
			positions += 1;
		}
		else if (isColorCoord(i)) {
			colors += 1;
		}
	}
	info.attrSizes[POS_STR] = positions;
	info.attrSizes[COL_STR] = colors;

	// write a binary file for each coordinate
	if (colors > 0) {
		//writeBinary(&info, COL_STR, overwriteBinaries);
		writeColorBinary(&info, overwriteBinaries);
	}
	if (positions > 0) {
		writePositionBinary(&info, overwriteBinaries);
	}
	

	return info;

}



void pcReader::writeColorBinary(pcInfo* info, bool overwrite) {
	
	// unpack pcinfo
	int numPoints = info->points;
	
	int numCoords = info->attrSizes[COL_STR];
	std::string textFile = info->textFile;

	// construct name of binary file
	std::string binFile = getBinaryFile(textFile, COL_STR);
	info->binFiles[COL_STR] = binFile;

	// check if binary files already exist
	if ((!exists(binFile)) || overwrite) {

		// define in and out streams
		std::cout << "Writing binary file: " << binFile << std::endl;
		//ProgressBar::Begin(numPoints*numCoords, "Binary Color Write Progress");

		std::ofstream out(binFile, std::ios::out | std::ios::binary);
		out.unsetf(std::ios::skipws);
		std::ifstream in(textFile);

		if (!in || !out) {
			std::runtime_error("Cannot open data file.\n");
		}

		// parse text data into a GLubyte vector
		std::string line;
		std::vector<GLubyte> vec(numPoints * numCoords);
		GLfloat temp;
		int pt = 0, coord = 0;

		int totalCoords = info->format.size();
		std::cout << numPoints*numCoords << " " << totalCoords << std::endl; 

		while (in >> temp) {

			if (isColorCoord(info->format[coord])) {
				vec[pt] = (GLubyte)temp;

				if (pt > ((numPoints*numCoords) - 6)) {
					std::cout << temp << " " << vec[pt] << std::endl;
				}

				pt++;

				
			}

			if (coord == totalCoords - 1) {
				coord = 0;
			}
			else {
				coord++;
			}

			
			
			//ProgressBar::Show(pt, "Binary Color Write Progress");
		}

		std::cout << "pt " << pt << std::endl;

		int datasize = numPoints * numCoords * sizeof(GLubyte); 
		out.write((char*)&vec[0], datasize);

		out.close();
		in.close();

		info->dataTypes[COL_STR] = 1;

		std::cout << "New binary file created: " << binFile << std::endl;
		std::cout << "Data Size: " << (float)datasize / 1000000000.f << " GB" << std::endl;
	}
	else {
		std::cout << "Binary file already exists: " << binFile << std::endl;
	}
	
}

void pcReader::writePositionBinary(pcInfo* info, bool overwrite)
{

	// unpack pcinfo & attribute vars
	int numPoints = info->points;
	int numCoords = info->attrSizes[POS_STR];
	std::string textFile = info->textFile;

	// construct name of binary file
	std::string binFile = getBinaryFile(textFile, POS_STR);
	info->binFiles[POS_STR] = binFile;

	// check if binary files already exist
	if ((!exists(binFile)) || overwrite) {

		// define in and out streams
		std::cout << "Writing binary file: " << binFile << std::endl;
		//ProgressBar::Begin(numPoints*numCoords, "Binary Position Write Progress");

		std::ofstream out(binFile, std::ios::out | std::ios::binary);
		std::ifstream in(textFile);

		if (!in || !out) {
			std::runtime_error("Cannot open data file.\n");
		}

		// parse text data into a GLfloat vector
		std::string line;
		std::vector<GLfloat> vec(numPoints * numCoords);
		bool halfOK = true;
		GLfloat temp;
		GLhalf half;
		int pt = 0, coord = 0;

		int totalCoords = info->format.size();

		while (in >> temp) {

			if (isPositionCoord(info->format[coord])) {
				vec[pt] = temp;
				if (halfOK) {
					half = static_cast<GLhalf>(vec[pt]);
					halfOK = vec[pt] == static_cast<GLfloat>(half);
				}
				pt++;
			}

			if (coord == totalCoords - 1) {
				coord = 0;
			}
			else {
				coord++;
			}

			//ProgressBar::Show(pt, "Binary Color Write Progress");
		}

		// analyze data types & write to binary file in smallest storage
		int datasize = numPoints * numCoords;
		if (halfOK) {
			std::cout << "Using GLhalf datatype for position attribute." << std::endl;
			info->dataTypes[POS_STR] = 2;
			std::vector<GLhalf> halfVec(vec.begin(), vec.end());
			std::vector<GLfloat>().swap(vec);
			datasize *= sizeof(GLhalf);
			out.write((char*)&halfVec[0], datasize);
		}
		else {
			info->dataTypes[POS_STR] = 4;
			datasize *= sizeof(GLfloat);
			out.write((char*)&vec[0], datasize);
		}

		out.close();
		in.close();

		std::cout << "New binary file created: " << binFile << std::endl;
		std::cout << "Data Size: " << (float)datasize / 1000000000.f << " GB" << std::endl;

	}
	else {
		std::cout << "Binary file already exists: " << binFile << std::endl;

	}

}
/*
void pcReader::writeBinary(pcInfo* info, std::string attribute, bool overwrite)
{

	// unpack pcinfo & attribute vars
	bool colAttr = false, posAttr = false;
	int numPoints = info->points;
	int numCoords = info->attrSizes[attribute];
	if (attribute == POS_STR)
		posAttr = true;
	else if (attribute == COL_STR)
		colAttr = true;
	else
		std::runtime_error("Attribute not valid.\n");

	// construct name of binary file
	size_t lastindex = info->textFile.find_last_of(".");
	std::string rawname = info->textFile.substr(0, lastindex);
	std::string binFile = rawname.append("_").append(attribute).append(".bin");
	info->binFiles[attribute] = binFile;

	// check if binary files already exist
	if ((!exists(binFile)) || overwrite) {

		// define in and out streams
		std::cout << "Writing binary file: " << binFile << std::endl;
		std::ofstream out(binFile, std::ios::out | std::ios::binary);
		std::ifstream in(info->textFile);

		if (!in || !out) {
			std::runtime_error("Cannot open data file.\n");
		}

		// parse text data into a GLfloat vector
		std::string line;
		std::vector<GLfloat> vec(numPoints * numCoords);
		bool halfOK = true;
		GLfloat temp;
		GLhalf half;
		int pt = 0, coord = 0;

		while (std::getline(in, line)) {

			std::istringstream iss(line);
			coord = 0;

			while (iss >> temp) {

				if (coord > numCoords - 1)
					break;

				if (posAttr && isPositionCoord(info->format[coord])) {
					vec[pt] = temp;
					if (halfOK) {
						half = static_cast<GLhalf>(vec[pt]);
						halfOK = vec[pt] == static_cast<GLfloat>(half);
					}
					coord++;
					pt++;

				}
				else if (colAttr && isColorCoord(info->format[coord])) {
					vec[pt] = temp;
					coord++;
					pt++;

				}

			}

		}

		// analyze data types & write to binary file in smallest storage
		int datasize = numPoints * numCoords;
		if (posAttr) {
			if (halfOK) {
				info->dataTypes[attribute] = 2;
				std::vector<GLhalf> halfVec(vec.begin(), vec.end());
				datasize *= sizeof(GLhalf);
				out.write(reinterpret_cast<const char*>(&halfVec[0]), datasize);
			}
			else {
				info->dataTypes[attribute] = 4;
				datasize *= sizeof(GLfloat);
				out.write(reinterpret_cast<const char*>(&vec[0]), datasize);
			}

		}
		else if (colAttr) {
			info->dataTypes[attribute] = 1;
			std::vector<GLubyte> byteVec(vec.begin(), vec.end());
			datasize *= sizeof(GLubyte);
			out.write(reinterpret_cast<const char*>(&byteVec[0]), datasize);
		}

		out.close();
		in.close();
		
		std::cout << "New binary file created: " << binFile << std::endl;
		std::cout << "Data Size: " << (float)datasize / 1000000000.f << " GB" << std::endl;

	}
	else {
		std::cout << "Binary file already exists: " << binFile << std::endl;

	}

}
*/

void pcReader::readColors(pcInfo* info, std::vector<GLint> &vec) {

	// get name of binary file
	std::string binFile = info->binFiles[COL_STR];
	std::cout << "Reading Color Data from: " << binFile << std::endl;

	// Open the stream
	std::ifstream is(binFile, std::ios::in | std::ios::binary);

	// Stop eating new lines in binary mode
	is.unsetf(std::ios::skipws);

	is.read(reinterpret_cast<char*>(&vec[0]), info->points * info->attrSizes[COL_STR]);

	//std::vector<int> test(info->points * info->attrSizes[COL_STR], 0);


	is.close();
}


void pcReader::readColors(pcInfo* info, std::vector<GLubyte> &vec) {

	// get name of binary file
	std::string binFile = info->binFiles[COL_STR];
	std::cout << "Reading Color Data from: " << binFile << std::endl;

	// Open the stream
	std::ifstream is(binFile, std::ios::in | std::ios::binary);

	// Stop eating new lines in binary mode
	is.unsetf(std::ios::skipws);

	is.read(reinterpret_cast<char*>(&vec[0]), info->points * info->attrSizes[COL_STR]);

	//std::vector<int> test(info->points * info->attrSizes[COL_STR], 0);


	is.close();
}

void pcReader::readPositions(pcInfo* info, std::vector<GLfloat> &vec) {

	

	// get name of binary file
	std::string binFile = info->binFiles[POS_STR];
	std::cout << "Reading Position Data from: " << binFile << std::endl;

	// Open the stream
	std::ifstream is(binFile, std::ios::in | std::ios::binary);

	// Load the data
	is.read((char*)&vec[0], info->points * info->attrSizes[POS_STR] *sizeof(GLfloat));
	// Close the file
	is.close();
}

void pcReader::readPositions(pcInfo* info, std::vector<GLhalf> &vec) {

	// get name of binary file
	std::string binFile = info->binFiles[POS_STR];
	std::cout << "Reading Position Data from: " << binFile << std::endl;

	// Open the stream
	std::ifstream is(binFile, std::ios::in | std::ios::binary);

	// Load the data
	is.read((char*)&vec[0], info->points * info->attrSizes[POS_STR] * sizeof(GLhalf));

	// Close the file
	is.close();
}

/*
Determines if a file exists.
*/
inline bool exists(std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

inline bool isPositionCoord(char x) {
	if (x == 'X' || x == 'x' || x == 'Y' || x == 'y' || x == 'Z' || x == 'z') {
		return true;
	}
	else {
		return false;
	}
}

inline bool isColorCoord(char x) {
	if (x == 'R' || x == 'r' || x == 'G' || x == 'g' || x == 'B' || x == 'b') {
		return true;
	}
	else {
		return false;
	}
}

/*
Construct name of binary file based on name of ascii file & attribute
*/
inline std::string getBinaryFile(std::string textFile, std::string attribute) {
	size_t lastindex = textFile.find_last_of(".");
	std::string rawname = textFile.substr(0, lastindex);
	std::string binFile = rawname.append("_").append(attribute).append(".bin");

	return binFile;
}
