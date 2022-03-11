#pragma once
#ifndef _MERSENNETWISTER_H
#define _MERSENNETWISTER_H
#include "Types.h"

/*
	NOTE: This is scuffed. Ninja Gaiden specific, don't recommend using in other projects

	MTFillBuffer and MTXorBuffer only supports lengths divisible by 4

	SELF-NOTE: Should revisit this and make it more complete
*/

struct MT_CONTEXT {
	byte state[2504];
};

void MTInit(MT_CONTEXT* context, u32 seed);
void MTFillBuffer(MT_CONTEXT* context, byte* buffer, u32 length);
void MTXorBuffer(MT_CONTEXT* context, byte* input, u32 length, byte* output);

#endif
