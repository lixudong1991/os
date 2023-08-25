global read_sectors_max_256,read_ata_sectors
extern bootparam

read_sectors_max_256:                           	;从硬盘读取一个逻辑扇区
         push ebp                           ;arg0目标缓冲区地址
         mov ebp,esp                        ;arg1逻辑扇区号
											;arg2读取的扇区数
         push edi                                  ;返回：EAX=EBX+512*cl
         push ecx
         push edx
         push ebx
      
         mov ebx,[ebp+0x8]
         mov cl,[ebp+0x10]
		 mov ax,256
		 mov ch,0
		 mul cx
		 movzx edi,ax
		 

         mov dx,0x1f2
         mov al,cl
         out dx,al                          ;读取的扇区数

         inc dx                             ;0x1f3
         mov eax,[ebp+0xc]
         out dx,al                          ;LBA地址7~0

         inc dx                             ;0x1f4
         mov cl,8
         shr eax,cl
         out dx,al                          ;LBA地址15~8

         inc dx                             ;0x1f5
         shr eax,cl
         out dx,al                          ;LBA地址23~16

         inc dx                             ;0x1f6
         shr eax,cl
         or al,0xe0                         ;第一硬盘  LBA地址27~24
         out dx,al

         inc dx                             ;0x1f7
         mov al,0x20                        ;读命令
         out dx,al

  .waits:
         in al,dx
         and al,0x88
         cmp al,0x08
         jnz .waits                         ;不忙，且硬盘已准备好数据传输 

         mov ecx,edi                        ;总共要读取的字数
         mov dx,0x1f0
  .readw:
         in ax,dx
         mov [ebx],ax
         add ebx,2
         loop .readw
      
         sub ebx,[ebp+0x8]
         mov eax,ebx
         pop ebx
         pop edx
         pop ecx
		 pop edi
         pop ebp
         ret  

dd_prtlen equ 0  
dd_dcr equ 4
dd_stLBA equ 6
dd_tf equ 10
dd_sbits  equ 12


read_ata_sectors:
	push ebp
	mov ebp,esp
	push ebx
	push edi
	push esi
	mov ebx,[ebp+0x10]
	mov edi,[ebp+0x8]
	mov esi,prtlen
	mov eax,[bootparam+656]
	mov [esi],eax
	mov ebp,[ebp+0xc]
	mov eax,ebx
	call read_ata_st
	jle readataret
	xor eax,eax
readataret:
	pop esi
	pop edi
	pop ebx
	pop ebp
	ret



; do a singletasking PIO ATA read
; inputs: ebx = # of sectors to read, edi -> dest buffer, esi -> driverdata struct, ebp = 4b LBA
; Note: ebp is a "relative" LBA -- the offset from the beginning of the partition
; outputs: ebp, edi incremented past read; ebx = 0
; flags: zero flag set on success, carry set on failure (redundant)	
read_ata_st:
	push edx
	push ecx
	push eax
	test ebx, ebx			; # of sectors < 0 is a "reset" request from software
	js short .reset
	cmp ebx, 0x3fffff		; read will be bigger than 2GB? (error)
	stc
	jg short .r_don
	mov edx, [esi + dd_prtlen]	; get the total partition length (sectors)
	dec edx				; (to avoid handling "equality" case)
	cmp edx, ebp			; verify ebp is legal (within partition limit)
	jb short .r_don			; (carry is set automatically on an error)
	cmp edx, ebx			; verify ebx is legal (forget about the ebx = edx case)
	jb short .r_don
	sub edx, ebx			; verify ebp + ebx - 1 is legal
	inc edx
	cmp edx, ebp			; (the test actually checks ebp <= edx - ebx + 1)
	jb short .r_don
	mov dx, [esi + dd_dcr]		; dx = alt status/DCR
	in al, dx			; get the current status
	test al, 0x88			; check the BSY and DRQ bits -- both must be clear
	je short .stat_ok
.reset:
	call srst_ata_st
	test ebx, ebx			; bypass any read on a "reset" request
	jns short .stat_ok
	xor ebx, ebx			; force zero flag on, carry clear
	jmp short .r_don
.stat_ok:
; preferentially use the 28bit routine, because it's a little faster
; if ebp > 28bit or esi.stLBA > 28bit or stLBA+ebp > 28bit or stLBA+ebp+ebx > 28bit, use 48 bit
	cmp ebp, 0xfffffff
	jg short .setreg
	mov eax, [esi + dd_stLBA]
	cmp eax, 0xfffffff
	jg short .setreg
	add eax, ebp
	cmp eax, 0xfffffff
	jg short .setreg
	add eax, ebx
	cmp eax, 0xfffffff
.setreg:
	mov dx, [esi + dd_tf]		; dx = IO port base ("task file")
	jle short .read28		; test the flags from the eax cmp's above
.read48:
	test ebx, ebx		; no more sectors to read?
	je short .r_don
	call pio48_read		; read up to 256 more sectors, updating registers
	je short .read48	; if successful, is there more to read?
	jmp short .r_don
.read28:
	test ebx, ebx		; no more sectors to read?
	je short .r_don
	call pio28_read		; read up to 256 more sectors, updating registers
	je short .read28	; if successful, is there more to read?
.r_don:
	pop eax
	pop ecx
	pop edx
	ret
 
 
;ATA PI0 28bit singletasking disk read function (up to 256 sectors)
; inputs: ESI -> driverdata info, EDI -> destination buffer
; BL = sectors to read, DX = base bus I/O port (0x1F0, 0x170, ...), EBP = 28bit "relative" LBA
; BSY and DRQ ATA status bits must already be known to be clear on both slave and master
; outputs: data stored in EDI; EDI and EBP advanced, EBX decremented
; flags: on success Zero flag set, Carry clear
pio28_read:
	add ebp, [esi + dd_stLBA]	; convert relative LBA to absolute LBA
	mov ecx, ebp			; save a working copy
	mov al, bl		; set al= sector count (0 means 256 sectors)
	or dl, 2		; dx = sectorcount port -- usually port 1f2
	out dx, al
	mov al, cl		; ecx currently holds LBA
	inc edx			; port 1f3 -- LBAlow
	out dx, al
	mov al, ch
	inc edx			; port 1f4 -- LBAmid
	out dx, al
	bswap ecx
	mov al, ch		; bits 16 to 23 of LBA
	inc edx			; port 1f5 -- LBAhigh
	out dx, al
	mov al, cl			; bits 24 to 28 of LBA
	or al, byte [esi + dd_sbits]	; master/slave flag | 0xe0
	inc edx				; port 1f6 -- drive select
	out dx, al
 
	inc edx			; port 1f7 -- command/status
	mov al, 0x20		; send "read" command to drive
	out dx, al
 
; ignore the error bit for the first 4 status reads -- ie. implement 400ns delay on ERR only
; wait for BSY clear and DRQ set
	mov ecx, 4
.lp1:
	in al, dx		; grab a status byte
	test al, 0x80		; BSY flag set?
	jne short .retry
	test al, 8		; DRQ set?
	jne short .data_rdy
.retry:
	dec ecx
	jg short .lp1
; need to wait some more -- loop until BSY clears or ERR sets (error exit if ERR sets)
 
.pior_l:
	in al, dx		; grab a status byte
	test al, 0x80		; BSY flag set?
	jne short .pior_l	; (all other flags are meaningless if BSY is set)
	test al, 0x21		; ERR or DF set?
	jne short .fail
.data_rdy:
; if BSY and ERR are clear then DRQ must be set -- go and read the data
	sub dl, 7		; read from data port (ie. 0x1f0)
	mov cx, 256
	rep insw		; gulp one 512b sector into edi
	or dl, 7		; "point" dx back at the status register
	in al, dx		; delay 400ns to allow drive to set new values of BSY and DRQ
	in al, dx
	in al, dx
	in al, dx
 
; After each DRQ data block it is mandatory to either:
; receive and ack the IRQ -- or poll the status port all over again
 
	inc ebp			; increment the current absolute LBA
	dec ebx			; decrement the "sectors to read" count
	test bl, bl		; check if the low byte just turned 0 (more sectors to read?)
	jne short .pior_l
 
	sub dx, 7		; "point" dx back at the base IO port, so it's unchanged
	sub ebp, [esi + dd_stLBA]	; convert absolute lba back to relative
; "test" sets the zero flag for a "success" return -- also clears the carry flag
	test al, 0x21		; test the last status ERR bits
	je short .done
.fail:
	stc
.done:
	ret
 
 
;ATA PI0 33bit singletasking disk read function (up to 64K sectors, using 48bit mode)
; inputs: bx = sectors to read (0 means 64K sectors), edi -> destination buffer
; esi -> driverdata info, dx = base bus I/O port (0x1F0, 0x170, ...), ebp = 32bit "relative" LBA
; BSY and DRQ ATA status bits must already be known to be clear on both slave and master
; outputs: data stored in edi; edi and ebp advanced, ebx decremented
; flags: on success Zero flag set, Carry clear
pio48_read:
	xor eax, eax
	add ebp, [esi + dd_stLBA]	; convert relative LBA to absolute LBA
; special case: did the addition overflow 32 bits (carry set)?
	adc ah, 0			; if so, ah = LBA byte #5 = 1
	mov ecx, ebp			; save a working copy of 32 bit absolute LBA
 
; for speed purposes, never OUT to the same port twice in a row -- avoiding it is messy but best
;outb (0x1F2, sectorcount high)
;outb (0x1F3, LBA4)
;outb (0x1F4, LBA5)			-- value = 0 or 1 only
;outb (0x1F5, LBA6)			-- value = 0 always
;outb (0x1F2, sectorcount low)
;outb (0x1F3, LBA1)
;outb (0x1F4, LBA2)
;outb (0x1F5, LBA3)
	bswap ecx		; make LBA4 and LBA3 easy to access (cl, ch)
	or dl, 2		; dx = sectorcount port -- usually port 1f2
	mov al, bh		; sectorcount -- high byte
	out dx, al
	mov al, cl
	inc edx
	out dx, al		; LBA4 = LBAlow, high byte (1f3)
	inc edx
	mov al, ah		; LBA5 was calculated above
	out dx, al		; LBA5 = LBAmid, high byte (1f4)
	inc edx
	mov al, 0		; LBA6 is always 0 in 32 bit mode
	out dx, al		; LBA6 = LBAhigh, high byte (1f5)
 
	sub dl, 3
	mov al, bl		; sectorcount -- low byte (1f2)
	out dx, al
	mov ax, bp		; get LBA1 and LBA2 into ax
	inc edx
	out dx, al		; LBA1 = LBAlow, low byte (1f3)
	mov al, ah		; LBA2
	inc edx
	out dx, al		; LBA2 = LBAmid, low byte (1f4)
	mov al, ch		; LBA3
	inc edx
	out dx, al		; LBA3 = LBAhigh, low byte (1f5)
 
	mov al, byte [esi + dd_sbits]	; master/slave flag | 0xe0
	inc edx
	and al, 0x50		; get rid of extraneous LBA28 bits in drive selector
	out dx, al		; drive select (1f6)
 
	inc edx
	mov al, 0x24		; send "read ext" command to drive
	out dx, al		; command (1f7)
 
; ignore the error bit for the first 4 status reads -- ie. implement 400ns delay on ERR only
; wait for BSY clear and DRQ set
	mov ecx, 4
.lp1:
	in al, dx		; grab a status byte
	test al, 0x80		; BSY flag set?
	jne short .retry
	test al, 8		; DRQ set?
	jne short .data_rdy
.retry:
	dec ecx
	jg short .lp1
; need to wait some more -- loop until BSY clears or ERR sets (error exit if ERR sets)
 
.pior_l:
	in al, dx		; grab a status byte
	test al, 0x80		; BSY flag set?
	jne short .pior_l	; (all other flags are meaningless if BSY is set)
	test al, 0x21		; ERR or DF set?
	jne short .fail
.data_rdy:
; if BSY and ERR are clear then DRQ must be set -- go and read the data
	sub dl, 7		; read from data port (ie. 0x1f0)
	mov cx, 256
	rep insw		; gulp one 512b sector into edi
	or dl, 7		; "point" dx back at the status register
	in al, dx		; delay 400ns to allow drive to set new values of BSY and DRQ
	in al, dx
	in al, dx
	in al, dx
 
; After each DRQ data block it is mandatory to either:
; receive and ack the IRQ -- or poll the status port all over again
 
	inc ebp			; increment the current absolute LBA (overflowing is OK!)
	dec ebx			; decrement the "sectors to read" count
	test bx, bx		; check if "sectorcount" just decremented to 0
	jne short .pior_l
 
	sub dx, 7		; "point" dx back at the base IO port, so it's unchanged
	sub ebp, [esi + dd_stLBA]	; convert absolute lba back to relative
; this sub handles the >32bit overflow cases correcty, too
; "test" sets the zero flag for a "success" return -- also clears the carry flag
	test al, 0x21		; test the last status ERR bits
	je short .done
.fail:
	stc
.done:
	ret
 
 
; do a singletasking PIO ata "software reset" with DCR in dx
srst_ata_st:
	push eax
	mov al, 4
	out dx, al			; do a "software reset" on the bus
	xor eax, eax
	out dx, al			; reset the bus to normal operation
	in al, dx			; it might take 4 tries for status bits to reset
	in al, dx			; ie. do a 400ns delay
	in al, dx
	in al, dx
.rdylp:
	in al, dx
	and al, 0xc0			; check BSY and RDY
	cmp al, 0x40			; want BSY clear and RDY set
	jne short .rdylp
	pop eax
	ret

prtlen dd 0
dcr	 dw 0x1f7
stLBA dd 0
tf  	 dw 0x1F0
sbits db 0xe0