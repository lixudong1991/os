
global exceptionCalls,general_interrupt_handler,systemCall,interrupt_8259a_handler,interrupt_70_handler

extern   kernelData,hexstr32

puts_int:                                 ;��ʾ0��ֹ���ַ������ƶ���� 
         push ebx                                   ;���룺DS:EBX=����ַ
		 mov ebx,[esp+8]
         push ecx
  .getc:
         mov cl,[ebx]
         or cl,cl
         jz .exit
         call .put_char
         inc ebx
         jmp .getc
  .exit:
         pop ecx
		 pop ebx
         
         ret                               ;�μ䷵��

.put_char:                                   ;�ڵ�ǰ��괦��ʾһ���ַ�,���ƽ�
                                            ;��ꡣ�����ڶ��ڵ��� 
                                            ;���룺CL=�ַ�ASCII�� 
         pushad

         ;����ȡ��ǰ���λ��
         mov dx,0x3d4
         mov al,0x0e
         out dx,al
         inc dx                             ;0x3d5
         in al,dx                           ;����
         mov ah,al

         dec dx                             ;0x3d4
         mov al,0x0f
         out dx,al
         inc dx                             ;0x3d5
         in al,dx                           ;����
         mov bx,ax                          ;BX=������λ�õ�16λ��
         and ebx,0x0000ffff                 ;׼��ʹ��32λѰַ��ʽ�����Դ� 
         
         cmp cl,0x0d                        ;�س�����
         jnz .put_0a                         
         
         mov ax,bx                          ;���°��س������� 
         mov bl,80
         div bl
         mul bl
         mov bx,ax
         jmp .set_cursor

  .put_0a:
         cmp cl,0x0a                        ;���з���
         jnz .put_other
         add bx,80                          ;����һ�� 
         jmp .roll_screen

  .put_other:                               ;������ʾ�ַ�
         shl bx,1
         mov [0xb8000+ebx],cl            ;�ڹ��λ�ô���ʾ�ַ� 

         ;���½����λ���ƽ�һ���ַ�
         shr bx,1
         inc bx

  .roll_screen:
         cmp bx,2000                        ;��곬����Ļ������
         jl .set_cursor

         cld
         mov esi,0xb80a0                 ;С�ģ�32λģʽ��movsb/w/d 
         mov edi,0xb8000                 ;ʹ�õ���esi/edi/ecx 
         mov ecx,960
         rep movsd
         mov bx,3840                        ;�����Ļ���һ��
         mov ecx,80                         ;32λ����Ӧ��ʹ��ECX
  .cls:
         mov word [0xb8000+ebx],0x0720
         add bx,2
         loop .cls

         mov bx,1920

  .set_cursor:
         mov dx,0x3d4
         mov al,0x0e
         out dx,al
         inc dx                             ;0x3d5
         mov al,bh
         out dx,al
         dec dx                             ;0x3d4
         mov al,0x0f
         out dx,al
         inc dx                             ;0x3d5
         mov al,bl
         out dx,al
         
         popad
         
         ret 

general_exception0_handler:
	push excep_msg
	call puts_int
	push intnum+0
	call puts_int
	hlt
general_exception1_handler:
	push excep_msg
	call puts_int
	push intnum+3
	call puts_int
	hlt
general_exception2_handler:
	push excep_msg
	call puts_int
	push intnum+6
	call puts_int
	hlt
general_exception3_handler:
	push excep_msg
	call puts_int
	push intnum+9
	call puts_int
	hlt
general_exception4_handler:
	push excep_msg
	call puts_int
	push intnum+12
	call puts_int
	hlt
general_exception5_handler:
	push excep_msg
	call puts_int
	push intnum+15
	call puts_int
	hlt
general_exception6_handler:
	push excep_msg
	call puts_int
	push intnum+18
	call puts_int
	hlt
general_exception7_handler:
	push excep_msg
	call puts_int
	push intnum+21
	call puts_int
	hlt
general_exception8_handler:
	push excep_msg
	call puts_int
	push intnum+24
	call puts_int
	hlt
general_exception9_handler:
	push excep_msg
	call puts_int
	push intnum+27
	call puts_int
	hlt
general_exception10_handler:
	push excep_msg
	call puts_int
	push intnum+30
	call puts_int
	hlt
general_exception11_handler:
	push excep_msg
	call puts_int
	push intnum+33
	call puts_int
	hlt
general_exception12_handler:
	push excep_msg
	call puts_int
	push intnum+36
	call puts_int
	hlt
general_exception13_handler:
    mov eax,[esp]
	push eax
	push dword excep_codebuff
	call hexstr32
	add esp,8
	mov eax,[esp+4]
	push eax
	push dword excep_precodebuff
	call hexstr32
	add esp,8
	push dword excep_msg
	call puts_int
	add esp,4
	push dword intnum+39
	call puts_int
	add esp,4
	push dword excep_precodestr
	call puts_int
	add esp,4
	push dword excep_codebuffstr
	call puts_int
	add esp,8
	hlt
general_exception14_handler:
    mov eax,[esp]
	push eax
	push excep_codebuff
	call hexstr32
	add esp,8
	mov eax,[esp+4]
	push eax
	push excep_precodebuff
	call hexstr32
	add esp,8
	push excep_msg
	call puts_int
	push intnum+42
	call puts_int
	push excep_precodestr
	call puts_int
	push excep_codebuffstr
	call puts_int
	hlt
general_exception15_handler:
	push excep_msg
	call puts_int
	push intnum+45
	call puts_int
	hlt
general_exception16_handler:
	push excep_msg
	call puts_int
	push intnum+48
	call puts_int
	hlt
general_exception17_handler:
	push excep_msg
	call puts_int
	push intnum+51
	call puts_int
	hlt
general_exception18_handler:
	push excep_msg
	call puts_int
	push intnum+54
	call puts_int
	hlt
general_exception19_handler:
	push excep_msg
	call puts_int
	push intnum+57
	call puts_int
	hlt
general_interrupt_handler: 
    iretd

interrupt_8259a_handler:
    push eax     
    mov al,0x20                        ;�жϽ�������EOI 
    out 0xa0,al                        ;���Ƭ���� 
    out 0x20,al                        ;����Ƭ����       
    pop eax        
    iretd

interrupt_70_handler:
    push eax   
	push ebx 
	push edx
	push esi
         mov al,0x20                        ;�жϽ�������EOI
         out 0xa0,al                        ;��8259A��Ƭ����
         out 0x20,al                        ;��8259A��Ƭ����

         mov al,0x0c                        ;�Ĵ���C���������ҿ���NMI
         out 0x70,al
         in al,0x71                         ;��һ��RTC�ļĴ���C������ֻ����һ���ж�
                                          ;�˴����������Ӻ��������жϵ����  										  
    mov ebx,kernelData
	mov eax,20
	mov edx,32
	test dword [ebx+eax+8],0xffffffff
	je ret70
	cmp dword [ebx+eax+8],1
	je ret70
	mov esi,[ebx+edx]
nexttask:
	mov esi,[esi]
	cmp esi,0
	jne nexttask1
	mov esi,[ebx+eax]
nexttask1:
	cmp esi,[ebx+edx]
	je ret70
	test dword [esi+0x10],0xffffffff
	jnz nexttask
	mov edx,[ebx+edx]
	mov dword [edx+0x10],0
	mov edx,32
	mov [ebx+edx],esi
	mov dword [esi+0x10],1
	jmp far [esi+8]
ret70:
	pop esi
	pop edx
	pop ebx
    pop eax        
    iretd
systemCall:
	push systemCallmsg
	call puts_int
	add esp,4
	iretd
	
systemCallmsg    db  'systemCall encounted 0x80  ',0
excep_msg        db  '********Exception encounted********',0
intnum			 db  '00',0,'01',0,'02',0,'03',0,'04',0,'05',0,'06',0,'07',0,'08',0,'09',0,'10',0,'11',0
				 db  '12',0,'13',0,'14',0,'15',0,'16',0,'17',0,'18',0,'19',0,'20',0
excep_precodestr db 'error code addr:'
excep_precodebuff    db  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
excep_codebuffstr db '  code: '
excep_codebuff    db  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
				 
exceptionCalls   dd  general_exception0_handler,general_exception1_handler,general_exception2_handler,general_exception3_handler,general_exception4_handler
				 dd  general_exception5_handler,general_exception6_handler,general_exception7_handler,general_exception8_handler,general_exception9_handler
				 dd  general_exception10_handler,general_exception11_handler,general_exception12_handler,general_exception13_handler,general_exception14_handler
				 dd  general_exception15_handler,general_exception16_handler,general_exception17_handler,general_exception18_handler,general_exception19_handler