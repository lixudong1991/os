
global exceptionCalls,general_interrupt_handler,interrupt_8259a_handler,interrupt_70_handler,interrupt_80_handler,interrupt_81_handler,interrupt_82_handler,interrupt_83_handler,interrupt_84_handler
;global interrupt_85_handler
extern   kernelData,general_exeption_code,general_exeption_no_code,apicTimeOut,systemCall,apicError,updataGdt,processorMtrrSync,sleepTimeOut
IA32_X2APIC_EOI equ 0x80B
IA32_X2APIC_ESR equ 0x828
IA32_X2APIC_INIT_COUNT equ 0x838
IA32_APIC_BASE_MSR equ 0x1B

XAPIC_ID_OFFSET  equ 0x20
XAPIC_LDR_OFFSET  equ 0xD0
XAPIC_InitialCount_OFFSET  equ 0x380
XAPIC_ErrStatus_OFFSET  equ 0x280
XAPIC_EOI_OFFSET  equ 0xB0
general_exception0_handler:
	push dword [esp]
	push dword 0
	call general_exeption_no_code
	add esp,8
	hlt
general_exception1_handler:
	push dword [esp]
	push dword 1
	call general_exeption_no_code
	add esp,8
	hlt
general_exception2_handler:
	push dword [esp]
	push dword 2
	call general_exeption_no_code
	add esp,8
	hlt
general_exception3_handler:
	push dword [esp]
	push dword 3
	call general_exeption_no_code
	add esp,8
	hlt
general_exception4_handler:
	push dword [esp]
	push dword 4
	call general_exeption_no_code
	add esp,8
	hlt
general_exception5_handler:
	push dword [esp]
	push dword 5
	call general_exeption_no_code
	add esp,8
	hlt
general_exception6_handler:
	push dword [esp]
	push dword 6
	call general_exeption_no_code
	add esp,8
	hlt
general_exception7_handler:
	push dword [esp]
	push dword 7
	call general_exeption_no_code
	add esp,8
	hlt
general_exception8_handler:
	push dword [esp]
	push dword 8
	call general_exeption_no_code
	add esp,8
	hlt
general_exception9_handler:
	push dword [esp]
	push dword 9
	call general_exeption_no_code
	add esp,8
	hlt
general_exception10_handler:
	mov eax,[esp]
	mov ebx,[esp+4]
	push ebx
	push eax
	push dword 10
	call general_exeption_code
	add esp,12
	hlt
general_exception11_handler:
	mov eax,[esp]
	mov ebx,[esp+4]
	push ebx
	push eax
	push dword 11
	call general_exeption_code
	add esp,12
	hlt
general_exception12_handler:
	mov eax,[esp]
	mov ebx,[esp+4]
	push ebx
	push eax
	push dword 12
	call general_exeption_code
	add esp,12
	hlt
general_exception13_handler:
    mov eax,[esp]
	mov ebx,[esp+4]
	push ebx
	push eax
	push dword 13
	call general_exeption_code
	add esp,12
	hlt
general_exception14_handler:
    mov eax,[esp]
	mov ebx,[esp+4]
	push ebx
	push eax
	push dword 14
	call general_exeption_code
	add esp,12
	hlt
general_exception15_handler:
	push dword 15
	call general_exeption_no_code
	add esp,4
	hlt
general_exception16_handler:
	push dword 16
	call general_exeption_no_code
	add esp,4
	hlt
general_exception17_handler:
	push dword 17
	call general_exeption_no_code
	add esp,4
	hlt
general_exception18_handler:
	push dword 18
	call general_exeption_no_code
	add esp,4
	hlt
general_exception19_handler:
	push dword 19
	call general_exeption_no_code
	add esp,4
	hlt

general_interrupt_handler:
    iretd

interrupt_8259a_handler:
    push eax     
    mov al,0x20                        ;中断结束命令EOI 
    out 0xa0,al                        ;向从片发送 
    out 0x20,al                        ;向主片发送       
    pop eax        
    iretd

interrupt_70_handler:
    push eax   
	push ebx 
	push edx
	push esi
         mov al,0x20                        ;中断结束命令EOI
         out 0xa0,al                        ;向8259A从片发送
         out 0x20,al                        ;向8259A主片发送

         mov al,0x0c                        ;寄存器C的索引。且开放NMI
         out 0x70,al
         in al,0x71                         ;读一下RTC的寄存器C，否则只发生一次中断
                                          ;此处不考虑闹钟和周期性中断的情况  										  
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

interrupt_80_handler:
	call systemCall
	iretd

interrupt_81_handler:
	call apicError
	iretd
 interrupt_82_handler:
 	call apicTimeOut
 	iretd
interrupt_83_handler:
	call updataGdt
	iretd
interrupt_84_handler:
	call processorMtrrSync
	iretd
;interrupt_85_handler:
;	call sleepTimeOut
;	iretd
; x2ApicTimeOut:
;     push eax   
; 	push ecx 
; 	push edx
; 	push esi									  
;     mov ecx,kernelData
; 	test dword [ecx+28],0xffffffff
; 	je ApicTimeOutret
; 	cmp dword [ecx+28],1
; 	je ApicTimeOutret
; 	mov esi,[ecx+32]
; ApicTimeOutnexttask:
; 	mov esi,[esi]
; 	cmp esi,0
; 	jne ApicTimeOutnexttask1
; 	mov esi,[ecx+20]
; ApicTimeOutnexttask1:
; 	cmp esi,[ecx+32]
; 	je ApicTimeOutret
; 	test dword [esi+0x10],0xffffffff
; 	jnz ApicTimeOutnexttask
; 	mov edx,[ecx+32]
; 	mov dword [edx+0x10],0
; 	mov [ecx+32],esi
; 	mov dword [esi+0x10],1
; 	cmp esi,[ecx+20]
; 	je noSetTimer
; 	mov eax,0xffff  ;task cpu time
; 	xor edx,edx
; 	mov ecx,IA32_X2APIC_INIT_COUNT
; 	mfence
; 	wrmsr
; noSetTimer:
; 	call writeEOI
; 	jmp far [esi+8]
; 	jmp ApicTimeOutret0
; ApicTimeOutret:
; 	call writeEOI
; ApicTimeOutret0:	
; 	pop esi
; 	pop edx
; 	pop ecx
;     pop eax  
; 	iretd
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
; systemCall:
; 	push systemCallmsg
; 	call puts_int
; 	add esp,4
; 	push edx
; 	call getXapicAddr
; 	mov edx,eax
; 	mov eax,[edx+XAPIC_ID_OFFSET]
; 	push eax
; 	push dword excep_codebuff
; 	call hexstr32
; 	add esp,8
; 	push excep_codebuffstr
; 	call puts_int
; 	add esp,4

; 	mov eax,[edx+XAPIC_LDR_OFFSET]
; 	push eax
; 	push dword excep_codebuff
; 	call hexstr32
; 	add esp,8
; 	push excep_codebuffstr
; 	call puts_int
; 	add esp,4
; 	pop edx
; 	call xapicwriteEOI
; 	iretd

; local_x2apic_error_handling:
; 	push ecx
; 	push edx
; 	xor eax,eax
; 	xor edx,edx
; 	mov ecx,IA32_X2APIC_ESR
; 	mfence
; 	wrmsr
; 	xor eax,eax
; 	xor edx,edx
; 	mov ecx,IA32_X2APIC_ESR
;     mfence
; 	rdmsr
; 	push eax
; 	push dword local_x2apic_error_codebuff
; 	call hexstr32
; 	add esp,8
; 	push local_x2apic_error_msg
; 	call puts_int
; 	add esp,4
; 	pop edx
; 	pop ecx
; 	call writeEOI
; 	iretd

; writeEOI:
; 	push edx
; 	push ecx
; 	xor edx,edx
; 	xor eax,eax
; 	mov ecx,IA32_X2APIC_EOI
; 	mfence
; 	wrmsr
;     pop ecx
; 	pop edx
; 	ret
; local_xapic_error_handling:
; 	push ecx
; 	push edx
; 	xor eax,eax
; 	xor edx,edx
; 	mov ecx,IA32_APIC_BASE_MSR
;     mfence
; 	rdmsr
; 	and eax,0xFFFFF000
; 	mov edx,eax
; 	mov dword [edx+XAPIC_ErrStatus_OFFSET],0
; 	mov eax,[edx+XAPIC_ErrStatus_OFFSET]
; 	push eax
; 	push dword local_x2apic_error_codebuff
; 	call hexstr32
; 	add esp,8
; 	push local_x2apic_error_msg
; 	call puts_int
; 	add esp,4
; 	call xapicwriteEOI
; 	pop edx
; 	pop ecx
; 	iretd
; interrupt_82_handler:
;     push eax   
; 	push ecx 
; 	push edx
; 	push esi									  
;     mov ecx,kernelData
; 	test dword [ecx+28],0xffffffff
; 	je xApicTimeOutret
; 	cmp dword [ecx+28],1
; 	je xApicTimeOutret
; 	mov esi,[ecx+32]
; xApicTimeOutnexttask:
; 	mov esi,[esi]
; 	cmp esi,0
; 	jne xApicTimeOutnexttask1
; 	mov esi,[ecx+20]
; xApicTimeOutnexttask1:
; 	cmp esi,[ecx+32]
; 	je xApicTimeOutret
; 	test dword [esi+0x10],0xffffffff
; 	jnz xApicTimeOutnexttask
; 	mov edx,[ecx+32]
; 	mov dword [edx+0x10],0
; 	mov [ecx+32],esi
; 	mov dword [esi+0x10],1
; 	cmp esi,[ecx+20]
; 	je xnoSetTimer
; 	call x_getXapicAddr
; 	mov dword [eax+XAPIC_InitialCount_OFFSET],0xffff ;task cpu time
; xnoSetTimer:
; 	call x_apicwriteEOI
; 	jmp far [esi+8]
; 	jmp xApicTimeOutret0
; notfoundNext:
; 	call x_getXapicAddr
; 	mov dword [eax+XAPIC_InitialCount_OFFSET],0xffff ;task cpu time
; xApicTimeOutret:
; 	call x_apicwriteEOI
; xApicTimeOutret0:	
; 	pop esi
; 	pop edx
; 	pop ecx
;     pop eax  
; 	iretd

; x_apicwriteEOI:
; 	call x_getXapicAddr
; 	mov dword [eax+XAPIC_EOI_OFFSET],0
; 	ret
; x_getXapicAddr:
; 	push ecx
; 	push edx
; 	xor eax,eax
; 	xor edx,edx
; 	mov ecx,IA32_APIC_BASE_MSR
; 	rdmsr
; 	and eax,0xFFFFF000
; 	pop edx
; 	pop ecx
; 	ret
; x_getXapicId:
; 	call x_getXapicAddr
; 	mov eax,[eax+XAPIC_ID_OFFSET]
; 	shr eax,24
; 	ret
				 
exceptionCalls   dd  general_exception0_handler,general_exception1_handler,general_exception2_handler,general_exception3_handler,general_exception4_handler
				 dd  general_exception5_handler,general_exception6_handler,general_exception7_handler,general_exception8_handler,general_exception9_handler
				 dd  general_exception10_handler,general_exception11_handler,general_exception12_handler,general_exception13_handler,general_exception14_handler
				 dd  general_exception15_handler,general_exception16_handler,general_exception17_handler,general_exception18_handler,general_exception19_handler		