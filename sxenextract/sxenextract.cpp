#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

using namespace std;

typedef unsigned char byte;

string tostring(int integer) {
	string res;
	ostringstream convert;
	convert << integer;
	return convert.str();
}

unsigned int VLQ_to_int(ifstream &pak) {
	unsigned int total = 0;

	//It is a little messy but gets the job done rather fast.
	string bits = "";
	byte b;
	do {
		b = pak.get();
		string seq = "";
		for (int i = 0; i < 7; i++) {
			seq = tostring((b & (1 << i)) != 0) + seq;
		}
		bits = bits + seq;
	} while ((b & (1 << 7)) != 0);

	for (unsigned int i = 0; i < bits.length(); i++) {
		total += pow((double)2, (double)bits.length() - (i + 1)) * (bits[i] != '0');
	}

	//cout << bits << endl;
	return total;
}

void extract_sxen_v01(ifstream &pak, string path) {
	unsigned char length;
	unsigned int fsize;
	vector<byte> file;
	string name = "";
	string cur_dir = "";
	ofstream fp;
	while (pak.eof() == false) {
		switch (pak.get()) {
			case 0x00:
				//File.
				length = pak.get();

				//Grab the Name of the file
				for (int i = 0; i < (int)length; i++) {
					name += pak.get();
				}

				//Get the length of the file (In Bytes)
				fsize = VLQ_to_int(pak);

				file.clear();
				file.resize(fsize);

				pak.read((char*) &file[0], fsize);

				fp.open(path + name, ios::binary);
				fp.write((char*) &file[0],fsize);
				fp.close();
				cout << "    Extracted " + name + "..." << endl;

				//Clear out information
				name = "";
				break;
			case 0xFF:
				//Marker.
				length = pak.get();
				string marker = "";
				for (int i = 0; i < (int)length; i++) {
					marker += pak.get();
				}
				cout << endl << "[" << marker << "]" << endl;
				cur_dir = marker;
				break;
		}
	}
}

int main() {
	string filename = "pak.song";
	string path = "";

	clock_t start, end;
	start = clock();

	ifstream pak;
	pak.open(filename, ios::binary);
	if (pak.fail()) {
		cout << "Failed to open the file." << endl; return -1;
	}
	//Carry on by skipping header for now (lazy at 2:46 AM).
	pak.seekg(0x0D);

	//Get Version Number
	unsigned int version_number = (pak.get() * 256) + pak.get();
	cout << "SXEN PAK Version " << version_number << " detected" << endl;
	switch (version_number) {
		case 1: extract_sxen_v01(pak, path); break;
		default: cout << "This extractor can not read this format (Perhaps too new?)" << endl; break;
	}
	pak.close();

	end = clock();
	cout << endl << "File Extraction complete (" << (double)(end - start)/CLOCKS_PER_SEC << "s" << ")." << endl;

	getchar();
}