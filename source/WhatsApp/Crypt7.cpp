#include <iostream>
#include <fstream>

#include "../AES/AES.h"
#include "../Exceptions/Exception.h"
#include "../MD5/MD5.h"
#include "Crypt5.h"
#include "Crypt7.h"

void extractKey(const std::string &keyFilename, unsigned char *key, unsigned char *iv)
{
	unsigned char *keyBytes;
	int filesize = loadFileUnsigned(keyFilename, &keyBytes);

	if (filesize != 158)
	{
		throw Exception("Expected key filesize of 158 bytes does not match.");
	}

	memcpy(iv, &keyBytes[110], 16);
	memcpy(key, &keyBytes[126], 32);

	delete[] keyBytes;
}

void decryptWhatsappDatabase7(const std::string &filename, const std::string &filenameDecrypted, unsigned char *key, unsigned char *initVector)
{
	unsigned char *fileBytes;
	int filesize = loadFileUnsigned(filename, &fileBytes);

	unsigned char iv[16];
	memcpy(iv, initVector, 16);

	const int skipBytes = 67;
	filesize -= skipBytes;
	unsigned char *databaseBytes = &fileBytes[skipBytes];

	decrypt_aes_cbc_256(databaseBytes, databaseBytes, filesize, key, iv);

	const char expectedBytes[] = "SQLite format 3";
	if (memcmp(databaseBytes, expectedBytes, sizeof(expectedBytes)) != 0)
	{
		throw Exception("Decryption failed. Wrong account name?");
	}

	std::ofstream output(filenameDecrypted.c_str(), std::ios::binary);
	if (!output)
	{
		throw Exception("Could not save decrypted WhatsApp database. Permissions needed?");
	}

	output.write(reinterpret_cast<char *>(databaseBytes), filesize);

	delete[] fileBytes;
}

void decryptWhatsappDatabase7(const std::string &filename, const std::string &filenameDecrypted, const std::string &keyFilename)
{
	unsigned char key[32];
	unsigned char iv[16];

	extractKey(keyFilename, key, iv);

	decryptWhatsappDatabase7(filename, filenameDecrypted, key, iv);
}
