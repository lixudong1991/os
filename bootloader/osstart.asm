prload: xor ax,ax
		mov es,ax
		mov ss,ax
		mov sp,7c00h
		xor dx,dx
		mov ax, osstartend -  osstart
		mov bx,512
		div bx
		cmp dx,0
		je r
		inc ax
r:		mov bx,0600h	
		mov ch,0
		mov cl,2
		mov dl,80h
		mov dh,0
		mov ah,2
		int 13h
		mov ax,0
		push ax
		mov ax,0600h
		push ax
		retf
		times 446-($-$$) db 0

fsActive		db 0x80
fsStartHead		db 0x20
fsStartCylSect	dw 0x21
fsPartType		db 0x0c
fsEndHead		db 0xfe
fsEndCylSect 	dw 0xffff
fsStartLBA		dd 0x800
fsSize			dd 0x4000000-0x800
; fsActive		db 0x00
; fsStartHead		db 0x20
; fsStartCylSect	dw 0x21
; fsPartType		db 0x07
; fsEndHead		db 0xfe
; fsEndCylSect 	dw 0xffff
; fsStartLBA		dd 0x800
; fsSize			dd 0x1d1c4800
		times 48 db 0
		db 55h,0aah
osstart:mov ax,0
		mov ds,ax
		mov ss,ax
		mov sp,07c00h
	
		call clr
		mov cl,111b
		mov dl,34	
		mov dh,10
		mov si,0600h+ dataa -  osstart
		call showstr
		mov dh,11
		mov si,0600h+ datab -  osstart
		call showstr
		mov dh,12
		mov si,0600h+ datac -  osstart
		call showstr
		mov dh,13
		mov si,0600h+ datad -  osstart
		call showstr
		call clear_buff
		mov bh,0
		mov dh,10
		mov dl,34
set:	mov ah,2
		int 10h
		mov ah,0
		int 16h
		cmp ah,48h
		je set0
		cmp ah,50h
		je set1	
		cmp ah,1ch
		je set2
		jmp short set
set0:	cmp dh,10
		jna set
		dec dh	
		jmp short set
set1:	cmp dh,13
		jnb set
		inc dh
		jmp short set	
set2:	call clr
		sub dh,10
		mov bl,dh
		mov bh,0
		add bx,bx
		add bx,bx
		mov si,0600h+ table -  osstart
		jmp far [si+bx]	
		
rst:	mov ax,0ffffh
		push ax
		mov ax,0
		push ax
		retf
		
showtms:call setesc	
		call printMemInfo
		mov si,0600h+ tmstr -  osstart
showtm1:call readtms
		mov dh,0
		mov dl,31
		mov bx,0600h+ timecolor -  osstart
		mov cl,[bx]
		call showstr
		jmp short showtm1
edittm: mov dh,3
		mov dl,20
		mov cl,111b
		mov si,0600h+ writetmmsg -  osstart
		call showstr
		mov dh,4
		mov si,0600h+ writetmmsg1 -  osstart
		call showstr
		mov dh,5
		mov si,0600h+ writetmmsg2 -  osstart
		call showstr
		mov dh,6
		mov si,0600h+ writetmmsg3 -  osstart
		call showstr
		mov si,0600h+ tmstr -  osstart
		call readtms
		mov dh,12
		mov dl,31
		mov bx,0600h+ timecolor -  osstart
		mov cl,[bx]
		call showstr
		
		mov bh,0
		mov ah,2
		int 10h
		
edittm1:mov ah,0
		int 16h
		cmp ah,4bh
		je mright
		cmp ah,4dh
		je mleft
		cmp ah,1
		je edittm3
		cmp ah,1ch
		je edittm2
		cmp al,30h
		jb edittm1
		cmp al,39h
		ja edittm1
		sub bx,bx
		mov bl,dl
		sub bl,31
		cmp byte [si+bx],30h
		jb edittm1
		cmp byte [si+bx],39h
		ja edittm1	
		mov [si+bx],al
		mov bl,dl
		mov dl,31
		call showstr
		mov dl,bl
		jmp edittm1
mright:	cmp dl,31
		jna edittm1
		dec dl
		mov ah,2
		int 10h
		jmp edittm1
mleft:	cmp dl,47
		jnb edittm1
		inc dl
		mov ah,2
		int 10h
		jmp edittm1
edittm2:call writetm
		jmp edittm1
edittm3:mov ax,0
		push ax
		mov ax,0600h
		push ax
		retf
readtms:push ax
		push bx
		push cx
		push si
		push es
		xor ax,ax
		mov es,ax	
		mov bx,0600h+ tmindex -  osstart
		mov cx,6
tms:	mov al,[es:bx]
		out 70h,al
		in al,71h
		mov ah,al
		push cx
		mov cl,4
		shr al,cl
		add al,30h
		mov [si],al
		and ah,1111b
		add ah,30h
		inc si
		mov [si],ah
		add si,2
		inc bx
		pop cx
		loop tms
		pop es
		pop si
		pop cx
		pop bx
		pop ax
		ret
writetm:push ax
		push bx
		push cx
		push si
		push es
		xor ax,ax
		mov es,ax	
		mov bx,0600h+ tmindex -  osstart
		mov cx,6
wtms1:	mov al,[es:bx]
		out 70h,al
		mov al,[si]
		sub al,30h
		push cx
		mov cl,4
		shl al,cl
		inc si
		mov ah,[si]
		sub ah,30h
		or al,ah
		out 71h,al
		add si,2
		inc bx
		pop cx
		loop wtms1
		pop es
		pop si
		pop cx
		pop bx
		pop ax
		ret
			
setesc:	push ax
		push ds
		push si
		xor ax,ax
		mov ds,ax
		mov si,0600h+ int9a -  osstart
		mov ax,[36]
		mov [si],ax
		mov ax,[38]
		mov [si+2],ax
		cli 
		mov word  [36],0600h+ doesc -  osstart
		mov word  [38],0
		sti
		pop si
		pop ds
		pop ax
		ret
		
doesc:	push ax
		push ds
		push si
		push bp
		mov bp,sp
		xor ax,ax
		mov ds,ax
		mov si,0600h+ int9a -  osstart
		in al,60h
		pushf
		call far [si]
		cmp al,1
		je esc1
		cmp al,3bh
		jne i9r
		mov si,0600h+ timecolor -  osstart
		inc byte [si]
		jmp short i9r
esc1:	mov ax, [si]
		mov [36],ax
		mov ax,[si+2]
		mov [38],ax
		mov word [bp+8],00600h
		mov word [bp+0ah],0
i9r:	pop bp
		pop si
		pop ds
		pop ax
		iret	
		
clr:	push es
		push cx
		push si
		mov cx,0b800h
		mov es,cx
		mov cx,2000
		xor si,si
clrs:	mov word [es:si],720h;清空屏幕内容，并设置黑底白字
		add si,2
		loop clrs
		pop si
		pop cx
		pop es
		ret	
		
clear_buff:
		push ax
clear_buff_begin:
		mov ah,1
		int 16h		;用16h中断例程的1号子程序检查键盘缓冲区是否有数据（ZF是否等于0），且没有等待输入。
		jz clear_buff_ret	;当键盘缓冲区有数据时调用16h中断例程的0号子程序逐个清除缓冲区的数据，缓冲区为空时返回。
		mov ah,0
		int 16h
		jmp clear_buff_begin
clear_buff_ret:
		pop ax
		ret		
showstr:push ax
		push es
		mov ax,0b800h
		mov es,ax
		push si
		push bp
		mov ax,160
		mul dh
		mov bp,ax
		mov ax,2
		mul dl
		add bp,ax
		mov ah,cl
		push cx	
		xor cx,cx
str0:	mov al,[si]
		mov cl,al
		jcxz str1
		mov [es:bp],ax
		inc si
		add bp,2
		jmp short str0
str1:	pop cx
		pop bp
		pop si
		pop es
		pop ax
		ret	
		

getMemInfo:
		push eax
		push ebx
		push ecx
		push edx
		push si
		push bp
		mov si,di
		xor bp,bp
		xor ebx,ebx
getMemInfo0:
		mov eax,0xe820
		mov ecx,20
		mov edx,0x534d4150
		int 15h
		jb err
		cmp eax,0x534d4150
		jne err
		cmp ecx,20
		jne err
		inc bp
		cmp ebx,0
		je getMemInfo1
		add di,20
		jmp getMemInfo0
err:
		mov di,si
		xor bp,bp
getMemInfo1:
		mov [0600h+memInfoDataSize-osstart],bp
		pop bp
		pop si
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
			
divdw:	push bx
		push ax
		mov  ax,dx
		sub dx,dx
		div cx
		mov bx,ax
		pop ax
		div cx
		mov cx,dx
		mov dx,bx
		pop bx
		ret 
itohstr:push bx
		push cx
		push di
		mov bx,si
		mov cx,8
itohstr0:		
		mov byte [bx],'0'
		inc bx
		loop itohstr0
		mov byte [bx],0
dtoc0:	mov cx,0x10
		call divdw
		movzx di,cl
		mov cl,[di+0600h+hexstrnum-osstart]
		dec bx			
		mov [bx],cl
		mov cx,ax
		jcxz dtoc1
		jmp short dtoc0
dtoc1:	mov cx,dx
		jcxz dtoc2
		jmp short dtoc0
dtoc2:	
		pop di
		pop cx
		pop bx
		ret
	
printMemInfo:
		push ax
		push bx
		push dx
		push es
		push ds
		push di
		call clr
		mov ax,0
		mov es,ax
		mov ds,ax
		mov di,0600h+memInfoData-osstart
		call getMemInfo
		cmp di,0600h+memInfoData-osstart
		je printMemInfoerr
		mov bx,0600h+memInfoData-osstart
			
		mov dl,10	
		mov dh,1
		mov cl,111b		
		mov si,0600h+hexstrbuff-osstart		
printMemInfo0:
		
		push dx
		mov ax,[bx+4]
		mov dx,[bx+6]
		call itohstr
		pop dx
		call showstr
		push dx
		mov ax,[bx]
		mov dx,[bx+2]
		call itohstr
		pop dx
		add dl,9
		call  showstr
		
		push dx
		mov ax,[bx+12]
		mov dx,[bx+14]
		call itohstr
		pop dx
		add dl,12
		call showstr
		push dx
		mov ax,[bx+8]
		mov dx,[bx+10]
		call itohstr
		pop dx
		add dl,9
		call showstr
		
		;add dl,12
		;mov si,0600h+ meminfostr -  osstart
		;mov ax,0600h+ meminfostr1 -  osstart	
		;cmp dword [bx+16],1 
		;cmovnz si,ax
		push dx
		mov ax,[bx+16]
		mov dx,[bx+18]
		call itohstr
		pop dx
		add dl,12
		call showstr
		mov si,0600h+ hexstrbuff -  osstart
		sub dl,42
		add dh,1
		cmp bx,di
		je printMemInforet
		add bx,20
		jmp printMemInfo0
printMemInfoerr:
		mov cl,111b
		mov dl,34	
		mov dh,10
		mov si,0600h+ meminfoerrstr -  osstart
		call showstr
printMemInforet:
		pop di
		pop ds
		pop es
		pop dx
		pop bx
		pop ax
		ret
setDescriptor:
		push bp
		mov bp,sp
		push ax
		push dx
		and word [si+2],0	
		and byte [si+4],0
		and byte [si+7],0
		
		mov ax,[bp+8]
		mov dx,[bp+10]
		or  [si+2],ax
		or  [si+4],dl
		or  [si+7],dh
		 
		and word [si],0
		and byte [si+6],11110000b
		
		mov ax,[bp+4]
		mov dx,[bp+6]
		sub ax,1
		sbb dl,0
		and dl,0xf
		or [si],ax
		or [si+6],dl
		
		and byte [si+5],11110000b
		and dh,0xf
		or [si+5],dh
		
		pop dx
		pop ax
		pop bp
		ret 8	
	
;-------------------------------------------------------------------------------
read_hard_disk_0:                        ;从硬盘读取ax个逻辑扇区
       push ax                                  ;输入：DI:SI=起始逻辑扇区号
	   push ds									; ES=目标缓冲区段地址	   											
	   push si
	   push dx
	   push cx
	   mov cx,ax 
	   xor ax,ax
	   mov ds,ax
	   mov word [0600h+bufferoff-osstart],0
	   mov ax,es
	   mov [0600h+bufferseg-osstart],ax
	   mov [0600h+blockNum-osstart],si
	   mov [0600h+blockNum-osstart+2],di

nextread64sec:
	   push cx
	   mov word [0600h+readsectioncount-osstart],64
	   cmp cx,64
	   ja read_hard_disk_max64
	   mov [0600h+readsectioncount-osstart],cx
read_hard_disk_max64:
	   mov ah,42h
	   mov dl,80h
	   mov si,0600h+packet-osstart
	   int 13h
	   jb read_hard_disk_0_ret
	   pop cx
	   sub cx,[0600h+readsectioncount-osstart]
	   add word [0600h+bufferseg-osstart],0x800
	   mov word [0600h+bufferoff-osstart],0
	   add dword [0600h+blockNum-osstart],64
	   cmp cx,0
	   jne nextread64sec
read_hard_disk_0_ret:	
	   pop cx
	   pop dx
	   pop si
	   pop ds
	   pop ax
	   ret		
	   
read_DriveParam:
		push dx
		push ds
		push si
		xor ax,ax
		mov ds,ax
		mov ah,48h
		mov dl,80h
		mov si,0600h+InfoSize-osstart
		int 13h
		pop si
		pop ds
		pop dx
		ret

setVGAPalete:
		push dx
		push ax
		push bx
		push ds
		push cx
		mov ax,0x0013
		int 0x10
		xor ax,ax
		mov ds,ax

		mov dx,0x03c8
		out dx,al

		mov cx,48
		mov bx,0600h+table_rgb-osstart
		mov dx,0x03c9
		
setVGAPalete1:
		mov al,[bx]
		shr al,2
		out dx,al
		inc bx
		loop setVGAPalete1
		pop cx
		pop ds
		pop bx
		pop ax
		pop dx
		ret

;ds:di 存储XResolution YResolutiond的指针 
;输入 ax 模式号
;ax 返回状态
getvbemodeResolution:
		push es
		push cx
		push di
		push ax
		xor ax,ax
		mov es,ax
		mov di,0600h+vbe_mode_info_structure-osstart
		pop cx
		mov ax,0x4f01
		int 0x10
		pop di
		cmp ax,0x004f
		jne getvbemodeResolutionret	
		mov cx,[0600h+vbe_mode_info_structure+18-osstart] ;XResolution
		mov [ds:di],cx
		mov cx,[0600h+vbe_mode_info_structure+20-osstart] ;YResolution
		mov [ds:di+2],cx
getvbemodeResolutionret:	
		pop cx
		pop es
		ret	

kernelSectionCount      equ      600    ;用户程序1+用户程序2+kernel的总扇区数
kernelStartSection      equ      16     ;内核加载起始扇区
kernelLoadSeg 			equ      0x3b00 ;内核加载内存起始地址
kernelVirAddr			equ      0xc0000000
startPhyPage 			equ      0x100

apcodeSectionCount      equ      8
apcodeStartSection      equ      616     ;ap加载起始扇区
apLoadSeg 			    equ      0x8600 ;ap加载内存起始地址


;fontSectionCount      equ      232
;fontStartSection      equ      320     ;加载起始扇区
;fontLoadSeg		  	  equ      0x6100 ;字体数据加载内存起始地址

stdos:  
		;call setVGAPalete  ;设置vga调色板 0-15号颜色
		cli
		xor ax,ax
		mov ds,ax
		mov es,ax
		mov di,0600h+vbe_info_structure-osstart
		mov ax,0x4f00
		int 0x10
		cmp ax,0x004f
		jne readKDataErr

		xor ax,ax
		mov ds,ax
		mov es,ax
		mov dl,10	
		mov dh,1
		mov cl,111b		
		mov si,0600h+hexstrbuff-osstart			
		push dx
		mov ax,[0600h+vbe_info_video_modes-osstart]
		mov dx,[0600h+vbe_info_video_modes+2-osstart]
		call itohstr
		pop dx
		call showstr

		xor ax,ax
		mov ds,ax
		mov ax,[0600h+vbe_info_video_modes+2-osstart]
		mov es,ax
		mov si,[0600h+vbe_info_video_modes-osstart]
		mov di,0600h+vbemodeResolution-osstart
		mov bx,0600h+vbemodbuff-osstart
cpvbemodestart:
		mov ax,[es:si]
		mov [ds:bx],ax
		add bx,2
		add si,2
		cmp ax,0xffff
		je cpvbemodeend
		call getvbemodeResolution
		cmp ax,0x004f
		jne readKDataErr
		add di,4
		jmp cpvbemodestart		
 cpvbemodeend:
		mov ax,[0600h+vbe_info_vendor+2-osstart]
		mov es,ax
		mov si,[0600h+vbe_info_vendor-osstart]
cpvbevenstart:
		mov al,[es:si]
		mov [ds:bx],al
		inc bx
		inc si
		cmp al,0
		je cpvbevenend
		jmp cpvbevenstart
cpvbevenend:
		mov byte [ds:bx-1],':'
		mov ax,[0600h+vbe_info_product_name+2-osstart]
		mov es,ax
		mov si,[0600h+vbe_info_product_name-osstart]
cpvbepronamestart:
		mov al,[es:si]
		mov [ds:bx],al
		inc bx
		inc si
		cmp al,0
		je cpvbepronameend
		jmp cpvbepronamestart
cpvbepronameend:
		mov byte [ds:bx-1],':'
		mov ax,[0600h+vbe_info_product_rev+2-osstart]
		mov es,ax
		mov si,[0600h+vbe_info_product_rev-osstart]
cpvbeprorevstart:
		mov al,[es:si]
		mov [ds:bx],al
		inc bx
		inc si
		cmp al,0
		je cpvbeprorevend
		jmp cpvbeprorevstart
cpvbeprorevend:

		xor ax,ax
		mov ds,ax
		mov es,ax
		mov di,0600h+vbe_mode_info_structure-osstart
		mov ax,0x4f01
		mov cx,0x14f;0x155;0x152 ;0x14c
		int 0x10
		cmp ax,0x004f
		jne readKDataErr

		mov ax, 0x4F02	; set VBE mode
		mov bx, 0x414f ;0x4152;0x4155 ;0x414c	; VBE mode number; notice that bits 0-13 contain the mode number and bit 14 (LFB) is set and bit 15 (DM) is clear.
		int 0x10			; call VBE BIOS
		cmp ax, 0x004F	; test for error
		jne readKDataErr


		xor ax,ax
		mov es,ax
		mov ds,ax
		mov ah,0x41
		mov bx,0x55AA
		mov dl,0x80
		int 0x13
		mov ah,4
		jb readKDataErr
		;加载kernel
		mov ax,kernelLoadSeg
		mov es,ax
		mov di,0
		mov si,kernelStartSection
		mov ax,kernelSectionCount
		call read_hard_disk_0
		jb readKDataErr
		;加载apcode
		mov ax,apLoadSeg
		mov es,ax
		mov di,0
		mov si,apcodeStartSection
		mov ax,apcodeSectionCount
		call read_hard_disk_0
		jb readKDataErr
		;加载字体数据
;		mov ax,fontLoadSeg
;		mov es,ax
;		mov di,0
;		mov si,fontStartSection
;		mov ax,fontSectionCount
;		call read_hard_disk_0
;		jb readKDataErr

		xor ax,ax
		mov ds,ax
		mov es,ax	
		mov di,0600h+memInfoData-osstart
		call getMemInfo
			
		call read_DriveParam
		jnb stdos0

readKDataErr:
		add ah,0x30
		add al,0x30
		mov [0600h+readDriveErrCode-osstart],ax
		mov si,0600h+ readDriveErr -  osstart
		mov dh,3
		mov dl,20
		mov cl,111b
		call showstr
hltt:	hlt	
		jmp	hltt
stdos0:	
		mov  eax,[0600h+Sectors-osstart]	
		mov  [0600h+prtlen-osstart],eax				
        lgdt [0600h+gdt_size-osstart]
		
		in al,0x92                         ;南桥芯片内的端口 
        or al,0000_0010B
        out 0x92,al                        ;打开A20
		 
		cli                                ;保护模式下中断机制尚未建立，应 
                                           ;禁止中断 
        mov eax,cr0
        or eax,1
        mov cr0,eax                        ;设置PE位
						;以下进入保护模式... ...
		jmp dword 0x0008:0x600+start32-osstart
	
[bits 32]		
start32:

		mov eax,0000000000010_000B         ;加载数据段选择子(0x10)
        mov es,eax
        mov ds,eax
        mov gs,eax
        mov fs,eax																			
		
	
		 ;以下用简单的示例来帮助阐述32位保护模式下的堆栈操作 
        mov eax,0000000000011_000B         ;加载堆栈段选择子
        mov ss,eax
		mov esp,0x7000

		movzx ecx,word [0600h+gdt_size-osstart]
		inc ecx
		mov esi,0600h+gdt_table-osstart
		mov edi,0xb000												
		rep movsb
		mov dword [0600h+gdt_base-osstart],kernelVirAddr							;0x7000idt
		mov dword [0xb000+24],0x00000030											; 0x8000~0x9000 全局页目录
		mov dword [0xb000+28],0x00CC9600									; 0x9000~0xb000 初始页表
														; 0xb000~0x1B000 gdt
		mov ecx,0xc00										; 0x1b000~0x3b000 pageStatus
		mov ebx,0x8000									; 0x3b000 内核加载起始地址
stdos1:	mov dword [ebx],0		
		add ebx,4
		loop stdos1

			
		mov ebx,0x8000	
		mov dword [ebx+0xffc],0x811B
		mov dword [ebx],0x9103
		mov eax,kernelSectionCount
		mov ecx,512
		mul ecx
		add eax,0x3AFFF
		shr eax,12
		mov ecx,eax
		inc ecx
		mov [0600h+loadend4k-osstart],ecx
		mov edx,0x1B
		mov ebx,0x9000
stdos2:	mov [ebx],edx
		add ebx,4
		add edx,0x1000
		loop stdos2
		
		mov eax,0xb8
		mov ebx,0xB811B
		mov ecx,8
stdos4:
		mov dword [0x9000+eax*4],ebx 
		inc eax
		add ebx,0x1000
		loop stdos4
		
		call initPageStat
		
		mov ebx,0x8000
		mov eax,kernelVirAddr
		shr eax,22
		shl eax,2
		mov dword [ebx+eax],0xA11B
		mov ecx,0x30
		mov edx,0xB11B
		mov ebx,0xa000
stdos3:	mov [ebx],edx
		mov eax,edx
		shr eax,12
		bts dword [0x1b000],eax
		add ebx,4
		add edx,0x1000
		loop stdos3
		
		mov dword [ebx],0x711B
		bts dword [0x1b000],7
		bts dword [0x1b000],8
		bts dword [0x1b000],9
		bts dword [0x1b000],0xa	
		
		 ;令CR3寄存器指向页目录，并正式开启页功能 
        mov eax,0x8018                 ;PCD=PWT=1 物理地址在cr3中,所以PAT index =2*PCD+PWD =3=PAT3=UC
        mov cr3,eax
		mov eax,cr0
        or eax,0x80000000
        mov cr0,eax                        ;开启分页机制
		
		lgdt [0600h+gdt_size-osstart]
		
		mov eax,cr4
		or eax,0x80 ;Enables global pagesPGE designated with G flag
		mov cr4,eax
		mov eax,cr3
		mov cr3,eax
		
		push dword 4
		call allocateVirtual4kPage
		add esp,4		
		push eax
			
		push 0600h+proEntry-osstart
		xor edx,edx
		mov eax,kernelLoadSeg
		shl eax,4
		; mov ecx,0x10
		; mul ecx
		push eax
		call loadElf
		add esp,8
		pop eax
		mov esp,eax
		add esp,0x4000
		mov eax,0000000000011_000B
		mov ss,eax
		;mov ebx,[0600h+phy_base-osstart]
		;mov eax,16                      ;起始地址 
       ; mov cl,[0600h+kernelSectionCount-osstart]
		;call read_hard_disk_1              ;以下读取程序的起始部分（一个扇区）
		;mov ebx,kernelSectionCount
		;mov edi,kernelLoadAddr
		;mov esi,0600h+prtlen-osstart
		;mov ebp,kernelStartSection
		;call read_ata_st
		;jle goKernel
		;hlt
		push dword 0600h+vbe_info_structure-osstart
		push dword 0600h+proEntry-osstart
		
		call [0600h+proEntry-osstart]
		cli 
		hlt

initPageStat:
		push eax 
		push ebx
		push ecx
		push edx
		push esi
		
		mov ecx,0x8000
		mov ebx,0x1b000
clrstat:mov dword [ebx],0xFFFFFFFF
		add ebx,4
		loop clrstat
		
		mov ecx,[0600h+memInfoDataSize-osstart]
		mov ebx,0600h+memInfoData-osstart
		mov esi,0
initPageStat0:	
		cmp dword [ebx+esi+16],1
		jnz initPageStat2
		mov edx,[ebx+esi+4]
		test edx,edx
		jnz initPageStat2
		mov edx,0xffffffff
		mov eax,[ebx+esi+8]
		dec eax
		test edx,[ebx+esi+12]
		cmovz edx,eax
		mov eax,[ebx+esi]
		add edx,eax
		push eax
		mov eax,0xffffffff
		cmovb edx,eax
		pop eax
		shr eax,12
		shr edx,12
initPageStat1:	
		btr dword [0x1b000],eax
		cmp eax,edx
		jae initPageStat2
		inc eax
		jmp initPageStat1	
		
initPageStat2:
		add esi,20
		loop initPageStat0
		
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	
allocatePhy4kPage:
		push edx
		push ebx
		xor edx,edx
		mov eax,[esp+0xc]
next4k:	bts dword [0x1b000],eax
		jnb target4k	
		inc eax
		cmp eax,0x10000
		jae no4k
		jmp next4k
no4k:	xor eax,eax
target4k:
		shl eax,12
		pop ebx
		pop edx
		ret
	
allocateVirtual4kPage:
		push ebp
		mov ebp,esp
		push ecx
		push ebx
		push edx
		mov ecx,[ebp+8]
		mov ebx,[0600h+kernelAllocateNextAddr-osstart]
	
allcate0:
		mov edx,ebx
		shr edx,22
		shl edx,2
		test dword [0xfffff000+edx],1
		jne addPhyaddr
		push dword startPhyPage
		call allocatePhy4kPage
		add esp,4
		or eax,3
		mov [0xfffff000+edx],eax
addPhyaddr:
		shr edx,2
		shl edx,12
		or  edx,0xffc00000
		mov eax,ebx
		and eax,0x3FF000
		shr eax,10
		add edx,eax
		test dword [edx],1
		jne allcate1
		push dword startPhyPage
		call allocatePhy4kPage
		add esp,4
		or eax,3
		mov [edx],eax
allcate1:		
		add ebx,0x1000
		loop allcate0
		mov eax,[0600h+kernelAllocateNextAddr-osstart]
		mov [0600h+kernelAllocateNextAddr-osstart],ebx
		pop edx
		pop ebx
		pop ecx
		pop ebp
		ret

;Elf32_Ehdr 
e_ident       equ      0 ;times 16 db 0
e_type        equ      16;dw 0
e_machine     equ      18;dw 0;		/* Architecture */
e_version     equ      20;dd 0;		/* Object file version */
e_entry       equ      24;dd 0;		/* Entry point virtual address */
e_phoff       equ      28;dd 0;		/* Program header table file offset */
e_shoff       equ      32;dd 0;		/* Section header table file offset */
e_flags       equ      36;dd 0;		/* Processor-specific flags */
e_ehsize      equ      40;dw 0;		/* ELF header size in bytes */
e_phentsize   equ      42;dw 0;		/* Program header table entry size */
e_phnum       equ      44;dw 0;		/* Program header table entry count */
e_shentsize   equ      46;dw 0;		/* Section header table entry size */
e_shnum       equ      48;dw 0;		/* Section header table entry count */
e_shstrndx    equ      50;dw 0;		/* Section header string table index */

;Elf32_Phdr   
p_type        equ      0;dd 0;			/* Segment type */
p_offset      equ      4;dd 0;		/* Segment file offset */
p_vaddr       equ      8;dd 0;		/* Segment virtual address */
p_paddr       equ      12;dd 0;		/* Segment physical address */
p_filesz      equ      16;dd 0;		/* Segment size in file */
p_memsz       equ      20;dd 0;		/* Segment size in memory */
p_flags       equ      24;dd 0;		/* Segment flags */
p_align       equ      28;dd 0;		/* Segment alignment */		


PT_LOAD		equ 1
;0600h+vir_base-osstart	
loadElf:
		push ebp
		mov ebp,esp
		push eax
		push ebx
		push ecx
		push edx
		push esi
		mov ebx,[ebp+8]
		mov eax,[ebp+0xc]
		push dword [ebx+e_entry]
		pop  dword [eax]
		movzx ecx,word [ebx+e_phnum]
		movzx edx,word [ebx+e_phentsize]
		mov esi,[ebx+e_phoff]	
loadElf0:
		push eax
		push esi
		push ebx
		call loadSeg
		add esp,0xc
		add esi,edx
		loop loadElf0
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		pop ebp
		ret	
loadSeg:
		push ebp
		mov ebp,esp
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		
		mov ebx,[ebp+8]
		mov eax,[ebp+0xc]
		cmp dword [ebx+eax+p_type],PT_LOAD
		jnz loadSeg0
		mov ecx,[ebx+eax+p_memsz]
		test ecx,ecx
		je loadSeg0
		mov esi,[ebp+0x10]
		add esi,4
		mov ecx,[esi]
		mov edi,[ebx+eax+p_vaddr]
		cmp edi,ecx	
		cmovb ecx,edi
		mov [esi],ecx
		push eax
		mov ecx,[ebx+eax+p_align]
		mov eax,[ebx+eax+p_memsz]
		xor edx,edx
		div ecx
		pop eax	
		sub ecx,edx
		dec ecx
		add edi,ecx
		add edi,[ebx+eax+p_memsz]
		mov ecx,[esi+4]
		cmp edi,ecx
		cmova ecx,edi
		mov [esi+4],ecx
		and ecx,0xfffff000
		mov edx,[ebx+eax+p_vaddr]
		and edx,0xfffff000
		push eax
loadSeg1:		
		mov [0600h+kernelAllocateNextAddr-osstart],edx
		push 1
		call allocateVirtual4kPage
		add esp,4
		cmp edx,ecx
		jae copyfileData
		add edx,0x1000
		jmp loadSeg1
copyfileData:	
		pop eax
		mov ecx,[ebx+eax+p_filesz]
		test ecx,ecx
		je loadSeg0
		mov edi,[ebx+eax+p_vaddr]
		mov esi,[ebx+eax+p_offset]
		add esi,ebx
		mov ecx,[ebx+eax+p_filesz]
		cld
		rep movsb
loadSeg0:
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		pop ebp
		ret


		
; do a singletasking PIO ATA read
; inputs: ebx = # of sectors to read, edi -> dest buffer, esi -> driverdata struct, ebp = 4b LBA
; Note: ebp is a "relative" LBA -- the offset from the beginning of the partition
; outputs: ebp, edi incremented past read; ebx = 0
; flags: zero flag set on success, carry set on failure (redundant)	

dd_prtlen equ 0  
dd_dcr equ 4
dd_stLBA equ 6
dd_tf equ 10
dd_sbits  equ 12

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

	
packet     		 db	10h             ;packet大小，16个字节
reserved	 	 db 0
readsectioncount	 dw	0		;读扇区
bufferoff        dw	7e00h          ;读到内存7e00处，偏移地址
bufferseg	 	 dw	0		;段地址
blockNum	 	 dd	0               ;起始LBA块
				 dd 0					 
dataa	db '1) reset pc',0
datab	db '2) start system',0
datac	db '3) clock',0
datad	db '4) set clock',0
tmstr	db 'yy/MM/dd hh:mm:ss',0
writetmmsg	db "'<-' Move cursor to the left",0
writetmmsg1	db "'->' Move cursor to the right",0
writetmmsg2	db "'1-9' Modify the number at the cursor",0
writetmmsg3	db "'enter' Write data to cmos",0
timecolor db 111b
tmindex	db 9,8,7,4,2,0
int9a	dw 0,0
table	dw 0600h+ rst -  osstart,0,0600h+ stdos -  osstart,0,0600h+ showtms -  osstart,0,0600h+ edittm -  osstart,0
proEntry dd 0
vir_base dd 0xffffffff   
vir_end  dd 0
phy_cs	 dd 0x8
phy_ds	 dd 0000000000010_000B
EBDAseg dw 0
gdt_size dw 39
gdt_base dd 0600h+gdt_table-osstart     ;GDT的物理地址 
pageStatus    dd kernelVirAddr+0x10000
idtTable      dd kernelVirAddr+0x30000
kernelAllocateNextAddr dd kernelVirAddr+0x31000
memInfoDataSize dw 0
memInfoData times 1200 db 0;每20个字节代表一段内存信息,20个字节如下
							;Offset in Bytes		Name		Description
							;		0	    BaseAddrLow		Low 32 Bits of Base Address
							;		4	    BaseAddrHigh	High 32 Bits of Base Address
							;		8	    LengthLow		Low 32 Bits of Length in Bytes
							;		12	    LengthHigh		High 32 Bits of Length in Bytes
							;		16	    Type		Address type of  this range.
;DriveParametersPacket	
InfoSize dw 26 ; // 数据包尺寸 (26 字节)
Flags dw 0 ; // 信息标志
Cylinders dd 0 ; // 磁盘柱面数
Heads dd 0 ; // 磁盘磁头数
SectorsPerTrack  dd 0 ; // 每磁道扇区数
Sectors dd 0
		dd 0; // 磁盘总扇区数
SectorSize dw 0 ; // 扇区尺寸 (以字节为单位) 


prtlen dd 0
dcr	 dw 0x1f7
stLBA dd 0
tf  	 dw 0x1F0
sbits db 0xe0
gdt_table dd 0x00,0x00
		  dd 0x0000ffff,0x00CF9A00 ;创建1#描述符，这是一个代码段，对应0~4GB的线性地址空间
		  dd 0x0000ffff,0x00cf9200 ;创建2#描述符，这是一个数据段，对应0~4GB的线性地址空间
		  dd 0x00000C00,0x00409600 ;建立保护模式下的堆栈段描述符
		  dd 0x80007fff,0x0040920b ;建立保护模式下的显示缓冲区描述符
loadend4k dd 0		  
hexstrbuff times 16 db 0
hexstrnum  db '0123456789ABCDEF'
meminfoerrstr db 'getMemInfo err',0
meminfostr db 'AddressRangeMemory',0
meminfostr1 db 'AddressRangeReserved',0
readDriveErr db 'int13 Extensions not support:'
readDriveErrCode db 0,0,0,0

readInt13Error db 'int13 Extensions not support',0

table_rgb db 0x00, 0x00, 0x00  ;/* 0:黑 */
db 0xff, 0x00, 0x00 ;/* 1:亮红 */
db 0x00, 0xff, 0x00 ;/* 2:亮绿 */
db 0xff, 0xff, 0x00 ;/* 3:亮黄 */
db 0x00, 0x00, 0xff ;/* 4:亮蓝 */
db 0xff, 0x00, 0xff ;/* 5:亮紫 */
db 0x00, 0xff, 0xff ;/* 6:浅亮蓝 */
db 0xff, 0xff, 0xff ;/* 7:白 */
db 0xc6, 0xc6, 0xc6 ;/* 8:亮灰 */
db 0x84, 0x00, 0x00 ;/* 9:暗红 */
db 0x00, 0x84, 0x00 ;/* 10:暗绿 */
db 0x84, 0x84, 0x00 ;/* 11:暗黄 */
db 0x00, 0x00, 0x84 ;/* 12:暗青 */
db 0x84, 0x00, 0x84 ;/* 13:暗紫 */
db 0x00, 0x84, 0x84 ;/* 14:浅暗蓝 */
db 0x84, 0x84, 0x84;/* 15:暗灰 */



vbe_info_structure  db "VBE2"
vbe_info_version dw 0;      // VBE version; high byte is major version, low byte is minor version
vbe_info_oem dd 0;          // segment:offset pointer to OEM
vbe_info_capabilities dd 0; // bitfield that describes card capabilities
vbe_info_video_modes dd 0;  // segment:offset pointer to list of supported video modes
vbe_info_video_memory dw 0; // amount of video memory in 64KB blocks
vbe_info_software_rev dw 0; // software revision
vbe_info_vendor dd 0;       // segment:offset to card vendor string
vbe_info_product_name dd 0; // segment:offset to card model name
vbe_info_product_rev dd 0;  // segment:offset pointer to product revision
times 222 db 0;    // reserved for future expansion
vbe_info_oem_data times 256 db 0;    // OEM BIOSes store their strings in this area

vbe_mode_info_structure times 256 db 0
vbemodbuff  dw 0xffff
times 510 db 0
vbemodeResolution times 768 db 0
osstartend:times 512 db 0 