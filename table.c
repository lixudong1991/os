#include "boot.h"

uint16 appendTableSegItem(Tableinfo *info,TableSegmentItem *item)
{
	uint64 data = 0;
	uint64 baseaddrdata = (item->segmentBaseAddr&0xffff);
	baseaddrdata <<= 16;
	uint64 baseaddrdata1 = (item->segmentBaseAddr&0xff0000);
	baseaddrdata1 <<=16;
	uint64 baseaddrdata2 = (item->segmentBaseAddr&0xff000000);
	baseaddrdata2 <<=32;
	
	uint64 addrlimit =(item->segmentLimit&0xffff);
	uint64 addrlimit1 =(item->segmentLimit&0xf0000);
	addrlimit1<<=32;
	data|=baseaddrdata;
	data|=baseaddrdata1;
	data|=baseaddrdata2;
	data|=addrlimit;
	data|=addrlimit1;
	if(item->G)
		data|=0x80000000000000;
	if(item->D_B)
		data|=0x40000000000000;
	if(item->L)
		data|=0x20000000000000;
	if(item->AVL)
		data|=0x10000000000000;
	
	if(item->P)
		data|=0x800000000000;
	if(item->S)
		data|=0x100000000000;

	uint64 dpl=(item->DPL&0x3);
	dpl <<= 45;
	data|=dpl;
	
	uint64 type=(item->Type&0xf);
	type <<= 40;
	data|=type;
	
	short index=(info->limit+1)/8;
	((uint64*)(info->base))[index]=data;
	info->limit=info->limit+8;
	uint16 select = (index<<3);
	if(info->type)
		select|=(1<<2);
	select|=(item->DPL&0x3);
	return select;
}
BOOL getTableSegItem(Tableinfo *info,TableSegmentItem *item,uint16 SegSelect)
{
	memset_s(item,0,sizeof(TableSegmentItem));
	uint16 index = SegSelect >> 3;
	if (index > (info->limit + 1) / 8 - 1)
		return 0;

	if (info->type != ((SegSelect & 4) >> 2))
		return 0;
	uint64 data = ((uint64*)(info->base))[index];
	if (((data >> 40) & 0xf) == 2 && (data & 0x100000000000 == 0))
		return 0;

	uint64 val = SegSelect & 0x3;
	val<<= 45;
	if (val != (data & 0x600000000000))
		return 0;
	item->DPL = (char)(SegSelect&3);


	uint32 segmentBaseAddr = (uint32)((data >> 16)&0xffff);
	segmentBaseAddr |= (uint32)((data >> 16) & 0xFF0000);
	segmentBaseAddr |= (uint32)((data >> 32) & 0xff000000);
	uint32 segmentLimit = (uint32)(data & 0xffff);
	segmentLimit |= (uint32)((data >> 32) & 0xF0000);
	if (data & 0x80000000000000 != 0)
		item->G = 1;
	if (data & 0x40000000000000!=0)
		item->D_B=1;
	if (data & 0x20000000000000 != 0)
		item->L = 1;
	if (data & 0x10000000000000 != 0)
		item->AVL = 1;

	if (data & 0x800000000000 != 0)
		item->P = 1;
	if (data & 0x100000000000 != 0)
		item->S = 1;
	item->Type = (char)((data >> 40) & 0xf);

	return 1;
}
uint16 appendTableGateItem(Tableinfo *info,TableGateItem *item)
{
	uint64 data = item->Type;
	uint64 segselect = item->segSelect;
	segselect<<=16;
	uint64 segAddr = (item->segAddr&0xffff);
	uint64 segAddr1 = (item->segAddr&0xffff0000);
	segAddr1<<=32;
	
	uint64 argcount =(item->argCount&0x1F);
	argcount<<=32;
	data|=segselect;
	data|=segAddr;
	data|=segAddr1;
	data|=argcount;
	if(item->P)
		data|= 0x800000000000;
	uint64 dpl=(item->GateDPL&0x3);
	dpl <<= 45;
	data|=dpl;
	
	short index=(info->limit+1)/8;
	((uint64*)(info->base))[index]=data;
	info->limit=info->limit+8;
	uint16 select = (index<<3);
	if(info->type)
		select|=(1<<2);
	select|=(item->GateDPL&0x3);
	return select;
}