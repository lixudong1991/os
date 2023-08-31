global memcpy_s,memcmp_s,memset_s,strcpy_s,strlen_s
memcpy_s:
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ecx
	mov edi,[ebp+8]
	mov esi,[ebp+0xc]
	mov ecx,[ebp+0x10]
	cld 
	rep movsb
	pop ecx
	pop edi
	pop esi
	pop ebp
	ret

memcmp_s:
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ecx
	mov esi,[ebp+8]
	mov edi,[ebp+0xc]
	mov ecx,[ebp+0x10]
	cld 
	repe cmpsb
	mov cl,[esi]
	sub cl,[edi]
	movsx eax,cl
	pop ecx
	pop edi
	pop esi
	pop ebp
	ret
memset_s:
	push ebp
	mov ebp,esp
	push eax
	push ecx
	push ebx
	mov ebx,[ebp+8]
	mov eax,[ebp+0xc]
	mov ecx,[ebp+0x10]
set:mov [ebx],al
	inc ebx
	loop set
	mov eax,[ebp+8]
	pop ebx
	pop ecx
	pop eax
	pop ebp
	ret
strcpy_s:
	push ebp
	mov ebp,esp
	push ecx
	push esi
	push edi
	mov esi,[ebp+0xc]
	mov edi,[ebp+8]
	xor ecx,ecx
strcpy_s0:	
	mov cl,[esi]
	mov [edi],cl
	jcxz strcpy_s1
	inc esi
	inc edi
	jmp strcpy_s0
strcpy_s1:
	mov eax,[ebp+8]
	pop edi
	pop esi 
	pop ecx
	pop ebp
	ret

strlen_s:
    push ebx
	mov ebx,[esp+8]
	push ecx
	xor eax,eax
	xor ecx,ecx
strlen_s0:
	mov cl,[ebx+eax]
	jcxz strlen_s1
	inc eax
	jmp strlen_s0
strlen_s1:	
	pop ecx
	pop ebx
	ret