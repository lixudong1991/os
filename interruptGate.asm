
global exceptionCalls,general_interrupt_handler,systemCall,interrupt_8259a_handler,interrupt_70_handler,local_x2apic_error_handling,x2ApicTimeOut
global local_xapic_error_handling,xApicTimeOut,getXapicAddr
extern   kernelData,hexstr32
IA32_X2APIC_EOI equ 0x80B
IA32_X2APIC_ESR equ 0x828
IA32_X2APIC_INIT_COUNT equ 0x838
IA32_APIC_BASE_MSR equ 0x1B

XAPIC_ID_OFFSET  equ 0x20
XAPIC_LDR_OFFSET  equ 0xD0
XAPIC_InitialCount_OFFSET  equ 0x380
XAPIC_ErrStatus_OFFSET  equ 0x280
XAPIC_EOI_OFFSET  equ 0xB0
puts_int:                                 ;显示0终止的字符串并移动光标 
         push ebx                                   ;输入：DS:EBX=串地址
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
         
         ret                               ;段间返回

.put_char:                                   ;在当前光标处显示一个字符,并推进
                                            ;光标。仅用于段内调用 
                                            ;输入：CL=字符ASCII码 
         pushad

         ;以下取当前光标位置
         mov dx,0x3d4
         mov al,0x0e
         out dx,al
         inc dx                             ;0x3d5
         in al,dx                           ;高字
         mov ah,al

         dec dx                             ;0x3d4
         mov al,0x0f
         out dx,al
         inc dx                             ;0x3d5
         in al,dx                           ;低字
         mov bx,ax                          ;BX=代表光标位置的16位数
         and ebx,0x0000ffff                 ;准备使用32位寻址方式访问显存 
         
         cmp cl,0x0d                        ;回车符？
         jnz .put_0a                         
         
         mov ax,bx                          ;以下按回车符处理 
         mov bl,80
         div bl
         mul bl
         mov bx,ax
         jmp .set_cursor

  .put_0a:
         cmp cl,0x0a                        ;换行符？
         jnz .put_other
         add bx,80                          ;增加一行 
         jmp .roll_screen

  .put_other:                               ;正常显示字符
         shl bx,1
         mov [0xb8000+ebx],cl            ;在光标位置处显示字符 

         ;以下将光标位置推进一个字符
         shr bx,1
         inc bx

  .roll_screen:
         cmp bx,2000                        ;光标超出屏幕？滚屏
         jl .set_cursor

         cld
         mov esi,0xb80a0                 ;小心！32位模式下movsb/w/d 
         mov edi,0xb8000                 ;使用的是esi/edi/ecx 
         mov ecx,960
         rep movsd
         mov bx,3840                        ;清除屏幕最底一行
         mov ecx,80                         ;32位程序应该使用ECX
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
    ; push eax     
    ; mov al,0x20                        ;中断结束命令EOI 
    ; out 0xa0,al                        ;向从片发送 
    ; out 0x20,al                        ;向主片发送       
    ; pop eax        
    iretd

interrupt_70_handler:
    push eax   
	push ebx 
	push edx
	push esi
        ;  mov al,0x20                        ;中断结束命令EOI
        ;  out 0xa0,al                        ;向8259A从片发送
        ;  out 0x20,al                        ;向8259A主片发送

        ;  mov al,0x0c                        ;寄存器C的索引。且开放NMI
        ;  out 0x70,al
        ;  in al,0x71                         ;读一下RTC的寄存器C，否则只发生一次中断
        ;                                   ;此处不考虑闹钟和周期性中断的情况  										  
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
	;call writeEOI
	jmp far [esi+8]
ret70:
	pop esi
	pop edx
	pop ebx
    pop eax        
    iretd

x2ApicTimeOut:
    push eax   
	push ecx 
	push edx
	push esi									  
    mov ecx,kernelData
	test dword [ecx+28],0xffffffff
	je ApicTimeOutret
	cmp dword [ecx+28],1
	je ApicTimeOutret
	mov esi,[ecx+32]
ApicTimeOutnexttask:
	mov esi,[esi]
	cmp esi,0
	jne ApicTimeOutnexttask1
	mov esi,[ecx+20]
ApicTimeOutnexttask1:
	cmp esi,[ecx+32]
	je ApicTimeOutret
	test dword [esi+0x10],0xffffffff
	jnz ApicTimeOutnexttask
	mov edx,[ecx+32]
	mov dword [edx+0x10],0
	mov [ecx+32],esi
	mov dword [esi+0x10],1
	cmp esi,[ecx+20]
	je noSetTimer
	mov eax,0xffff  ;task cpu time
	xor edx,edx
	mov ecx,IA32_X2APIC_INIT_COUNT
	mfence
	wrmsr
noSetTimer:
	call writeEOI
	jmp far [esi+8]
	jmp ApicTimeOutret0
ApicTimeOutret:
	call writeEOI
ApicTimeOutret0:	
	pop esi
	pop edx
	pop ecx
    pop eax  
	iretd
;如果使用x2apic 则放开下面
; systemCall:
; 	push systemCallmsg
; 	call puts_int
; 	add esp,4
; 	push ecx
; 	push edx
; 	mov ecx,0x802
; 	rdmsr
; 	push eax
; 	push dword excep_codebuff
; 	call hexstr32
; 	add esp,8
; 	push excep_codebuffstr
; 	call puts_int
; 	add esp,4

; 	mov ecx,0x80D
; 	rdmsr
; 	push eax
; 	push dword excep_codebuff
; 	call hexstr32
; 	add esp,8
; 	push excep_codebuffstr
; 	call puts_int
; 	add esp,4

; 	pop ecx
; 	pop edx
; 	call writeEOI
; 	iretd
systemCall:
	push systemCallmsg
	call puts_int
	add esp,4
	push edx
	call getXapicAddr
	mov edx,eax
	mov eax,[edx+XAPIC_ID_OFFSET]
	push eax
	push dword excep_codebuff
	call hexstr32
	add esp,8
	push excep_codebuffstr
	call puts_int
	add esp,4

	mov eax,[edx+XAPIC_LDR_OFFSET]
	push eax
	push dword excep_codebuff
	call hexstr32
	add esp,8
	push excep_codebuffstr
	call puts_int
	add esp,4
	pop edx
	call xapicwriteEOI
	iretd

local_x2apic_error_handling:
	push ecx
	push edx
	xor eax,eax
	xor edx,edx
	mov ecx,IA32_X2APIC_ESR
	mfence
	wrmsr
	xor eax,eax
	xor edx,edx
	mov ecx,IA32_X2APIC_ESR
    mfence
	rdmsr
	push eax
	push dword local_x2apic_error_codebuff
	call hexstr32
	add esp,8
	push local_x2apic_error_msg
	call puts_int
	add esp,4
	pop edx
	pop ecx
	call writeEOI
	iretd

writeEOI:
	push edx
	push ecx
	xor edx,edx
	xor eax,eax
	mov ecx,IA32_X2APIC_EOI
	mfence
	wrmsr
    pop ecx
	pop edx
	ret
local_xapic_error_handling:
	push ecx
	push edx
	xor eax,eax
	xor edx,edx
	mov ecx,IA32_APIC_BASE_MSR
    mfence
	rdmsr
	and eax,0xFFFFF000
	mov edx,eax
	mov dword [edx+XAPIC_ErrStatus_OFFSET],0
	mov eax,[edx+XAPIC_ErrStatus_OFFSET]
	push eax
	push dword local_x2apic_error_codebuff
	call hexstr32
	add esp,8
	push local_x2apic_error_msg
	call puts_int
	add esp,4
	call xapicwriteEOI
	pop edx
	pop ecx
	iretd
xApicTimeOut:
    push eax   
	push ecx 
	push edx
	push esi									  
    mov ecx,kernelData
	test dword [ecx+28],0xffffffff
	je xApicTimeOutret
	cmp dword [ecx+28],1
	je xApicTimeOutret
	mov esi,[ecx+32]
xApicTimeOutnexttask:
	mov esi,[esi]
	cmp esi,0
	jne xApicTimeOutnexttask1
	mov esi,[ecx+20]
xApicTimeOutnexttask1:
	cmp esi,[ecx+32]
	je xApicTimeOutret
	test dword [esi+0x10],0xffffffff
	jnz xApicTimeOutnexttask
	mov edx,[ecx+32]
	mov dword [edx+0x10],0
	mov [ecx+32],esi
	mov dword [esi+0x10],1
	cmp esi,[ecx+20]
	je xnoSetTimer
	call getXapicAddr
	mov dword [eax+XAPIC_InitialCount_OFFSET],0xffff ;task cpu time
xnoSetTimer:
	call xapicwriteEOI
	jmp far [esi+8]
	jmp xApicTimeOutret0
xApicTimeOutret:
	call xapicwriteEOI
xApicTimeOutret0:	
	pop esi
	pop edx
	pop ecx
    pop eax  
	iretd

xapicwriteEOI:
	call getXapicAddr
	mov dword [eax+XAPIC_EOI_OFFSET],0
	ret
getXapicAddr:
	push ecx
	push edx
	xor eax,eax
	xor edx,edx
	mov ecx,IA32_APIC_BASE_MSR
	rdmsr
	and eax,0xFFFFF000
	pop edx
	pop ecx
	ret


local_x2apic_error_msg         db 'apic error: '
local_x2apic_error_codebuff    db  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	
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