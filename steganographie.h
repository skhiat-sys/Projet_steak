
#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <gdiplus.h>
#include <windows.h>

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

using namespace std;

class Steganographie {
public:

	vector<unsigned char> ReadBMP(const string& filename) {
		ifstream file(filename, ios::binary); //ouvre le fichier
		if (!file) {
			cerr << "Erreur: impossible d'ouvrir le fichier BMP.\n";
			exit(1);
		}

		vector<unsigned char> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()); //recupere la liste des bits du fichier
		file.close(); //ferme le fichier originel (guarde le data collecte)
		return data;
	}


	void WriteBMP(const string& filename, const vector<unsigned char>& data) {
		ofstream file(filename, ios::binary); //ouvre le fichier (mode binaire pour modifier le code du fichier lui meme)
		if (!file) {
			cerr << "Erreur: impossible d'écrire le fichier BMP.\n";
			exit(1);
		}

		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		file.close();
	}


	void EmbedLSB(vector<unsigned char>& bmpData, const string& message) {
		int offset = *reinterpret_cast<int*>(&bmpData[10]);
		int dataIndex = offset;

		for (char c : message) {
			for (int i = 0; i < 8; ++i) {
				bmpData[dataIndex] &= 0xFE; // bit faible a 0 pour choix simple
				bmpData[dataIndex] |= ((c >> i) & 1); // bit faible mis comme pour le message
				dataIndex++;
				if (dataIndex >= bmpData.size()) {
					cerr << "Erreur: message trop long pour l'image!\n";
					return;
				}
			}
		}

		for (int i = 0; i < 8; ++i) {
			bmpData[dataIndex] &= 0xFE;
			dataIndex++;
		}
	}


	string ExtractLSB(const vector<unsigned char>& bmpData) {
		int offset = *reinterpret_cast<const int*>(&bmpData[10]);
		int dataIndex = offset;
		string message;
		char c = 0;
		int bitCount = 0;

		while (dataIndex < bmpData.size()) {
			c |= ((bmpData[dataIndex] & 1) << bitCount);
			bitCount++;
			dataIndex++;
			if (bitCount == 8) {
				if (c == 0) break; // fin du message
				message += c;
				c = 0;
				bitCount = 0;
			}
		}

		return message;
	}

};
