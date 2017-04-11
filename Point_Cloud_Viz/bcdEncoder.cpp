#include "bcdEncoder.h"

/*
Converts a text file into a binary file & saves it
*/
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

/*
Reads a binary file into a vector fast
*/
void bcdEncoder::readBinaryFile(string binaryFile, int numElements, vector<GLfloat> &vec)
{
	// Open the stream
	std::ifstream is(binaryFile, ios::in | ios::binary);
	// Load the data
	is.read((char*)&vec[0], numElements * sizeof(GLfloat));
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