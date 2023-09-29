#include <stddef.h>
#include <stdint.h>
#include <CRC32.h>
extern hcrc;

/**
 * checks that Buffer ends with a valid CRC-32
 * @param Buff	data, including 4 byte little endian
 * 				CRC-32 value
 * @param Len	number of bytes in Buffer, including 4 byte
 * 				CRC-32
 */

bool CheckCRC(const uint8_t *Buff, const uint32_t Len)
{
	//make sure args and crc32 return are not NULL
	if(!Buff || !Len){
		return false;
	}
	//Use HAL provided CRC32 module to calculate sum
	uint32_t result = HAL_CRC_Calculate(&hcrc, (uint32_t*)Buff, 4);
	result = result ^ ~0U;

	if(!result){
		return false;
	}
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


/*
hcrc.Instance = CRC;
hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
*/
