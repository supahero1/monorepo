/*
 *   Copyright 2024-2026 Franciszek Balcerak
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/base64.h>
#include <shared/alloc/base.h>

#include <stddef.h>
#include <stdint.h>


uint64_t
base64_encoded_len(
	uint64_t len
	)
{
	return ((len + 2) / 3) << 2;
}


uint8_t
base64_encode_table[] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};


uint8_t*
base64_encode(
	const uint8_t* data,
	uint64_t in_len,
	uint64_t* out_len
	)
{
	assert_ptr(data, in_len);

	uint64_t encoded_len = base64_encoded_len(in_len);
	uint8_t* encoded = alloc_malloc(encoded, encoded_len);
	assert_ptr(encoded, encoded_len);

	uint64_t full_encodes = in_len / 3;
	const uint8_t* data_end = data + full_encodes * 3;

	while(data != data_end)
	{
		*(encoded++) = base64_encode_table[*data >> 2];
		*(encoded++) = base64_encode_table[((*data & 0x03) << 4) | (*(data + 1) >> 4)];
		++data;
		*(encoded++) = base64_encode_table[((*data & 0x0F) << 2) | (*(data + 1) >> 6)];
		++data;
		*(encoded++) = base64_encode_table[*data & 0x3F];
		++data;
	}


	switch(in_len % 3)
	{

	case 0: break;

	case 1:
	{
		*(encoded++) = base64_encode_table[*data >> 2];
		*(encoded++) = base64_encode_table[(*data & 0x03) << 4];
		*(encoded++) = '=';
		*(encoded++) = '=';
		break;
	}

	case 2:
	{
		*(encoded++) = base64_encode_table[*data >> 2];
		*(encoded++) = base64_encode_table[((*data & 0x03) << 4) | (*(data + 1) >> 4)];
		++data;
		*(encoded++) = base64_encode_table[(*data & 0x0F) << 2];
		*(encoded++) = '=';
		break;
	}

	default: assert_unreachable();

	}


	if(out_len != NULL)
	{
		*out_len = encoded_len;
	}

	return encoded - encoded_len;
}


uint64_t
base64_decoded_len(
	uint64_t len
	)
{
	return (len * 3) >> 2;
}


uint8_t
base64_decode_table[256] =
{
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, -1, -1, -1,  0, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,
	 7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, -1, -1, -1, -1, -1,
};


uint8_t*
base64_decode(
	const uint8_t* data,
	uint64_t in_len,
	uint64_t* out_len
	)
{
	assert_true(base64_is_valid(data, in_len));

	uint64_t decoded_len = base64_decoded_len(in_len);
	uint8_t* decoded = alloc_malloc(decoded, decoded_len);
	assert_ptr(decoded, decoded_len);

	const uint8_t* data_end = data + in_len;

	while(data != data_end)
	{
		*(decoded++) = (base64_decode_table[*data] << 2) | (base64_decode_table[*(data + 1)] >> 4);
		++data;
		*(decoded++) = (base64_decode_table[*data] << 4) | (base64_decode_table[*(data + 1)] >> 2);
		++data;
		*(decoded++) = (base64_decode_table[*data] << 6) | base64_decode_table[*(data + 1)];
		data += 2;
	}

	if(in_len)
	{
		decoded -= decoded_len;
		uint64_t new_len = decoded_len;

		while(*(data - 1) == '=')
		{
			--data;
			--new_len;
		}

		decoded = alloc_remalloc(decoded, decoded_len, new_len);
		decoded_len = new_len;
		assert_not_null(decoded);
	}

	if(out_len != NULL)
	{
		*out_len = decoded_len;
	}

	return decoded;
}


bool
base64_is_valid(
	const uint8_t* data,
	uint64_t len
	)
{
	assert_ptr(data, len);

	if((len & 3) != 0)
	{
		return false;
	}

	const uint8_t* data_end = data + len;

	while(data != data_end)
	{
		if(base64_decode_table[*data] == (uint8_t) -1)
		{
			return false;
		}

		++data;
	}

	return true;
}
