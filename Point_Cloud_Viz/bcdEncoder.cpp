#include "bcdEncoder.h"

string bcdEncoder::writeBinaryFile(string textFile, int numElements)
{
	// construct name of binary file
	size_t lastindex = textFile.find_last_of(".");
	string rawname = textFile.substr(0, lastindex);
	string binFile = rawname.append("_").append(to_string(numElements)).append(".bin");
	
	if (!exists(binFile)) {

		cout << "Writing binary file of " << textFile << " with " << numElements << " elements..." << endl;

		ifstream in(textFile);
		ofstream out(binFile, ios::out | ios::binary);

		GLfloat d;
		//GLfloat nums[6];
		int counter = 0;
		while (in >> d && counter < numElements) {
		//while (!in.eof()) {
			//cout << ++counter << endl;
			//in >> nums[0] >> nums[1] >> nums[2] >> nums[3] >> nums[4] >> nums[5];
			out.write(reinterpret_cast<const char*> (&d), sizeof(d));
			//out.write((char*)nums, 6 * sizeof(GLfloat));
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

void bcdEncoder::readBinaryFile(string binaryFile, int numElements, vector<GLfloat> &vec)
{
	// Open the stream
	std::ifstream is(binaryFile, ios::in | ios::binary);
	//fstream is;
	//is.open(binaryFile, ios::in | ios::binary | ios::trunc);

	// Determine the file length
	//is.seekg(0, ios_base::end);
	//std::size_t size = is.tellg();
	//is.seekg(0, ios_base::beg);
	// Load the data
	//is.read((char*)&vec[0], size);
	is.read((char*)&vec[0], numElements * sizeof(GLfloat));
	// Close the file
	is.close();
}

inline bool exists(string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}