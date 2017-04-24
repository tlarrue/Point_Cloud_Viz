#include "bcdEncoder.h"

/*
Converts a text file into a binary file & saves it
*/
/*
string bcdEncoder::writeBinaryFile(string textFile)
{
	// construct name of binary file
	size_t lastindex = textFile.find_last_of(".");
	string rawname = textFile.substr(0, lastindex);
	string binFile = rawname.append(".bin");
	
	if (!exists(binFile)) {

		cout << "Writing binary file of " << textFile << endl;

		ifstream in(textFile);
		ofstream out(binFile, ios::out | ios::binary);

		GLfloat d;
		while (in >> d) {
			out.write(reinterpret_cast<const char*> (&d), sizeof(d));
		}

		out.close();

		cout << "Created binary file: " << binFile << endl;
		return binFile;
	}
	else {
		cout << "Binary file already exists: " << binFile << endl;
		return binFile;
	}
	
}
*/

/*
Converts a X-Y-Z-R-G-B text file into a binary files in the form
 X-Y-Z & saves it
*/
pcBinFiles bcdEncoder::writeBinaryFiles(string textFile, bool overwrite)
{
	// construct name of binary file
	size_t lastindex = textFile.find_last_of(".");
	string rawname = textFile.substr(0, lastindex);
	string posFileName = rawname.append("_positions.bin");
	string colFileName = rawname.append("_colors.bin");

	// declare vars
	bool writePos, writeCol = false;
	ofstream posOut, colOut;
	pcBinFiles* binFiles;
	binFiles->posFileName = posFileName;
	binFiles->colFileName = posFileName;

	// check if binary files already exist
	if (!exists(posFileName) || (exists(posFileName) && overwrite)) {
		writePos = true;
		std::cout << "Writing binary file: " << posFileName << endl;
		posOut.open(posFileName, ios::out | ios::binary);
	}
	else {
		std::cout << "Binary file already exists: " << posFileName << endl;
	}

	if (!exists(colFileName) || (exists(colFileName) && overwrite)) {
		writeCol = true;
		std::cout << "Writing binary file: " << colFileName << endl;
		colOut.open(colFileName, ios::out | ios::binary);
	}
	else {
		std::cout << "Binary file already exists: " << colFileName << endl;
	}

	// parse text files if 1 or more binary files do not exist
	if (writePos || writeCol){

		ifstream in(textFile);
		GLfloat pos[3];
		GLubyte col[3];

		// write data size per point as first line
		GLubyte posDsize = sizeof(GLfloat);
		posOut.write(reinterpret_cast<const char*> (&posDsize), sizeof(GLubyte));
		//int linesPos = posOut.tellg();

		int numLines = 0;
		while (!in.eof()) {
			in >> pos[0] >> pos[1] >> pos[2] >> col[0] >> col[1] >> col[2];

			if (writePos)
				posOut.write(reinterpret_cast<const char*> (pos), 3 * sizeof(GLfloat));
			if (writeCol)
				colOut.write(reinterpret_cast<const char*> (col), 3 * sizeof(GLubyte));

			++numLines;
		}

		if (writePos) {
			posOut.close();
			std::cout << "Created binary file: " << posFileName << endl;
			binFiles->posFileSize = numLines * 3 * sizeof(GLfloat);
		}
			
		if (writeCol) {
			colOut.close();
			std::cout << "Created binary file: " << colFileName << endl;
			binFiles->colFileSize = numLines * 3 * sizeof(GLubyte);
		}
		
	}

	return *binFiles;

}


/*
Converts a X-Y-Z-R-G-B text file into two binary files in the form
R-G-B and X-Y-Z & saves it
*/
string bcdEncoder::writeBinaryFile(string textFile)
{
	// construct name of binary file
	size_t lastindex = textFile.find_last_of(".");
	string rawname = textFile.substr(0, lastindex);
	string posFile = rawname.append("_positions.bin");
	string colFile = rawname.append("_colors.bin");

	if (!exists(posFile) || !exists(colFile)) {

		//cout << "Writing binary file of " << textFile << endl;

		ifstream in(textFile);
		if (!exists(posFile)) {
			ofstream posOut(posFile, ios::out | ios::binary);
			cout << "Writing binary file: " << posFile << endl;
		}
		
		if (!exists(colFile)) {
			ofstream colOut(posFile, ios::out | ios::binary);
			ofstream colOut(posFile, ios::out | ios::binary);
		}
		

		GLfloat pos[3];
		GLubyte col[3];

		//GLfloat d;
		while (!in.eof()) {
			in >> pos[0] >> pos[1] >> pos[2] >> col[0] >> col[1] >> col[2];

			posOut.write(reinterpret_cast<const char*> (pos), 3 * sizeof(GLfloat));
			colOut.write(reinterpret_cast<const char*> (col), 3 * sizeof(GLfloat));

			//out.write(reinterpret_cast<const char*> (&d), sizeof(d));

		}

		posOut.close();
		cout << "Created binary file: " << posFile << endl;

		colOut.close();
		cout << "Created binary file: " << colFile << endl;
		
		return binFile;
	}
	else {
		cout << "Binary file already exists: " << binFile << endl;
		return binFile;
	}

}

/*
Reads a binary file into a vector fast
*/
void bcdEncoder::readBinaryFile(string binaryFile, int size, vector<GLfloat> &vec)
{
	// Open the stream
	std::ifstream is(binaryFile, ios::in | ios::binary);
	// Load the data
	is.read((char*)&vec[0], size);
	// Close the file
	is.close();
}

/*
Determines if a file exists.
*/
inline bool exists(string& name) {
	ifstream f(name.c_str());
	return f.good();
}