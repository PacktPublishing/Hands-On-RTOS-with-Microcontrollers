#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * checks that Buffer ends with a valid CRC-32
 * @param Buff	data, including 4 byte little endian
 * 				CRC-32 value
 * @param Len	number of bytes in Buffer, including 4 byte
 * 				CRC-32
 * CRC32 code derived from work by Gary S. Brown.
 * https://web.mit.edu/freebsd/head/sys/libkern/crc32.c
 *
 */

bool CheckCRC(const uint8_t *Buff, const uint32_t Len, const uint32_t * hcrc)
{
	//make sure args and crc32 return are not NULL
	if(!Buff || !Len){
		return false;
	}

	uint32_t result = HAL_CRC_Calculate(hcrc, (uint32_t*)Buff, 4);

	if(!result){
		return false;
	}

	result = result ^ ~0U;

	//for each byte of crc32 in packet, ensure equality with crc32 calculated on packet contents
	for (int i = 0; i < 4; i++)
	{
		uint32_t hashresult = result & (0x000000FF << (8 * i));
		char hex[3];
		//first 2 digits of each hash result is used; all other digits are significant but always 0
		sprintf(hex, "%02x", hashresult >> (8 * i));

		uint8_t uint8_value = strtoul(hex, NULL, 16);

		if (uint8_value != Buff[i + 5])
		{
			return false;
		}
	}

	return true;
}
