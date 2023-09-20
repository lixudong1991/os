#include "boot.h"

int read_sectors(char *des, int startSector, int count)
{
	int sum = count;
	int startsector = startSector;
	int memindex = 0;
	int readcount = 0;
	if (count == 0)
		return 0;
	do
	{
		readcount = sum > 256 ? 256 : sum;
		memindex += read_sectors_max_256(des + memindex, startsector, readcount);
		startsector += readcount;
		sum -= readcount;
	} while (sum != 0);
	return startsector - startSector;
}
