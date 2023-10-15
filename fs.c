#include "ff.h"
#include "stdint.h"
#include "boot.h"
#include "printf.h"
#include "string.h"
#include "ps2device.h"
#include "diskio.h"
#include "stdio.h"




#ifdef TEST_FATFS
static FATFS* FatFs = NULL;   //一定是一个全局变量
static BYTE* Buff = NULL;//[FF_MAX_SS]; //一定是一个全局变量
#define Buff_LEN 4096
static DIR* Dir = NULL;
static char *Line = NULL;				/* Console input buffer */
static FIL *File[2];
static FILINFO *Finfo=NULL;

void initFATfsObj()
{
	//     static FATFS *FatFs=NULL;   //一定是一个全局变量
	// static BYTE *Buff=NULL;//[FF_MAX_SS]; //一定是一个全局变量
	// static DIR  *Dir=NULL;
	// static char *Line = NULL;				/* Console input buffer */
	// static FIL *File[2];
	// static FILINFO *Finfo=NULL;

	FatFs = kernel_malloc(sizeof(FATFS));
	memset(FatFs, 0, sizeof(FATFS));

	Buff = kernel_malloc(Buff_LEN);
	memset(Buff, 0, Buff_LEN);

	Dir = kernel_malloc(sizeof(DIR));
	memset(Dir, 0, sizeof(DIR));

	Line = kernel_malloc(256);
	memset(Line, 0, 256);

	File[0] = kernel_malloc(sizeof(FIL));
	memset(File[0], 0, sizeof(FIL));
	File[1] = kernel_malloc(sizeof(FIL));
	memset(File[1], 0, sizeof(FIL));

	Finfo = kernel_malloc(sizeof(FILINFO));
	memset(Finfo, 0, sizeof(FILINFO));
}

static FRESULT scan_files (
	char* path,		/* Pointer to the path name working buffer */
	UINT* n_dir,
	UINT* n_file,
	QWORD* sz_file
)
{
	DIR dirs;
	FRESULT res;
	BYTE i;


	if ((res = f_opendir(&dirs, path)) == FR_OK) {
		i = strlen(path);
		while (((res = f_readdir(&dirs, Finfo)) == FR_OK) && Finfo->fname[0]) {
			if (Finfo->fattrib & AM_DIR) {
				(*n_dir)++;
				*(path+i) = '/'; strcpy(path+i+1, Finfo->fname);
				res = scan_files(path, n_dir, n_file, sz_file);
				*(path+i) = '\0';
				if (res != FR_OK) break;
			} else {
			/*	xprintf("%s/%s\n", path, fn); */
				(*n_file)++;
				*sz_file += Finfo->fsize;
			}
		}
	}

	return res;
}
static
void put_rc (FRESULT rc)
{
	const char *str =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0" "INVALID_PARAMETER\0";
	FRESULT i;

	for (i = 0; i != rc && *str; i++) {
		while (*str++) ;
	}
	printf("rc=%u FR_%s\n", (UINT)rc, str);
}
static
const char HelpMsg[] =
	"[Disk contorls]\n"
	" di <pd#> - Initialize disk\n"
	" dd [<pd#> <lba>] - Dump a secrtor\n"
	" ds <pd#> - Show disk status\n"
	"[Buffer controls]\n"
	" bd <ofs> - Dump working buffer\n"
	" be <ofs> [<data>] ... - Edit working buffer\n"
	" br <pd#> <lba> [<count>] - Read disk into working buffer\n"
	" bw <pd#> <lba> [<count>] - Write working buffer into disk\n"
	" bf <val> - Fill working buffer\n"
	"[File system controls]\n"
	" fi <ld#> [<mount>]- Force initialized the volume\n"
	" fs [<path>] - Show volume status\n"
	" fl [<path>] - Show a directory\n"
	" fo <mode> <file> - Open a file\n"
	" fc - Close the file\n"
	" fe <ofs> - Move fp in normal seek\n"
	" fd <len> - Read and dump the file\n"
	" fr <len> - Read the file\n"
	" fw <len> <val> - Write to the file\n"
	" fn <org.name> <new.name> - Rename an object\n"
	" fu <name> - Unlink an object\n"
	" fv - Truncate the file at current fp\n"
	" fk <name> - Create a directory\n"
	" fa <atrr> <mask> <object name> - Change attribute of an object\n"
	" ft <year> <month> <day> <hour> <min> <sec> <name> - Change timestamp of an object\n"
	" fx <src.file> <dst.file> - Copy a file\n"
	" fg <path> - Change current directory\n"
	" fq - Show current directory\n"
	" fb <name> - Set volume label\n"
	" fm <ld#> <type> <csize> - Create file system\n"
	" fz [<len>] - Change/Show R/W length for fr/fw/fx command\n"
	"[Misc commands]\n"
	" md[b|h|w] <addr> [<count>] - Dump memory\n"
	" mf <addr> <value> <count> - Fill memory\n"
	" me[b|h|w] <addr> [<value> ...] - Edit memory\n"
	" t [<year> <mon> <mday> <hour> <min> <sec>] - Set/Show RTC\n"
	" v <name> vbeinfo wirte\n"
	"\n";

#define xputs puts
#define xprintf  printf
#define xputc putchar
#define xgets fgets

void put_dump (
	const void* buff,		/* Pointer to the array to be dumped */
	unsigned int addr,		/* Heading address value */
	int len,				/* Number of items to be dumped */
	int width				/* Size of buff[0] (1, 2, 4) */
)
{
	int i;
	const unsigned char *bp;
	const unsigned short *sp;
	const unsigned int *lp;


	xprintf("%08lX ", addr);		/* address */

	switch (width) {
	case 1:
		bp = buff;
		for (i = 0; i < len; i++) {		/* Hexdecimal dump */
			xprintf(" %02X", bp[i]);
		}
		xputs("  ");
		for (i = 0; i < len; i++) {		/* ASCII dump */
			xputc((unsigned char)((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.'));
		}
		break;
	case 2:
		sp = buff;
		do {							/* Hexdecimal dump */
			xprintf(" %04X", *sp++);
		} while (--len);
		break;
	case 4:
		lp = buff;
		do {							/* Hexdecimal dump */
			xprintf(" %08LX", *lp++);
		} while (--len);
		break;
	}

	xputc('\n');
}

int xatoi (			/* 0:Failed, 1:Successful */
	char **str,		/* Pointer to pointer to the string */
	int *res		/* Pointer to the valiable to store the value */
)
{
	unsigned int val;
	unsigned char c, r, s = 0;


	*res = 0;

	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */

	if (c == '-') {		/* negative? */
		s = 1;
		c = *(++(*str));
	}

	if (c == '0') {
		c = *(++(*str));
		switch (c) {
		case 'x':		/* hexdecimal */
			r = 16; c = *(++(*str));
			break;
		case 'b':		/* binary */
			r = 2; c = *(++(*str));
			break;
		default:
			if (c <= ' ') return 1;	/* single zero */
			if (c < '0' || c > '9') return 0;	/* invalid char */
			r = 8;		/* octal */
		}
	} else {
		if (c < '0' || c > '9') return 0;	/* EOL or invalid char */
		r = 10;			/* decimal */
	}

	val = 0;
	while (c > ' ') {
		if (c >= 'a') c -= 0x20;
		c -= '0';
		if (c >= 17) {
			c -= 7;
			if (c <= 9) return 0;	/* invalid char */
		}
		if (c >= r) return 0;		/* invalid char for current radix */
		val = val * r + c;
		c = *(++(*str));
	}
	if (s) val = 0 - val;			/* apply sign if needed */

	*res = val;
	return 1;
}
void wirteVBEinfo(char *filename);
void testFATfs()
{
    initFATfsObj();
	static const char *ft[] = {"", "FAT12", "FAT16", "FAT32", "exFAT"};
	static const char days[] = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";
	char *ptr, *ptr2;
	int p1, p2, p3;
	BYTE res, b, drv = 0;
	UINT s1, s2, cnt, blen = Buff_LEN, acc_files, acc_dirs;
	DWORD ofs = 0, sect = 0, blk[2], dw;
	QWORD acc_size;
	FATFS *fs;
    int templen=0;
	xputs(FF_USE_LFN ? "LFN Enabled" : "LFN Disabled");
	xprintf(", Code page: %u\n", FF_CODE_PAGE);
    UINT Timer=1;
	for (;;) {
		xputc('>');
	    templen = xgets(Line, 256);
        Line[templen - 1] = 0;
		ptr = Line;
		switch (*ptr++) {
		case '?' :	/* Show Command List */
			xputs(HelpMsg);
			break;

		case 'm' :	/* Memory dump/fill/edit */
			switch (*ptr++) {
			case 'd' :	/* md[b|h|w] <address> [<count>] - Dump memory */
				switch (*ptr++) {
				case 'w': p3 = 4; break;
				case 'h': p3 = 2; break;
				default: p3 = 1;
				}
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) p2 = 128 / p3;
				for (ptr = (char*)p1; p2 >= 16 / p3; ptr += 16, p2 -= 16 / p3)
					put_dump(ptr, (DWORD)ptr, 16 / p3, p3);
				if (p2) put_dump((BYTE*)ptr, (UINT)ptr, p2, p3);
				break;
			case 'f' :	/* mf <address> <value> <count> - Fill memory */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				while (p3--) {
					*(BYTE*)p1 = (BYTE)p2;
					p1++;
				}
				break;
			case 'e' :	/* me[b|h|w] <address> [<value> ...] - Edit memory */
				switch (*ptr++) {	/* Get data width */
				case 'w': p3 = 4; break;
				case 'h': p3 = 2; break;
				default: p3 = 1;
				}
				if (!xatoi(&ptr, &p1)) break;	/* Get start address */
				if (xatoi(&ptr, &p2)) {	/* 2nd parameter is given (direct mode) */
					do {
						switch (p3) {
						case 4: *(DWORD*)p1 = (DWORD)p2; break;
						case 2: *(WORD*)p1 = (WORD)p2; break;
						default: *(BYTE*)p1 = (BYTE)p2;
						}
						p1 += p3;
					} while (xatoi(&ptr, &p2));	/* Get next value */
					break;
				}
				for (;;) {				/* 2nd parameter is not given (interactive mode) */
					switch (p3) {
					case 4: xprintf("%08X 0x%08X-", p1, *(DWORD*)p1); break;
					case 2: xprintf("%08X 0x%04X-", p1, *(WORD*)p1); break;
					default: xprintf("%08X 0x%02X-", p1, *(BYTE*)p1);
					}
					ptr = Line;templen = xgets(ptr, 256);
                    ptr[templen - 1] = 0;
					if (*ptr == '.') break;
					if ((BYTE)*ptr >= ' ') {
						if (!xatoi(&ptr, &p2)) continue;
						switch (p3) {
						case 4: *(DWORD*)p1 = (DWORD)p2; break;
						case 2: *(WORD*)p1 = (WORD)p2; break;
						default: *(BYTE*)p1 = (BYTE)p2;
						}
					}
					p1 += p3;
				}
				break;
			}
			break;

		case 'd' :	/* Disk I/O layer controls */
			switch (*ptr++) {
			case 'd' :	/* dd [<pd#> <sect>] - Dump secrtor */
				if (!xatoi(&ptr, &p1)) {
					p1 = drv; p2 = sect;
				} else {
					if (!xatoi(&ptr, &p2)) break;
				}
				drv = (BYTE)p1; sect = p2;
				res = disk_read(drv, Buff, sect, 1);
				if (res) { xprintf("rc=%d\n", (WORD)res); break; }
				xprintf("PD#:%u LBA:%lu\n", drv, sect++);
				for (ptr=(char*)Buff, ofs = 0; ofs < 0x200; ptr += 16, ofs += 16)
					put_dump((BYTE*)ptr, ofs, 16, 1);
				break;

			case 'i' :	/* di <pd#> - Initialize disk */
				if (!xatoi(&ptr, &p1)) break;
				xprintf("rc=%d\n", (WORD)disk_initialize((BYTE)p1));
				break;

			case 's' :	/* ds <pd#> - Show disk status */
				if (!xatoi(&ptr, &p1)) break;
				if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &p2) == RES_OK)
					{ xprintf("Drive size: %lu sectors\n", p2); }
				if (disk_ioctl((BYTE)p1, GET_BLOCK_SIZE, &p2) == RES_OK)
					{ xprintf("Block size: %lu sectors\n", p2); }
				if (disk_ioctl((BYTE)p1, MMC_GET_TYPE, &b) == RES_OK)
					{ xprintf("Media type: %u\n", b); }
				if (disk_ioctl((BYTE)p1, MMC_GET_CSD, Buff) == RES_OK)
					{ xputs("CSD:\n"); put_dump(Buff, 0, 16, 1); }
				if (disk_ioctl((BYTE)p1, MMC_GET_CID, Buff) == RES_OK)
					{ xputs("CID:\n"); put_dump(Buff, 0, 16, 1); }
				if (disk_ioctl((BYTE)p1, MMC_GET_OCR, Buff) == RES_OK)
					{ xputs("OCR:\n"); put_dump(Buff, 0, 4, 1); }
				if (disk_ioctl((BYTE)p1, MMC_GET_SDSTAT, Buff) == RES_OK) {
					xputs("SD Status:\n");
					for (s1 = 0; s1 < 64; s1 += 16) put_dump(Buff+s1, s1, 16, 1);
				}
				break;

			case 'c' :	/* Disk ioctl */
				switch (*ptr++) {
				case 's' :	/* dcs <pd#> - CTRL_SYNC */
					if (!xatoi(&ptr, &p1)) break;
					xprintf("rc=%d\n", disk_ioctl((BYTE)p1, CTRL_SYNC, 0));
					break;
				case 'e' :	/* dce <pd#> <s.lba> <e.lba> - CTRL_TRIM */
					if (!xatoi(&ptr, &p1) || !xatoi(&ptr, (int*)&blk[0]) || !xatoi(&ptr, (int*)&blk[1])) break;
					xprintf("rc=%d\n", disk_ioctl((BYTE)p1, CTRL_TRIM, blk));
					break;
				}
				break;
			}
			break;

		case 'b' :	/* Buffer controls */
			switch (*ptr++) {
			case 'd' :	/* bd <ofs> - Dump R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				for (ptr=(char*)&Buff[p1], ofs = p1, cnt = 32; cnt; cnt--, ptr+=16, ofs+=16)
					put_dump((BYTE*)ptr, ofs, 16, 1);
				break;

			case 'e' :	/* be <ofs> [<data>] ... - Edit R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				if (xatoi(&ptr, &p2)) {
					do {
						Buff[p1++] = (BYTE)p2;
					} while (xatoi(&ptr, &p2));
					break;
				}
				for (;;) {
					xprintf("%04X %02X-", (WORD)(p1), (WORD)Buff[p1]);
					templen = xgets(Line, 256);
                    Line[templen - 1] = 0;
					ptr = Line;
					if (*ptr == '.') break;
					if (*ptr < ' ') { p1++; continue; }
					if (xatoi(&ptr, &p2))
						Buff[p1++] = (BYTE)p2;
					else
						xputs("???\n");
				}
				break;

			case 'r' :	/* br <pd#> <lba> [<num>] - Read disk into R/W buffer */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				if (!xatoi(&ptr, &p3)) p3 = 1;
				xprintf("rc=%u\n", (WORD)disk_read((BYTE)p1, Buff, p2, p3));
				break;

			case 'w' :	/* bw <pd#> <lba> [<num>] - Write R/W buffer into disk */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				if (!xatoi(&ptr, &p3)) p3 = 1;
				xprintf("rc=%u\n", (WORD)disk_write((BYTE)p1, Buff, p2, p3));
				break;

			case 'f' :	/* bf <val> - Fill working buffer */
				if (!xatoi(&ptr, &p1)) break;
				memset(Buff, (BYTE)p1, Buff_LEN);
				break;

			}
			break;

		case 'f' :	/* FatFS API controls */
			switch (*ptr++) {

			case 'i' :	/* fi [<opt>]- Initialize logical drive */
				if (!xatoi(&ptr, &p2)) p2 = 0;
				put_rc(f_mount(FatFs, "", (BYTE)p2));
				break;

			case 's' :	/* fs [<path>] - Show volume status */
				while (*ptr == ' ') ptr++;
				res = f_getfree(ptr, (DWORD*)&p1, &fs);
				if (res) { put_rc(res); break; }
				xprintf("FAT type = %s\n", ft[fs->fs_type]);
				xprintf("Bytes/Cluster = %lu\n", (DWORD)fs->csize * 512);
				xprintf("Number of FATs = %u\n", fs->n_fats);
				if (fs->fs_type < FS_FAT32) xprintf("Root DIR entries = %u\n", fs->n_rootdir);
				xprintf("Sectors/FAT = %lu\n", fs->fsize);
				xprintf("Number of clusters = %lu\n", (DWORD)fs->n_fatent - 2);
				xprintf("Volume start (lba) = %lu\n", fs->volbase);
				xprintf("FAT start (lba) = %lu\n", fs->fatbase);
				xprintf("DIR start (lba,clustor) = %lu\n", fs->dirbase);
				xprintf("Data start (lba) = %lu\n\n", fs->database);
#if FF_USE_LABEL
				res = f_getlabel(ptr, (char*)Buff, (DWORD*)&p2);
				if (res) { put_rc(res); break; }
				xprintf(Buff[0] ? "Volume name is %s\n" : "No volume label\n", (char*)Buff);
				xprintf("Volume S/N is %04X-%04X\n", (DWORD)p2 >> 16, (DWORD)p2 & 0xFFFF);
#endif
				acc_size = acc_files = acc_dirs = 0;
				xprintf("...");
				res = scan_files(ptr, &acc_dirs, &acc_files, &acc_size);
				if (res) { put_rc(res); break; }
				xprintf("\r%u files, %llu bytes.\n%u folders.\n"
						"%lu KiB total disk space.\n%lu KiB available.\n",
						acc_files, acc_size, acc_dirs,
						(fs->n_fatent - 2) * (fs->csize / 2), (DWORD)p1 * (fs->csize / 2)
				);
				break;

			case 'l' :	/* fl [<path>] - Directory listing */
				while (*ptr == ' ') ptr++;
				res = f_opendir(Dir, ptr);
				if (res) { put_rc(res); break; }
				acc_size = acc_dirs = acc_files = 0;
				for(;;) {
					res = f_readdir(Dir, Finfo);
					if ((res != FR_OK) || !Finfo->fname[0]) break;
					if (Finfo->fattrib & AM_DIR) {
						acc_dirs++;
					} else {
						acc_files++; acc_size += Finfo->fsize;
					}
					xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s\n",
							(Finfo->fattrib & AM_DIR) ? 'D' : '-',
							(Finfo->fattrib & AM_RDO) ? 'R' : '-',
							(Finfo->fattrib & AM_HID) ? 'H' : '-',
							(Finfo->fattrib & AM_SYS) ? 'S' : '-',
							(Finfo->fattrib & AM_ARC) ? 'A' : '-',
							(Finfo->fdate >> 9) + 1980, (Finfo->fdate >> 5) & 15, Finfo->fdate & 31,
							(Finfo->ftime >> 11), (Finfo->ftime >> 5) & 63,
							Finfo->fsize, Finfo->fname);
				}
				xprintf("%4u File(s),%10llu bytes total\n%4u Dir(s)", acc_files, acc_size, acc_dirs);
				res = f_getfree(ptr, &dw, &fs);
				if (res == FR_OK) {
					xprintf(", %10llu bytes free\n", (QWORD)dw * fs->csize * 512);
				} else {
					put_rc(res);
				}
				break;

			case 'o' :	/* fo <mode> <file> - Open a file */
				if (!xatoi(&ptr, &p1)) break;
				while (*ptr == ' ') ptr++;
				put_rc(f_open(File[0], ptr, (BYTE)p1));
				break;

			case 'c' :	/* fc - Close a file */
				put_rc(f_close(File[0]));
				break;

			case 'e' :	/* fe - Seek file pointer */
				if (!xatoi(&ptr, &p1)) break;
				res = f_lseek(File[0], p1);
				put_rc(res);
				if (res == FR_OK) {
					xprintf("fptr=%lu(0x%lX)\n", File[0]->fptr, File[0]->fptr);
				}
				break;

			case 'd' :	/* fd <len> - read and dump file from current fp */
				if (!xatoi(&ptr, &p1)) break;
				ofs = File[0]->fptr;
				while (p1) {
					if ((UINT)p1 >= 16) { cnt = 16; p1 -= 16; }
					else 				{ cnt = p1; p1 = 0; }
					res = f_read(File[0], Buff, cnt, &cnt);
					if (res != FR_OK) { put_rc(res); break; }
					if (!cnt) break;
					put_dump(Buff, ofs, cnt, 1);
					ofs += 16;
				}
				break;

			case 'r' :	/* fr <len> - read file */
				if (!xatoi(&ptr, &p1)) break;
				p2 = 0;
				Timer = 1;
				while (p1) {
					if ((UINT)p1 >= blen) {
						cnt = blen; p1 -= blen;
					} else {
						cnt = p1; p1 = 0;
					}
					res = f_read(File[0], Buff, cnt, &s2);
					if (res != FR_OK) { put_rc(res); break; }
					p2 += s2;
					if (cnt != s2) break;
				}
				xprintf("%lu bytes read with %lu kB/sec.\n", p2, Timer ? (p2 / Timer) : 0);
				break;

			case 'w' :	/* fw <len> <val> - write file */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				memset(Buff, (BYTE)p2, blen);
				p2 = 0;
				Timer = 1;
				while (p1) {
					if ((UINT)p1 >= blen) {
						cnt = blen; p1 -= blen;
					} else {
						cnt = p1; p1 = 0;
					}
					res = f_write(File[0], Buff, cnt, &s2);
					if (res != FR_OK) { put_rc(res); break; }
					p2 += s2;
					if (cnt != s2) break;
				}
				xprintf("%lu bytes written with %lu kB/sec.\n", p2, Timer ? (p2 / Timer) : 0);
				break;

			case 'n' :	/* fn <org.name> <new.name> - Change name of an object */
				while (*ptr == ' ') ptr++;
				ptr2 = strchr(ptr, ' ');
				if (!ptr2) break;
				*ptr2++ = 0;
				while (*ptr2 == ' ') ptr2++;
				put_rc(f_rename(ptr, ptr2));
				break;

			case 'u' :	/* fu <name> - Unlink an object */
				while (*ptr == ' ') ptr++;
				put_rc(f_unlink(ptr));
				break;

			case 'v' :	/* fv - Truncate file */
				put_rc(f_truncate(File[0]));
				break;

			case 'k' :	/* fk <name> - Create a directory */
				while (*ptr == ' ') ptr++;
				put_rc(f_mkdir(ptr));
				break;

			case 'a' :	/* fa <atrr> <mask> <name> - Change attribute of an object */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				while (*ptr == ' ') ptr++;
				put_rc(f_chmod(ptr, p1, p2));
				break;

			case 't' :	/* ft <year> <month> <day> <hour> <min> <sec> <name> - Change timestamp of an object */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				Finfo->fdate = ((p1 - 1980) << 9) | ((p2 & 15) << 5) | (p3 & 31);
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				Finfo->ftime = ((p1 & 31) << 11) | ((p2 & 63) << 5) | ((p3 >> 1) & 31);
				put_rc(f_utime(ptr, Finfo));
				break;

			case 'x' : /* fx <src.name> <dst.name> - Copy a file */
				while (*ptr == ' ') ptr++;
				ptr2 = strchr(ptr, ' ');
				if (!ptr2) break;
				*ptr2++ = 0;
				while (*ptr2 == ' ') ptr2++;
				xprintf("Opening \"%s\"", ptr);
				res = f_open(File[0], ptr, FA_OPEN_EXISTING | FA_READ);
				xputc('\n');
				if (res) {
					put_rc(res);
					break;
				}
				xprintf("Creating \"%s\"", ptr2);
				res = f_open(File[1], ptr2, FA_CREATE_ALWAYS | FA_WRITE);
				xputc('\n');
				if (res) {
					put_rc(res);
					f_close(File[0]);
					break;
				}
				xprintf("Copying file...");
				Timer = 1;
				p1 = 0;
				for (;;) {
					res = f_read(File[0], Buff, blen, &s1);
					if (res || s1 == 0) break;   /* error or eof */
					res = f_write(File[1], Buff, s1, &s2);
					p1 += s2;
					if (res || s2 < s1) break;   /* error or disk full */
				}
				xprintf("\n%lu bytes copied with %lu kB/sec.\n", p1, p1 / Timer);
				f_close(File[0]);
				f_close(File[1]);
				break;
#if FF_FS_RPATH
			case 'g' :	/* fg <path> - Change current directory */
				while (*ptr == ' ') ptr++;
				put_rc(f_chdir(ptr));
				break;
#if FF_FS_RPATH >= 2
			case 'q' :	/* fq - Show current dir path */
				res = f_getcwd(Line, 256);
				if (res)
					put_rc(res);
				else
					xprintf("%s\n", Line);
				break;
#endif
#endif
#if FF_USE_LABEL
			case 'b' :	/* fb <name> - Set volume label */
				while (*ptr == ' ') ptr++;
				put_rc(f_setlabel(ptr));
				break;
#endif	/* FF_USE_LABEL */
#if FF_USE_MKFS
			case 'm' :	/* fm [<fs type> [<au size> [<align> [<n_fats> [<n_root>]]]]] - Create filesystem */
				{
					MKFS_PARM opt, *popt = 0;

					if (xatoi(&ptr, &p2)) {
						memset(&opt, 0, sizeof(MKFS_PARM));
						popt = &opt;
						popt->fmt = (BYTE)p2;
						if (xatoi(&ptr, &p2)) {
							popt->au_size = p2;
							if (xatoi(&ptr, &p2)) {
								popt->align = p2;
								if (xatoi(&ptr, &p2)) {
									popt->n_fat = (BYTE)p2;
									if (xatoi(&ptr, &p2)) {
										popt->n_root = p2;
									}
								}
							}
						}
					}
					xprintf("The volume will be formatted. Are you sure? (Y/n)=");
					templen = xgets(Line, 256);
                    Line[templen - 1] = 0;
					if (Line[0] == 'Y') put_rc(f_mkfs("", popt, Buff, Buff_LEN));
					break;
				}
#endif	/* FF_USE_MKFS */
			case 'z' :	/* fz [<size>] - Change/Show R/W length for fr/fw/fx command */
				if (xatoi(&ptr, &p1) && p1 >= 1 && p1 <= (int)Buff_LEN)
					blen = p1;
				xprintf("blen=%u\n", blen);
				break;
			}
			break;

		case 't' :	/* t [<year> <mon> <mday> <hour> <min> <sec>] - Set/Show RTC */
        {
            SYSTEMTIME currtime;
	        memset(&currtime,0,sizeof(SYSTEMTIME));
	        getCmosDateTime(&currtime);
	        printf("year:%d month:%d day:%d hour:%d minute:%d second:%d\n",currtime.wYear,currtime.wMonth,currtime.wDay,
	        currtime.wHour,currtime.wMinute,currtime.wSecond);
			break;
        }
		case 'v':
		{
			while (*ptr == ' ') ptr++;
			wirteVBEinfo(ptr);
		}
		}
	}
}
void wirteVBEinfo(char *filename)
{

}
#else
static FATFS* gFatFs = NULL;   //一定是一个全局变量
//static DIR* gCurrentDir = NULL;
void initFs()
{
	gFatFs = kernel_malloc(sizeof(FATFS));
	memset(gFatFs, 0, sizeof(FATFS));
	//gCurrentDir = kernel_malloc(sizeof(DIR));
	//memset(gCurrentDir, 0, sizeof(DIR));

	FRESULT  res;   //局部变量
	disk_initialize(0);

	res = f_mount(gFatFs, "", 0);   //挂载文件系统 ， "1:"就是挂载的设备号为1的设备
	if (res != FR_OK)  //FR_NO_FILESYSTEM值为13，表示没有有效的设备
	{
		//fswork = kernel_malloc(FF_MAX_SS);
		//res = f_mkfs("0:", 0, fswork, FF_MAX_SS);
		// printf("f_mkfs  is  over\r\n");
		// printf("res = %d\r\n",res);
		//res = f_mount(NULL, "0:", 0);   //取消文件系统
		//res = f_mount(&fsobject, "0:", 0);   //挂载文件系统
		printf("initFs mount error\n");
		return;
	}
	//res = f_opendir(gCurrentDir, "");
	//if (res != FR_OK)
	//{
	//	printf("initFs opendir error\n");
	//	return;
//	}

	/*
	char* testbuff = kernel_malloc(1024);
	uint32_t br = 0, wr;

	res = f_open(&fp, "0:diskdata.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	printf("open file res = %d \n", res);

	if (res == FR_OK)
	{
		res = f_read(&fp, testbuff, f_size(&fp), &br);
		if (res == FR_OK)
		{
			testbuff[br] = 0;
			printf(" read data size %d  %s\n", br, testbuff);
		}
		f_close(&fp);
	}	
	res = f_open(&fp, "0:diskdata1.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	printf("open w file res = %d \n", res);

	if (res == FR_OK)
	{
		res = f_write(&fp, testbuff, br, &wr);
		if (res == FR_OK)
		{
			printf(" wirte data success\n");
		}
		f_close(&fp);
	}
	DWORD count = 0;
	f_getfree("0", &count, &fsobject);
	printf("empty sec count:0x%x\n", count);

	*/
}

#endif