#pragma once
#include "Data\Types.h"
#include "Data\FileIO.h"
#include "Data\Functions.h"
#include "Data\Hexadecimal.h"
#include "MersenneTwister\MersenneTwister.h"
#include "MersenneTwisterRevised\MersenneTwisterRevised.h"
#include "Blowfish\Blowfish.h"
#include "XCalcSigLib\XCalcSigLib.h"

int main(int argc, char* argv[]);
bool CheckXboxHDKey(utf8* in_XboxHDKey, byte* out_XboxHDKey);
bool CheckXUID(utf8* in_XUID, byte* out_XUID);
void SignFile(byte* buffer, u32 length, byte* XboxHDKey, byte* hash, bool NGB);
void DecryptData(u32 seed, byte* buffer, u32 length);
void EncryptData(u32 seed, byte* buffer, u32 length);
void PrintHex(byte* buffer, u32 length, bool newLine);
void PrintInfo();
void PrintUsage();

#define INFO_GAME_NAME1 "Ninja Gaiden\0"
#define INFO_GAME_NAME2 "Ninja Gaiden Black\0"
#define INFO_SUPPORTED_REGIONS "NTSC-U / NTSC-J / PAL\0"
#define INFO_TITLE_ID1 "54430003\0"
#define INFO_TITLE_ID2 "5443000D\0"

#define INFO_URL "https://github.com/feudalnate\0"
#define INFO_BUILD_DATE __DATE__