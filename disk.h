#ifndef DISK_H_H
#define DISK_H_H

extern int read_sectors_max_256(char *des, int startSector, int count);
int read_sectors(char *des, int startSector, int count);
extern int read_ata_sectors(char *des, int startSector, int count);


void checkAHCI();
#endif