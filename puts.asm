video_ram_seg_sel     equ  0x20    ;视频显示缓冲区的段选择子
global die,clearscreen,setcursor,getcursor
;puts:                                 ;显示0终止的字符串并移动光标 
;         push ebx                                   ;输入：DS:EBX=串地址
;		 mov ebx,[esp+8]
;         push ecx
;  .getc:
;         mov cl,[ebx]
;         or cl,cl
;         jz .exit
;         call put_char
;         inc ebx
;         jmp .getc
;  .exit:
;         pop ecx
;		 pop ebx
;         
;         ret                               ;段间返回
;
;put_char:                                   ;在当前光标处显示一个字符,并推进
;                                            ;光标。仅用于段内调用 
;                                            ;输入：CL=字符ASCII码 
;         pushad
;
;         ;以下取当前光标位置
;         mov dx,0x3d4
;         mov al,0x0e
;         out dx,al
;         inc dx                             ;0x3d5
;         in al,dx                           ;高字
;         mov ah,al
;
;         dec dx                             ;0x3d4
;         mov al,0x0f
;         out dx,al
;         inc dx                             ;0x3d5
;         in al,dx                           ;低字
;         mov bx,ax                          ;BX=代表光标位置的16位数
;         and ebx,0x0000ffff                 ;准备使用32位寻址方式访问显存 
;         
;         cmp cl,0x0d                        ;回车符？
;         jnz .put_0a                         
;         
;         mov ax,bx                          ;以下按回车符处理 
;         mov bl,80
;         div bl
;         mul bl
;         mov bx,ax
;         jmp .set_cursor
;
;  .put_0a:
;         cmp cl,0x0a                        ;换行符？
;         jnz .put_other
;         add bx,80                          ;增加一行 
;         jmp .roll_screen
;
;  .put_other:                               ;正常显示字符
;         shl bx,1
;         mov [0xb8000+ebx],cl            ;在光标位置处显示字符 
;
;         ;以下将光标位置推进一个字符
;         shr bx,1
;         inc bx
;
;  .roll_screen:
;         cmp bx,2000                        ;光标超出屏幕？滚屏
;         jl .set_cursor
;
;         cld
;         mov esi,0xb80a0                 ;小心！32位模式下movsb/w/d 
;         mov edi,0xb8000                 ;使用的是esi/edi/ecx 
;         mov ecx,960
;         rep movsd
;         mov bx,3840                        ;清除屏幕最底一行
;         mov ecx,80                         ;32位程序应该使用ECX
;  .cls:
;         mov word [0xb8000+ebx],0x0720
;         add bx,2
;         loop .cls
;
;         mov bx,1920
;
;  .set_cursor:
;         mov dx,0x3d4
;         mov al,0x0e
;         out dx,al
;         inc dx                             ;0x3d5
;         mov al,bh
;         out dx,al
;         dec dx                             ;0x3d4
;         mov al,0x0f
;         out dx,al
;         inc dx                             ;0x3d5
;         mov al,bl
;         out dx,al
;         
;         popad
;         
;         ret    
die:
	hlt
	
setcursor:
		push ebp
		mov ebp,esp
		push eax
		push edx
		mov dx,0x3d4
              mov al,0x0e
              out dx,al
		mov dx,0x3d5
		mov al,[ebp+9]
		out dx,al
		
		mov dx,0x3d4
		mov al,0x0f
              out dx,al
		mov dx,0x3d5
		mov al,[ebp+8]
		out dx,al
		
		pop edx
		pop eax
		pop ebp
		ret

getcursor:
              push edx
              xor eax,eax
              mov dx,0x3d4
              mov al,0x0e
              out dx,al
              mov dx,0x3d5
              in  al,dx
              mov ah,al
              mov dx,0x3d4
              mov al,0x0f
              out dx,al
              mov dx,0x3d5
              in  al,dx
              pop edx
              ret

clearscreen:
		push es
		push ecx
		push esi
		mov  ecx,video_ram_seg_sel
		mov es,ecx
		mov ecx,1000
		mov esi,0
clrs:	mov dword [es:esi],0x07200720;清空屏幕内容，并设置黑底白字
		add esi,4
		loop clrs
		push word 0
		call setcursor
		add esp,2
		pop esi
		pop ecx
		pop es
		ret	
	