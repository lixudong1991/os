global ps2Controllerinit,wait60PortToread,wait60_64PortTowirte

ps2Controllerinit:
	push ecx
	cli
	xor eax,eax
	xor ecx,ecx
	call wait60_64PortTowirte
	mov al,0xad
	out 0x64,al
	call wait60_64PortTowirte
	mov al,0xa7
	out 0x64,al

	in al,0x60

	call wait60_64PortTowirte
	mov al,0x20
	out 0x64,al

	call wait60PortToread
	in al,0x60
	and al,0xbc
	mov cl,al
	
	call wait60_64PortTowirte
	mov al,0x60
	out 0x64,al

	call wait60_64PortTowirte
	mov al,cl
	out 0x60,al

	call wait60_64PortTowirte
	mov al,0xaa
	out 0x64,al

	call wait60PortToread
	in al,0x60
	cmp al,0x55
	jne ps2keyboardinitret

	test cl,0x20
	je ps2keyboardinit0
	call wait60_64PortTowirte
	mov al,0xa9
	out 0x64,al
	call wait60PortToread
	in al,0x60
	test al,al
	jne ps2keyboardinitret
	call wait60_64PortTowirte
	mov al,0xa8
	out 0x64,al
	call wait60_64PortTowirte
	mov al,0x20
	out 0x64,al
	call wait60PortToread
	in al,0x60
	or al,2
	mov cl,al
	call wait60_64PortTowirte
	mov al,0x60
	out 0x64,al
	call wait60_64PortTowirte
	mov al,cl
	out 0x60,al

ps2keyboardinit0:
	call wait60_64PortTowirte
	mov al,0xab
	out 0x64,al
	call wait60PortToread
	in al,0x60
	test al,al
	jne ps2keyboardinitret
	call wait60_64PortTowirte
	mov al,0xae
	out 0x64,al
	call wait60_64PortTowirte
	mov al,0x20
	out 0x64,al
	call wait60PortToread
	in al,0x60
	or al,1
	mov cl,al
	call wait60_64PortTowirte
	mov al,0x60
	out 0x64,al
	call wait60_64PortTowirte
	mov al,cl
	out 0x60,al

	call wait60_64PortTowirte
	mov al,0xff
	out 0x60,al
	call wait60PortToread
	in al,0x60
	cmp al,0xfa
	jne ps2keyboardinitret
	call wait60PortToread
	in al,0x60
	cmp al,0xaa
	jne ps2keyboardinitret

resetscancode:
	call wait60_64PortTowirte
	mov al,0xF0
	out 0x60,al
	call wait60PortToread
	in al,0x60
	cmp al,0xFE
	je resetscancode
	call wait60_64PortTowirte
	mov al,0x2 ;SET  scan code set2
	out 0x60,al
	call wait60PortToread
	in al,0x60
	cmp al,0xFE
	je resetscancode
ps2keyboardinitret:
	pop ecx
	ret




wait60PortToread:
	push eax
read64status:
	in al,0x64
	test al,1
	je read64status
	pop eax
	ret
wait60_64PortTowirte:
	push eax
read64status0:
	in al,0x64
	test al,2
	jne read64status0
	pop eax
	ret