global puts,readSectors,clearScreen,exit,setcursor

puts_s dd 0,0x33  
clearScreen_s dd 0,0x3B
setcursor_s dd 0,0x43
readSectors_s dd 0,0x4b
exit_s  dd 0,0x53

puts:
	push dword [esp+4]
	call far [puts_s]
	ret


readSectors:
	push ebp
	mov ebp,esp
	push dword [ebp+0x10]
	push dword [ebp+0xc]
	push dword [ebp+0x8]
	call far [readSectors_s]
	ret
clearScreen:
	call far [clearScreen_s]
	ret
exit:
	push dword [esp+4]
	call far [exit_s]
	ret
setcursor:
	push dword [esp+4]
	call far [setcursor_s]
	ret