#include "main.h"


/*
Game: Ninja Gaiden
Region: NTSC-U / NTSC-J / PAL
TitleID: 0x54430003
Signature Key: A50114CA2B7C8198E829E7C937D6FC40

Save file location: \UDATA\5443003\<SaveID>\saveXXX.dat (replace XXX with 000-999)
Save file size: 0x17488 (95,368) bytes (static)

Settings file location: \TDATA\5443003\system.dat
Settings file size: 0x1B8 (440) bytes (static)

*/

/*
Game: Ninja Gaiden Black
Region: NTSC-U / NTSC-J / PAL
TitleID: 0x5443000D
Signature Key: FC3376488B3E5F00F65A6BDA9209CFE8

Alternate TitleID: 0x5443003 (for importing Ninja Gaiden saves/settings?)
Alternate Signature Key: A50114CA2B7C8198E829E7C937D6FC40 (for importing Ninja Gaiden saves/settings?)

Save file location: \UDATA\5443000D\<SaveID>\saveXXX.dat (replace XXX with 000-999)
Save file size: 0x174A8 (95,400) bytes (static)

Settings file location: \TDATA\5443000D\system.dat
Settings file size: 0x550 (1,360) bytes (static)

*/

//Ninja Gaiden
#define NG_SAVE_FILE_SIZE     0x17488 //includes hash and seed size
#define NG_SAVE_DATA_SIZE     0x17470 //actual data size
#define NG_SETTINGS_FILE_SIZE 0x1B8   //includes hash and seed size
#define NG_SETTINGS_DATA_SIZE 0x1A0   //actual data size
#define NG_XUID_OFFSET        0x1663B //offset to XUID from beginning of data start - 0x16653 is the offset from beginning of a save file (applicable to Ninja Gaiden only)
byte NG_TITLE_SIGNATURE_KEY[] = 
{ 0xA5, 0x01, 0x14, 0xCA, 0x2B, 0x7C, 0x81, 0x98, 0xE8, 0x29, 0xE7, 0xC9, 0x37, 0xD6, 0xFC, 0x40 };

//Ninja Gaiden Black
#define NGB_SAVE_FILE_SIZE     0x174A8 //includes hash and seed size
#define NGB_SAVE_DATA_SIZE     0x17490 //actual data size
#define NGB_SETTINGS_FILE_SIZE 0x550   //includes hash and seed size
#define NGB_SETTINGS_DATA_SIZE 0x538   //actual data size
byte NGB_TITLE_SIGNATURE_KEY[] = 
{ 0xFC, 0x33, 0x76, 0x48, 0x8B, 0x3E, 0x5F, 0x00, 0xF6, 0x5A, 0x6B, 0xDA, 0x92, 0x09, 0xCF, 0xE8 };

/*
known bytes found at the end of decrypted data

seemingly static data in both Ninja Gaiden and Ninja Gaiden Black and both saveXXX.dat and system.dat files

a better metric for determining decrypted data or decryption success would be to decompress the block of data in the header which holds a copyright string for the specific game

NOTE: this is lazy data detection and what these tail bytes are is unknown. actual game code uses the compressed copyright string to determine successful decryption
*/
byte KNOWN_TAIL_BYTES[] =
{ 0x94, 0x45, 0x8E, 0xD2, 0x8A, 0x4F };
#define KNOWN_TAIL_BYTES_SIZE 6

//applicable to both games and both file types
#define SIGNATURE_OFFSET 0
#define SIGNATURE_SIZE   0x14
#define SEED_OFFSET      0x14
#define DATA_OFFSET      0x18

//Xbox stuff
#define XBOXHDKEY_TEXT_SIZE 32
#define XBOXHDKEY_BYTE_SIZE 16
#define XUID_TEXT_SIZE      16
#define XUID_BYTE_SIZE      8
#define TITLE_KEY_SIZE      16


int main(int argc, char* argv[]) {

	utf8* file;
	FILE* file_handle = NULL;
	byte* file_buffer = 0;
	u32 file_size;
	bool is_ninja_gaiden_black; //true if working with Ninja Gaiden Black data, else Ninja Gaiden
	bool is_save_data; //true if working with save data, else settings data

	utf8* XboxHDKey_string;
	byte* XboxHDKey = 0;
	utf8* XUID_string;
	byte* XUID = 0;
	
	PrintInfo();

	if (argc < 3) {

		PrintUsage();
		goto exit;
	}

	if (strcmp((utf8*)argv[1], (utf8*)"encrypt", 7)) {

		if (argc != 4) {

			PrintUsage();
			goto exit;
		}

		//encrypt/sign file
		XboxHDKey_string = (utf8*)argv[2];
		file = (utf8*)argv[3];

		XboxHDKey = new byte[XBOXHDKEY_BYTE_SIZE];
		if (!CheckXboxHDKey(XboxHDKey_string, XboxHDKey)) {

			//error
			printf("\nError: Invalid XboxHDKey! Expected 32 hexadecimal characters\n");
			goto free_and_exit;
		}

		file_size = GetFileSize(file);

		if (file_size == 0) {

			//error
			printf("\nError: Failed to access file or file does not exist! \"%s\"\n", file);
			goto free_and_exit;
		}

		if (file_size == NG_SAVE_FILE_SIZE) {

			is_ninja_gaiden_black = false;
			is_save_data = true;
		}
		else if (file_size == NG_SETTINGS_FILE_SIZE) {

			is_ninja_gaiden_black = false;
			is_save_data = false;
		}
		else if (file_size == NGB_SAVE_FILE_SIZE) {

			is_ninja_gaiden_black = true;
			is_save_data = true;
		}
		else if (file_size == NGB_SETTINGS_FILE_SIZE) {

			is_ninja_gaiden_black = true;
			is_save_data = false;
		}
		else {

			//error
			printf("\nError: File is unexpected size! \"%s\"\n", file);
			goto free_and_exit;
		}

		file_handle = OpenFile(file, FILEMODE_READWRITE);
		if (!file_handle) {
			
			//error
			printf("\nError: Failed to open file! \"%s\"\n", file);
			goto free_and_exit;
		}

		file_buffer = new byte[file_size];
		if (ReadBytes(file_handle, 0, file_size, file_buffer) != file_size) {

			//error
			printf("\nError: Failed to read from file! \"%s\"\n", file);
			goto free_and_exit;
		}

		//check file is Ninja Gaiden/Ninja Gaiden Black data and that it's decrypted
		if (!memcmp((file_buffer + (file_size - KNOWN_TAIL_BYTES_SIZE)), KNOWN_TAIL_BYTES, KNOWN_TAIL_BYTES_SIZE)) {

			//error
			printf("\nError: File data is unknown or is not decrypted! \"%s\"\n", file);
			goto free_and_exit;
		}

		EncryptData(
			*(u32*)(file_buffer + SEED_OFFSET),
			(file_buffer + DATA_OFFSET),
			(is_ninja_gaiden_black ? (is_save_data ? NGB_SAVE_DATA_SIZE : NGB_SETTINGS_DATA_SIZE) : (is_save_data ? NG_SAVE_DATA_SIZE : NG_SETTINGS_DATA_SIZE))
		);

		SignFile((file_buffer + SIGNATURE_SIZE), (file_size - SIGNATURE_SIZE), XboxHDKey, (file_buffer + SIGNATURE_OFFSET), is_ninja_gaiden_black);

		if (WriteBytes(file_handle, 0, file_size, file_buffer) != file_size) {

			//error
			printf("\nError: Failed to write to file! \"%s\"\n", file);
			goto free_and_exit;
		}

		printf("\nFile encrypted and signed successfully.\n");
		goto free_and_exit;

	}
	else if (strcmp((utf8*)argv[1], (utf8*)"decrypt", 7)) {

		if (argc != 3) {

			PrintUsage();
			goto exit;
		}

		//decrypt file
		file = (utf8*)argv[2];

		file_size = GetFileSize(file);

		if (file_size == 0) {

			//error
			printf("\nError: Failed to access file or file does not exist! \"%s\"\n", file);
			goto free_and_exit;
		}

		if (file_size == NG_SAVE_FILE_SIZE) {

			is_ninja_gaiden_black = false;
			is_save_data = true;
		}
		else if (file_size == NG_SETTINGS_FILE_SIZE) {

			is_ninja_gaiden_black = false;
			is_save_data = false;
		}
		else if (file_size == NGB_SAVE_FILE_SIZE) {

			is_ninja_gaiden_black = true;
			is_save_data = true;
		}
		else if (file_size == NGB_SETTINGS_FILE_SIZE) {

			is_ninja_gaiden_black = true;
			is_save_data = false;
		}
		else {

			//error
			printf("\nError: File is unexpected size! \"%s\"\n", file);
			goto free_and_exit;
		}

		file_handle = OpenFile(file, FILEMODE_READWRITE);
		if (!file_handle) {

			//error
			printf("\nError: Failed to open file! \"%s\"\n", file);
			goto free_and_exit;
		}

		file_buffer = new byte[file_size];
		if (ReadBytes(file_handle, 0, file_size, file_buffer) != file_size) {

			//error
			printf("\nError: Failed to read from file! \"%s\"\n", file);
			goto free_and_exit;
		}

		DecryptData(
			*(u32*)(file_buffer + SEED_OFFSET),
			(file_buffer + DATA_OFFSET),
			(is_ninja_gaiden_black ? (is_save_data ? NGB_SAVE_DATA_SIZE : NGB_SETTINGS_DATA_SIZE) : (is_save_data ? NG_SAVE_DATA_SIZE : NG_SETTINGS_DATA_SIZE))
		);

		//check file decrypted properly
		if (!memcmp((file_buffer + (file_size - KNOWN_TAIL_BYTES_SIZE)), KNOWN_TAIL_BYTES, KNOWN_TAIL_BYTES_SIZE)) {

			//error
			printf("\nError: Failed to decrypt file! \"%s\"\n", file);
			goto free_and_exit;
		}

		if (WriteBytes(file_handle, 0, file_size, file_buffer) != file_size) {

			//error
			printf("\nError: Failed to write to file! \"%s\"\n", file);
			goto free_and_exit;
		}

		printf("\nFile decrypted successfully.\n");
		goto free_and_exit;

	}
	else if (strcmp((utf8*)argv[1], (utf8*)"xuid", 4)) {

		if (argc != 3 && argc != 5) {

			PrintUsage();
			goto exit;
		}

		if (argc == 3) { //decrypt/display XUID

			file = (utf8*)argv[2];

			file_size = GetFileSize(file);

			if (file_size == 0) {

				//error
				printf("\nError: Failed to access file or file does not exist! \"%s\"\n", file);
				goto free_and_exit;
			}

			if (file_size != NG_SAVE_FILE_SIZE) {

				//error
				printf("\nError: File is unexpected size! \"%s\"\n", file);
				goto free_and_exit;
			}

			is_ninja_gaiden_black = false;
			is_save_data = true;

			file_handle = OpenFile(file, FILEMODE_READWRITE);
			if (!file_handle) {

				//error
				printf("\nError: Failed to open file! \"%s\"\n", file);
				goto free_and_exit;
			}

			file_buffer = new byte[file_size];
			if (ReadBytes(file_handle, 0, file_size, file_buffer) != file_size) {

				//error
				printf("\nError: Failed to read from file! \"%s\"\n", file);
				goto free_and_exit;
			}

			DecryptData(
				*(u32*)(file_buffer + SEED_OFFSET),
				(file_buffer + DATA_OFFSET),
				(is_ninja_gaiden_black ? (is_save_data ? NGB_SAVE_DATA_SIZE : NGB_SETTINGS_DATA_SIZE) : (is_save_data ? NG_SAVE_DATA_SIZE : NG_SETTINGS_DATA_SIZE))
			);

			//check file decrypted properly
			if (!memcmp((file_buffer + (file_size - KNOWN_TAIL_BYTES_SIZE)), KNOWN_TAIL_BYTES, KNOWN_TAIL_BYTES_SIZE)) {

				//error
				printf("\nError: Failed to decrypt file! \"%s\"\n", file);
				goto free_and_exit;
			}

			printf("\nCurrent XUID: ");
			PrintHex((file_buffer + (DATA_OFFSET + NG_XUID_OFFSET)), XUID_BYTE_SIZE, true);

			goto free_and_exit;

		}
		else { //decrypt/change XUID/encrypt/sign file

			XUID_string = (utf8*)argv[2];
			XboxHDKey_string = (utf8*)argv[3];
			file = (utf8*)argv[4];

			XUID = new byte[XUID_BYTE_SIZE];
			if (!CheckXUID(XUID_string, XUID)) {

				//error
				printf("\nError: Invalid XUID! Expected 16 hexadecimal characters\n");
				goto free_and_exit;
			}

			XboxHDKey = new byte[XBOXHDKEY_BYTE_SIZE];
			if (!CheckXboxHDKey(XboxHDKey_string, XboxHDKey)) {

				//error
				printf("\nError: Invalid XboxHDKey! Expected 32 hexadecimal characters\n");
				goto free_and_exit;
			}

			file_size = GetFileSize(file);

			if (file_size == 0) {

				//error
				printf("\nError: Failed to access file or file does not exist! \"%s\"\n", file);
				goto free_and_exit;
			}

			if (file_size != NG_SAVE_FILE_SIZE) {

				//error
				printf("\nError: File is unexpected size! \"%s\"\n", file);
				goto free_and_exit;
			}

			is_ninja_gaiden_black = false;
			is_save_data = true;

			file_handle = OpenFile(file, FILEMODE_READWRITE);
			if (!file_handle) {

				//error
				printf("\nError: Failed to open file! \"%s\"\n", file);
				goto free_and_exit;
			}

			file_buffer = new byte[file_size];
			if (ReadBytes(file_handle, 0, file_size, file_buffer) != file_size) {

				//error
				printf("\nError: Failed to read from file! \"%s\"\n", file);
				goto free_and_exit;
			}

			DecryptData(
				*(u32*)(file_buffer + SEED_OFFSET),
				(file_buffer + DATA_OFFSET),
				(is_ninja_gaiden_black ? (is_save_data ? NGB_SAVE_DATA_SIZE : NGB_SETTINGS_DATA_SIZE) : (is_save_data ? NG_SAVE_DATA_SIZE : NG_SETTINGS_DATA_SIZE))
			);

			//check file decrypted properly
			if (!memcmp((file_buffer + (file_size - KNOWN_TAIL_BYTES_SIZE)), KNOWN_TAIL_BYTES, KNOWN_TAIL_BYTES_SIZE)) {

				//error
				printf("\nError: Failed to decrypt file! \"%s\"\n", file);
				goto free_and_exit;
			}

			printf("\nChanged XUID: ");
			PrintHex((file_buffer + (DATA_OFFSET + NG_XUID_OFFSET)), XUID_BYTE_SIZE, false);
			printf(" -> ");
			memcpy((file_buffer + (DATA_OFFSET + NG_XUID_OFFSET)), XUID, XUID_BYTE_SIZE);
			PrintHex((file_buffer + (DATA_OFFSET + NG_XUID_OFFSET)), XUID_BYTE_SIZE, true);

			EncryptData(
				*(u32*)(file_buffer + SEED_OFFSET),
				(file_buffer + DATA_OFFSET),
				(is_ninja_gaiden_black ? (is_save_data ? NGB_SAVE_DATA_SIZE : NGB_SETTINGS_DATA_SIZE) : (is_save_data ? NG_SAVE_DATA_SIZE : NG_SETTINGS_DATA_SIZE))
			);

			SignFile((file_buffer + SIGNATURE_SIZE), (file_size - SIGNATURE_SIZE), XboxHDKey, (file_buffer + SIGNATURE_OFFSET), is_ninja_gaiden_black);

			if (WriteBytes(file_handle, 0, file_size, file_buffer) != file_size) {

				//error
				printf("\nError: Failed to write to file! \"%s\"\n", file);
				goto free_and_exit;
			}

			printf("\nXUID changed successfully. File signed successfully.\n");
			goto free_and_exit;

		}

	}
	else {
	
		//error
		printf("\nError: Invalid parameter(s) specified!\n");
		PrintUsage();
		goto exit;
	}


free_and_exit:
	if (file_handle)
		CloseFile(file_handle);

	if (file_buffer)
		delete[] file_buffer;

	if (XboxHDKey)
		delete[] XboxHDKey;

	if (XUID)
		delete[] XUID;

exit:
	return 0;
}

bool CheckXboxHDKey(utf8* input_XboxHDKey_string, byte* output_XboxHDKey_bytes) {

	if (input_XboxHDKey_string && output_XboxHDKey_bytes) {

		u32 num_bytes = ByteCountFromHexLen(input_XboxHDKey_string);
		if (num_bytes != XBOXHDKEY_BYTE_SIZE)
			return false;

		return HexToBytes(input_XboxHDKey_string, XBOXHDKEY_TEXT_SIZE, output_XboxHDKey_bytes);

	}

	return false;

}

bool CheckXUID(utf8* input_XUID_string, byte* output_XUID_bytes) {

	if (input_XUID_string && output_XUID_bytes) {

		u32 num_bytes = ByteCountFromHexLen(input_XUID_string);
		if (num_bytes != XUID_BYTE_SIZE)
			return false;

		return HexToBytes(input_XUID_string, XUID_TEXT_SIZE, output_XUID_bytes);

	}

	return false;

}

void SignFile(byte* buffer, u32 length, byte* XboxHDKey, byte* hash, bool is_ninja_gaiden_black) {

	XCALCSIG_CONTEXT* context = XCalculateSignatureBegin(
		XCALCSIG_FLAG_NON_ROAMABLE, 
		(is_ninja_gaiden_black ? NGB_TITLE_SIGNATURE_KEY : NG_TITLE_SIGNATURE_KEY), 
		XboxHDKey
	);

	if (context) {

		XCalculateSignatureUpdate(context, buffer, length);
		XCalculateSignatureEnd(context, hash);
	}

}

void DecryptData(u32 seed, byte* buffer, u32 length) {

	MT_CONTEXT* MersenneTwister = new MT_CONTEXT{ 0 }; //pseudo random number generator
	BLOWFISH_CONTEXT* Blowfish = new BLOWFISH_CONTEXT{ 0 }; //crypto
	byte* key = new byte[56]; //max blowfish key size

	MTInit(MersenneTwister, seed); //init MT
	MTFillBuffer(MersenneTwister, key, 56); //generate key bytes based on seed

	MTXorBuffer(MersenneTwister, buffer, length, buffer); //xor decrypt the encrypted data

	BlowfishInit(Blowfish, key, 56); //init blowfish crypto

	u32 left;
	u32 right;

	for (u32 i = 0; i < (length / 8); i++) { //blowfish decrypt the encrypted data, 8 byte blocks

		left = *(u32*)(buffer + (i * 8));
		right = *(u32*)(buffer + ((i * 8) + 4));
		
		BlowfishDecryptBlock(Blowfish, &left, &right);

		*(u32*)(buffer + (i * 8)) = left;
		*(u32*)(buffer + ((i * 8) + 4)) = right;
	}

	//clean up
	delete MersenneTwister;
	delete Blowfish;
	delete[] key;

}

void EncryptData(u32 seed, byte* buffer, u32 length) {

	MT_CONTEXT* MersenneTwister = new MT_CONTEXT{ 0 }; //pseudo random number generator
	BLOWFISH_CONTEXT* Blowfish = new BLOWFISH_CONTEXT{ 0 }; //crypto
	byte* key = new byte[56]; //max blowfish key size

	MTInit(MersenneTwister, seed); //init MT
	MTFillBuffer(MersenneTwister, key, 56); //generate key bytes based on seed (seed is the Xbox cpu tick count and stored in the file upon saving)

	BlowfishInit(Blowfish, key, 56); //init blowfish crypto

	u32 left;
	u32 right;

	for (u32 i = 0; i < (length / 8); i++) { //blowfish encrypt the data, 8 byte blocks

		left = *(u32*)(buffer + (i * 8));
		right = *(u32*)(buffer + ((i * 8) + 4));

		BlowfishEncryptBlock(Blowfish, &left, &right);

		*(u32*)(buffer + (i * 8)) = left;
		*(u32*)(buffer + ((i * 8) + 4)) = right;
	}

	MTXorBuffer(MersenneTwister, buffer, length, buffer); //xor encrypt the encrypted data

	//clean up
	delete MersenneTwister;
	delete Blowfish;
	delete[] key;

}

void PrintHex(byte* buffer, u32 length, bool newLine) {

	if (buffer && length > 0) {

		u32 string_length = HexLenFromByteCount(length, true);
		utf8* string = new utf8[string_length];
		BytesToHex(buffer, length, string);
		string[string_length - 1] = '\0';

		printf("%s", string);
		if (newLine)
			printf("\n");

		delete[] string;
	}

}

void PrintInfo() {

	printf("\nNinja Gaiden Encryption/Signing Tool (%s)\n\n", INFO_BUILD_DATE);
	printf("-----------------------------------------------------\n");
	printf("Game(s)      : %s / %s\n", INFO_GAME_NAME1, INFO_GAME_NAME2);
	printf("TitleID(s)   : %s / %s\n", INFO_TITLE_ID1, INFO_TITLE_ID2);
	printf("Region(s)    : %s\n\n", INFO_SUPPORTED_REGIONS);
	printf("%s\n", INFO_URL);
	printf("-----------------------------------------------------\n\n");

}

void PrintUsage() {

	printf("\n------------------------Usage------------------------\n\n");
	printf("Encrypt     : ninjacrypt.exe encrypt <XboxHDKey> <file>\n");
	printf("Decrypt     : ninjacrypt.exe decrypt <file>\n");
	printf("Show XUID   : ninjacrypt.exe xuid <file>\n");
	printf("Change XUID : ninjacrypt.exe xuid <XUID> <XboxHDKey> <file>\n\n");
	printf("XUID functions are for Ninja Gaiden save data only");
	printf("\n-----------------------------------------------------\n\n");

}