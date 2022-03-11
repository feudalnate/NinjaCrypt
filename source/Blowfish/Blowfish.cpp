#include "Blowfish.h";

void BlowfishInit(BLOWFISH_CONTEXT* context, byte* key, u32 key_length) {

	if (context && key && key_length >= 4 && key_length <= 56) {

		//copy P-Box
		for (u32 i = 0; i < 18; i++)
			context->P[i] = P[i];

		//copy S-Boxes
		for (u32 i = 0; i < 256; i++) {

			context->S[0][i] = S0[i];
			context->S[1][i] = S1[i];
			context->S[2][i] = S2[i];
			context->S[3][i] = S3[i];
		}

		//init key
		u32 key_index = 0;
		u32 key_value = 0;
		for (u32 i = 0; i < 18; i++) {

			for (u32 x = 0; x < 4; x++) {

				key_value = ((key_value << 8) | key[key_index]);
				key_index = ((key_index + 1) % key_length);
			}

			context->P[i] ^= key_value;
			key_value = 0;

		}

		//expand key
		u32 left = 0;
		u32 right = 0;
		for (u32 i = 0; i < 18; i += 2) {

			BlowfishEncryptBlock(context, &left, &right);
			context->P[i] = left;
			context->P[i + 1] = right;

		}

		for (u32 i = 0; i < 4; i++) {

			for (u32 x = 0; x < 256; x += 2) {

				BlowfishEncryptBlock(context, &left, &right);
				context->S[i][x] = left;
				context->S[i][x + 1] = right;

			}

		}

	}

}

u32 F(BLOWFISH_CONTEXT* context, u32 value) {

	if (context) {

		u32 a, b, c, d, x;

		d = (value & 0xFF);
		value = (value >> 8);

		c = (value & 0xFF);
		value = (value >> 8);

		b = (value & 0xFF);
		value = (value >> 8);

		a = (value & 0xFF);

		x = (context->S[0][a] + context->S[1][b]);
		x = (x ^ context->S[2][c]);
		x = (x + context->S[3][d]);

		return x;

	}

}

void BlowfishEncryptBlock(BLOWFISH_CONTEXT* context, u32* left, u32* right) {

	if (context && left && right) {

		u32 temp;
		u32 left_value = *left;
		u32 right_value = *right;

		for (u32 i = 0; i < 16; i++) {

			left_value = (left_value ^ context->P[i]);
			right_value = (F(context, left_value) ^ right_value);

			//swap
			temp = left_value;
			left_value = right_value;
			right_value = temp;

		}

		//swap
		temp = left_value;
		left_value = right_value;
		right_value = temp;

		right_value = (right_value ^ context->P[16]);
		left_value = (left_value ^ context->P[17]);

		*left = left_value;
		*right = right_value;

	}

}

void BlowfishDecryptBlock(BLOWFISH_CONTEXT* context, u32* left, u32* right) {

	if (context && left && right) {

		u32 temp;
		u32 left_value = *left;
		u32 right_value = *right;

		for (u32 i = 17; i > 1; i--) {

			left_value = (left_value ^ context->P[i]);
			right_value = (F(context, left_value) ^ right_value);

			//swap
			temp = left_value;
			left_value = right_value;
			right_value = temp;

		}

		//swap
		temp = left_value;
		left_value = right_value;
		right_value = temp;

		right_value = (right_value ^ context->P[1]);
		left_value = (left_value ^ context->P[0]);

		*left = left_value;
		*right = right_value;

	}

}