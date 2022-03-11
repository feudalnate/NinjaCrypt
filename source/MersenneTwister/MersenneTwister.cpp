#include "MersenneTwister.h"

void MTInit(MT_CONTEXT* context, u32 seed) {

	if (context) {

		*(u32*)context->state = seed;
		u32 index = 1;
		byte* table_offset = context->state;

		do {

			*(u32*)(table_offset + 4) = index++ + 0x6C078965 * (*(u32*)table_offset ^ (*(u32*)table_offset >> 30));
			table_offset += 4;

		} while (index < 624);

		*(u32*)(context->state + 2496) = 1;

	}

}

void MTTwist(MT_CONTEXT* context) {

	if (context) {

		byte* table_offset = context->state;
		*(u32*)(context->state + 2496) = 624;
		*(u32*)(context->state + 2500) = (u32)context->state;

		u32 index = 227;
		do
		{
			*(u32*)table_offset = *(u32*)(table_offset + 1588) ^
				((*(u32*)(table_offset + 4) & 1) != 0 ? 0x9908B0DF : 0) ^
				((*(u32*)table_offset ^ (*(u32*)(table_offset + 4) ^ *(u32*)table_offset) & 0x7FFFFFFEu) >> 1);

			table_offset += 4;
			--index;

		} while (index);

		index = 396;
		do
		{
			*(u32*)table_offset = *(u32*)(table_offset - 908) ^
				((*(u32*)(table_offset + 4) & 1) != 0 ? 0x9908B0DF : 0) ^
				((*(u32*)table_offset ^ (*(u32*)(table_offset + 4) ^ *(u32*)table_offset) & 0x7FFFFFFEu) >> 1);

			table_offset += 4;
			--index;

		} while (index);

		*(u32*)table_offset = *(u32*)(table_offset - 908) ^
			((*(u32*)context->state & 1) != 0 ? 0x9908B0DF : 0) ^
			((*(u32*)table_offset ^ (*(u32*)context->state ^ *(u32*)table_offset) & 0x7FFFFFFEu) >> 1);

	}

}

void MTFillBuffer(MT_CONTEXT* context, byte* buffer, u32 length) {

	if (context && buffer && (length / 4) > 0) {

		u32* value;
		u32 n;
		u32 count = (length / 4);

		do
		{

			if ((*(u32*)(context->state + 2496))-- == 1)
				MTTwist(context);

			value = *(u32**)(context->state + 2500);
			n = *value;
			*(u32*)(context->state + 2500) = (u32)(value + 1);

			u32 a = ((((n >> 11) ^ n) & 0xFF3A58AD) << 7) ^ (n >> 11) ^ n;
			u32 b = ((a & 0xFFFFDF8C) << 15) ^ a;
			*(u32*)buffer = b ^ (b >> 18);
			buffer += 4;

			--count;

		} while (count);

	}

}

void MTXorBuffer(MT_CONTEXT* context, byte* input, u32 length, byte* output) {

	if (context && input && (length / 4) > 0 && output) {

		byte* input_offset = input;
		byte* output_offset = output;
		u32 count = (length / 4);

		do {

			u32 x = *(u32*)input_offset;

			if ((*(u32*)(context->state + 2496))-- == 1)
				MTTwist(context);

			u32* value = *(u32**)(context->state + 2500);
			u32 n = *value;
			*(u32*)(context->state + 2500) = (u32)(value + 1);

			u32 a = ((((n >> 11) ^ n) & 0xFF3A58AD) << 7) ^ (n >> 11) ^ n;
			u32 b = ((a & 0xFFFFDF8C) << 15) ^ a ^ ((((a & 0xFFFFDF8C) << 15) ^ a) >> 18);

			*(u32*)output_offset = x ^ b;

			input_offset += 4;
			output_offset += 4;

			count--;

		} while (count);

	}

}