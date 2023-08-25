global puts_s,readSectors_s,clearScreen_s,exit_s,setcursor_s
extern puts,read_sectors,clearscreen,TerminateProgram,setcursor,kernelData
kernelDataSel      equ  0000000000010_000B
puts_s:
	push ebp
	mov ebp,esp
	push eax
	push ecx
	mov ax,ds
	mov cx,11b
	cli
	arpl ax,cx
	mov ds,ax
	pop eax
	pop ecx
	push dword [ebp+0xc]
	call puts
	sti
	add esp,4
	pop ebp
	retf 4
readSectors_s:
	push ebp
	mov ebp,esp
	push eax
	push ecx
	mov ax,ds
	mov cx,11b
	cli
	arpl ax,cx
	mov ds,ax
	pop eax
	pop ecx
	push dword [ebp+0x14]
	push dword [ebp+0x10]
	push dword [ebp+0xc]
	call read_sectors
	sti
	add esp,12
	pop ebp
	retf 12


clearScreen_s:
	call clearscreen
	retf

exit_s:
	mov eax,kernelDataSel
	mov ds,eax
	mov ebx,[esp+0x8]
	push ebx
	call TerminateProgram
	add esp,4
	iret
	

setcursor_s:
	push ebp
	mov ebp,esp
	push dword [ebp+0xc]
	cli
	call setcursor
	sti
	add esp,4
	pop ebp
	retf 4