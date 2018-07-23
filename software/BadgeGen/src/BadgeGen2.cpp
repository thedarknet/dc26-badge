//============================================================================
// Name        : BadgeGen2.cpp
// Author      : CmdC0de
// Version     :
// Copyright   : DCDarkNet Industries LLC  all right reserved
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "sha256.h"
#include <uECC.h>
#include <memory.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <set>
#include <cstdlib>


using namespace std;

uECC_Curve theCurve = uECC_secp192r1();
#define PRIVATE_KEY_SIZE = 24;
#define PUBLIC_KEY = 48;
#define COMPRESSED_PUBLIC_KEY = 25;
#define SIGNATURE_SIZE = 48;
#define BYTES_OF_SIGNATURE_TO_USE = 16;

bool makeKey(uint8_t privKey[24], uint8_t pubKey[48], uint8_t compressPub[26]) {
	memset(&privKey[0], 0, 24);
	memset(&pubKey[0], 0, 48);
	memset(&compressPub[0], 0, 26);
	if (uECC_make_key(pubKey, privKey, theCurve) == 1) {

		uECC_compress(pubKey, compressPub, theCurve);

		if (uECC_valid_public_key(pubKey, theCurve) == 1) {
			return true;
		}
	}
	return false;
}

void printKeys(uint8_t privKey[24], uint8_t pubKey[26]) {
	cout << "PrivateKey:" << endl;
	cout << "\t";
	for (int i = 0; i < 24; i++) {
		if (i != 0) {
			cout << ":";
		}
		cout << setfill('0') << setw(2) << hex << (int) privKey[i] << dec;
	}
	cout << endl;
	cout << "PublicKey:" << endl;
	cout << "\t";
	for (int i = 0; i < 26; i++) {
		if (i != 0) {
			cout << ":";
		}
		cout << setfill('0') << setw(2) << hex << (int) pubKey[i] << dec;
	}
	cout << endl;
}

bool exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void usage() {
	cout
	<< "BadgeGen -u <make uber init file> -c <create daemon keys> -n <number of badge keys to generate>"
	<< endl;
}

void loadDups(const std::string &dupFile, std::set<int> &dups) {
	std::ifstream in(dupFile.c_str());
	while(in.good()) {
		char buf[256];
		in.getline(&buf[0],sizeof(buf));
		int i = atoi(&buf[0]);
		dups.insert(i);
	}
}

int main(int argc, char *argv[]) {
	char create = 0, generate = 0, makeUber = 0;

	uint8_t privateKey[24] = { 0x00 };
	uint8_t unCompressPubKey[48] = { 0x00 };
	uint8_t compressPubKey[26] = { 0x00 }; //only need 25
	uint8_t RadioID[2];
	std::string dupFile;

	int ch = 0;
	int numberToGen = 0;

	while ((ch = getopt(argc, argv, "ucn:d:")) != -1) {
		switch (ch) {
		case 'c':
			create = 1;
			break;
		case 'n':
			numberToGen = atoi(optarg);
			generate = 1;
			break;
		case 'd':
			dupFile = optarg;
			break;
		case 'u':
			makeUber = 1;
			break;
		case '?':
		default:
			usage();
			return -1;
			break;
		}
	}

	if (1 == create) {
		if (makeKey(privateKey, unCompressPubKey, compressPubKey)) {
			printKeys(privateKey, compressPubKey);
		} else {
			cerr << "Error generating key" << endl;
		}
	} else if (1 == generate) {
		std::ofstream sqlFile("badge-info.sql");
		std::set<int> DuplicateSet;
		loadDups(dupFile,DuplicateSet);
		for (int i = 0; i < numberToGen; i++) {
			uECC_RNG_Function f = uECC_get_rng();
			f(&RadioID[0], 2);
			unsigned short rid = (*(unsigned short *)&RadioID[0]);
			if(DuplicateSet.find(rid)!=DuplicateSet.end()) {
				--i;
				cout << "duplicated unique id generated from years past...skipping " << rid << endl;
				continue;
			}
			if (makeKey(privateKey, unCompressPubKey, compressPubKey)) {
				cout << "RadioID: " << endl;
				cout << "\t" << setfill('0') << setw(2) << hex << (int) RadioID[0] << dec << ":";
				cout << setfill('0') << setw(2) << hex << (int) RadioID[1] << dec << endl;
				printKeys(privateKey, compressPubKey);
				cout << endl;
				cout << endl;
				ostringstream oss;
				oss << setfill('0') << setw(2) << hex << (int) RadioID[0] << (int) RadioID[1] << ends;
				std::string fileName = oss.str();
				if (!exists("./keys")) {
					mkdir("./keys", 0700);
				}
				std::string fullFileName = "./keys/" + fileName;
				if (exists(fullFileName)) {
					numberToGen++;
				} else {
					unsigned short reserveFlags = makeUber == 1 ? 0x1 : 0x0;
					ofstream of(fullFileName.c_str());
					//                   			magic 	magic	reserved	Num Contacts 		settings 1		Settings 2
					const unsigned char magic[2] = { 0xDC, 0xDC };
					of.write((const char *) &magic[0], sizeof(magic));
					of.write((const char *) &RadioID[0], sizeof(RadioID));
					of.write((const char *) &privateKey[0], sizeof(privateKey));
					of.write((const char *) &reserveFlags, sizeof(reserveFlags));  //just zero-ing out memory
					of.flush();
					//generate registration ID
					ShaOBJ shaCtx;
					sha256_init(&shaCtx);
					sha256_add(&shaCtx,&privateKey[0],sizeof(privateKey));
					sha256_add(&shaCtx,&RadioID[0],sizeof(RadioID));
					uint8_t digest[32];
					sha256_digest(&shaCtx,digest);
					//unsigned short rid = (*(unsigned short *)&RadioID[0]);
					sqlFile << "INSERT INTO BADGE(RADIO_ID, PRIV_KEY, FLAGS, REG_KEY) VALUES (" << rid  << ",'" << std::hex;
					for(int j=0;j<24;j++) {
						sqlFile << std::setfill('0') << std::setw(2) << int(privateKey[j]);
					}
					sqlFile << "'," <<  std::dec << reserveFlags << ",'"  
					<< std::hex; 
					for(unsigned int j=0;j<sizeof(digest);j++) {
						sqlFile << std::setfill('0') << std::setw(2) << int(digest[j]);
					}
					sqlFile << std::dec << "');" << std::endl;
				}
			}
		}
	} else {
		usage();
	}
	return 0;
}
