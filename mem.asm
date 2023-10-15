global memcpy_s,memcmp_s,memset_s,strcpy_s,strlen_s,memWordset_s,memDWordset_s,memWordcpy_s,memDWordcpy_s
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
memWordcpy_s:
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ecx
	mov edi,[ebp+8]
	mov esi,[ebp+0xc]
	mov ecx,[ebp+0x10]
	cld 
	rep movsw
	pop ecx
	pop edi
	pop esi
	pop ebp
	ret
memDWordcpy_s:
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ecx
	mov edi,[ebp+8]
	mov esi,[ebp+0xc]
	mov ecx,[ebp+0x10]
	cld 
	rep movsd
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
	push ecx
	push ebx
	mov ebx,[ebp+8]
	mov eax,[ebp+0xc]
	mov ecx,[ebp+0x10]
set:mov byte [ebx],al
	inc ebx
	loop set
	mov eax,[ebp+8]
	pop ebx
	pop ecx
	pop ebp
	ret
memWordset_s:
	push ebp
	mov ebp,esp
	push ecx
	push ebx
	mov ebx,[ebp+8]
	mov eax,[ebp+0xc]
	mov ecx,[ebp+0x10]
memWordset0:
	mov word [ebx],ax
	add ebx,2
	loop memWordset0
	mov eax,[ebp+8]
	pop ebx
	pop ecx
	pop ebp
	ret
memDWordset_s:
	push ebp
	mov ebp,esp
	push ecx
	push ebx
	mov ebx,[ebp+8]
	mov eax,[ebp+0xc]
	mov ecx,[ebp+0x10]
memDWordset0:
	mov dword [ebx],eax
	add ebx,4
	loop memDWordset0
	mov eax,[ebp+8]
	pop ebx
	pop ecx
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