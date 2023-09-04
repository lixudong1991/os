
global setgdtr,setldtr,settr,cs_data,ds_data,ss_data,fs_data,gs_data,cpuidcall,rdmsrcall,wrmsrcall,wrmsr_fence,rdmsr_fence,setds,setgs,setfs,esp_data,cr3_data,flags_data,setBit,resetBit,testBit,allocatePhy4kPage,freePhy4kPage,sysInLong,sysOutLong,callTss,setidtr,cli_s,sti_s,invlpg_s,intcall,resetcr3,rtc_8259a_enable,interrupt8259a_disable
global _monitor,_mwait,cr0_data,set_cr0data,cr4_data,set_cr4data,pre_mtrr_change,post_mtrr_change
extern bootparam
pageStatusOffset equ 28
IA32_MTRR_DEF_TYPE_MSR equ 0x2FF
setgdtr:
	push ebx
	mov ebx,[esp+8]
	lgdt [ebx]
	pop ebx
	ret
setidtr:
	push ebx
	mov ebx,[esp+8]
	lidt [ebx]
	pop ebx
	ret
invlpg_s:
	push ebx
	mov ebx,[esp+8]
	invlpg [ebx]
	pop ebx
	ret
cli_s:
	cli
	ret
sti_s:
	sti
	ret
intcall:
	int 0x70
	ret
	
setldtr:
	lldt [esp+4]
	ret
	
settr:
	ltr [esp+4]
	ret

cs_data:
	xor eax,eax
	mov ax,cs
	ret
	
ds_data:
	xor eax,eax
	mov ax,ds
	ret
ss_data:
	xor eax,eax
	mov ax,ss
	ret
fs_data:
	xor eax,eax
	mov ax,fs
	ret
gs_data:
	xor eax,eax
	mov ax,gs
	ret
flags_data:
	pushf
	pop eax
	ret
esp_data:
	mov eax,esp
	sub eax,4
	ret
cr0_data:
	mov eax,cr0
	ret
set_cr0data:
	mov eax,[esp+4]
	mov cr0,eax
	ret
cr4_data:
	mov eax,cr4
	ret
set_cr4data:
	mov eax,[esp+4]
	mov cr4,eax
	ret
cr3_data:
	mov eax,cr3
	ret
resetcr3:
	mov eax,cr3
	mov cr3,eax
	ret
setds:
	push eax
	mov eax,[esp+8]
	mov ds,eax
	pop eax
	ret
setgs:
	push eax
	mov eax,[esp+8]
	mov gs,eax
	pop eax
	ret
setfs:
	push eax
	mov eax,[esp+8]
	mov fs,eax
	pop eax
	ret
setBit:
	push ebp
	mov ebp,esp
	push eax
	push ebx
	mov ebx,[ebp+8]
	mov eax,[ebp+0xc]
	LOCK bts dword [ebx],eax
	pop ebx
	pop eax
	pop ebp
	ret
resetBit:
	push ebp
	mov ebp,esp
	push eax
	push ebx
	mov ebx,[ebp+8]
	mov eax,[ebp+0xc]
	LOCK btr dword [ebx],eax
	pop ebx
	pop eax
	pop ebp
	ret
testBit:
	push ebp
	mov ebp,esp
	push ebx
	mov ebx,[ebp+8]
	mov eax,[ebp+0xc]
	bt dword [ebx],eax
	mov eax,1
	mov ebx,0
	cmovnb eax,ebx
	pop ebx
	pop ebp
	ret

callTss:
	push dword [esp+4]
	push dword 0
	jmp far [esp]
	add esp,8
	ret


allocatePhy4kPage:
		push edx
		push ebx		
		mov eax,[esp+0xc]
		mov edx,[bootparam+pageStatusOffset]
next4k:	LOCK bts dword [edx],eax
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
freePhy4kPage:
		push edx
		mov eax,[esp+8]
		shr eax,12
		mov edx,[bootparam+pageStatusOffset]
		LOCK btr dword [edx],eax
		mov eax,1
		jb  freePhy4kPageret
		xor eax,eax
freePhy4kPageret:
		pop edx
		ret
;allocateVirtual4kPage:
;		push ebp
;		mov ebp,esp
;		push ecx
;		push ebx
;		push edx
;		mov ecx,[ebp+8]
;		mov ebx,[ebp+0xc]
;		mov ebx,[ebx]
;allcate0:
;		mov edx,ebx
;		shr edx,22
;		shl edx,2
;		test dword [0xfffff000+edx],1
;		jne addPhyaddr
;		push dword 0
;		call allocatePhy4kPage
;		add esp,4
;		or eax,7
;		mov [0xfffff000+edx],eax
;addPhyaddr:
;		shr edx,2
;		shl edx,12
;		or  edx,0xffc00000
;		mov eax,ebx
;		and eax,0x3FF000
;		shr eax,10
;		add edx,eax
;		test dword [edx],1
;		jne allcate1
;		push dword 0
;		call allocatePhy4kPage
;		add esp,4
;		or eax,[ebp+0x10]
;		mov [edx],eax
;allcate1:		
;		add ebx,0x1000
;		loop allcate0
;		mov edx,[ebp+0xc]
;		mov eax,[edx]
;		mov [edx],ebx
;		pop edx
;		pop ebx
;		pop ecx
;		pop ebp
;		ret



;Elf32_Ehdr 
;e_ident       equ      0 ;times 16 db 0
;e_type        equ      16;dw 0
;e_machine     equ      18;dw 0;		/* Architecture */
;e_version     equ      20;dd 0;		/* Object file version */
;e_entry       equ      24;dd 0;		/* Entry point virtual address */
;e_phoff       equ      28;dd 0;		/* Program header table file offset */
;e_shoff       equ      32;dd 0;		/* Section header table file offset */
;e_flags       equ      36;dd 0;		/* Processor-specific flags */
;e_ehsize      equ      40;dw 0;		/* ELF header size in bytes */
;e_phentsize   equ      42;dw 0;		/* Program header table entry size */
;e_phnum       equ      44;dw 0;		/* Program header table entry count */
;e_shentsize   equ      46;dw 0;		/* Section header table entry size */
;e_shnum       equ      48;dw 0;		/* Section header table entry count */
;e_shstrndx    equ      50;dw 0;		/* Section header string table index */
;
;;Elf32_Phdr   
;p_type        equ      0;dd 0;			/* Segment type */
;p_offset      equ      4;dd 0;		/* Segment file offset */
;p_vaddr       equ      8;dd 0;		/* Segment virtual address */
;p_paddr       equ      12;dd 0;		/* Segment physical address */
;p_filesz      equ      16;dd 0;		/* Segment size in file */
;p_memsz       equ      20;dd 0;		/* Segment size in memory */
;p_flags       equ      24;dd 0;		/* Segment flags */
;p_align       equ      28;dd 0;		/* Segment alignment */		
;
;
;PT_LOAD		equ 1
;
;PF_X   equ 1
;PF_W   equ 2
;PF_R   equ 4
;
;;0600h+vir_base-osstart	
;loadElf:
;		push ebp
;		mov ebp,esp
;		push eax
;		push ebx
;		push ecx
;		push edx
;		push esi
;		mov ebx,[ebp+8]
;		mov eax,[ebp+0xc]
;		push dword [ebx+e_entry]
;		pop  dword [eax]
;		movzx ecx,word [ebx+e_phnum]
;		movzx edx,word [ebx+e_phentsize]
;		mov esi,[ebx+e_phoff]	
;loadElf0:
;		push eax
;		push esi
;		push ebx
;		call loadSeg
;		add esp,0xc
;		add esi,edx
;		loop loadElf0
;		pop esi
;		pop edx
;		pop ecx
;		pop ebx
;		pop eax
;		pop ebp
;		ret	
;loadSeg:
;		push ebp
;		mov ebp,esp
;		push eax
;		push ebx
;		push ecx
;		push edx
;		push esi
;		push edi
;		
;		mov ebx,[ebp+8]
;		mov eax,[ebp+0xc]
;		cmp dword [ebx+eax+p_type],PT_LOAD
;		jnz loadSeg0
;		mov ecx,[ebx+eax+p_memsz]
;		test ecx,ecx
;		je loadSeg0
;		mov esi,[ebp+0x10]
;		add esi,4
;		mov ecx,[esi]
;		mov edi,[ebx+eax+p_vaddr]
;		cmp edi,ecx	
;		cmovb ecx,edi
;		mov [esi],ecx
;		push eax
;		mov ecx,[ebx+eax+p_align]
;		mov eax,[ebx+eax+p_memsz]
;		xor edx,edx
;		div ecx
;		pop eax	
;		sub ecx,edx
;		dec ecx
;		add edi,ecx
;		add edi,[ebx+eax+p_memsz]
;		mov ecx,[esi+4]
;		cmp edi,ecx
;		cmova ecx,edi
;		mov [esi+4],ecx
;		and ecx,0xfffff000
;		mov edx,[ebx+eax+p_vaddr]
;		and edx,0xfffff000
;		push eax
;		sub esp,8
;		mov ebp,esp
;		mov dword [ebp+4],5
;		test dword [ebx+eax+p_flags],PF_W
;		jz loadSeg1
;		mov dword [ebp+4],7
;loadSeg1:		
;		mov [ebp],edx
;		push dword [ebp+4]
;		push ebp
;		push 1
;		call allocateVirtual4kPage
;		add esp,12
;		cmp edx,ecx
;		jae copyfileData
;		add edx,0x1000
;		jmp loadSeg1
;copyfileData:	
;		add esp,8
;		pop eax
;		mov ecx,[ebx+eax+p_filesz]
;		test ecx,ecx
;		je loadSeg0
;		mov edi,[ebx+eax+p_vaddr]
;		mov esi,[ebx+eax+p_offset]
;		add esi,ebx
;		mov ecx,[ebx+eax+p_filesz]
;		cld
;		rep movsb
;loadSeg0:
;		pop edi
;		pop esi
;		pop edx
;		pop ecx
;		pop ebx
;		pop eax
;		pop ebp
;		ret

sysInLong:
	push edx
	mov  dx,[esp+8]
	in   eax,dx
	pop edx
	ret
sysOutLong:
	push edx
	push eax
	mov dx,[esp+0xc]
	mov eax,[esp+0xe]
	out dx,eax
	pop eax
	pop edx
	ret



cpuidcall:
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	xor ebx,ebx
	xor ecx,ecx
	xor edx,edx
	mov eax,[ebp+8]
	cpuid
	push ebx
	mov ebx,[ebp+0xc]
	mov [ebx],eax
	pop ebx
	mov eax,[ebp+0x10]
	mov [eax],ebx
	mov eax,[ebp+0x14]
	mov [eax],ecx
	mov eax,[ebp+0x18]
	mov [eax],edx
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret




rdmsrcall:
	push ebp
	mov ebp,esp
	push ecx
	push edx
	xor eax,eax
	xor edx,edx
	mov ecx,[ebp+8]
	rdmsr
	mov ecx,[ebp+0xc]
	mov [ecx],eax
	mov ecx,[ebp+0x10]
	mov [ecx],edx
    pop edx
	pop ecx
	pop ebp
	ret

wrmsrcall:
	push ebp
	mov ebp,esp
	push ecx
	push edx
	mov ecx,[ebp+8]
	mov eax,[ebp+0xc]
	mov edx,[ebp+0x10]
	wrmsr
    pop edx
	pop ecx
	pop ebp
	ret

wrmsr_fence:
	push ebp
	mov ebp,esp
	push ecx
	push edx
	mov ecx,[ebp+8]
	mov eax,[ebp+0xc]
	mov edx,[ebp+0x10]
	mfence
	wrmsr
    pop edx
	pop ecx
	pop ebp
	ret
rdmsr_fence:
	push ebp
	mov ebp,esp
	push ecx
	push edx
	xor eax,eax
	xor edx,edx
	mov ecx,[ebp+8]
	mfence
	rdmsr
	mov ecx,[ebp+0xc]
	mov [ecx],eax
	mov ecx,[ebp+0x10]
	mov [ecx],edx
    pop edx
	pop ecx
	pop ebp
	ret

_monitor:
	push ebp
	mov ebp,esp
	push ecx
	push edx
	mov eax,[ebp+8]
	mov ecx,[ebp+0xc]
	mov edx,[ebp+0x10]
	monitor
	pop edx
	pop ecx
	pop ebp
	ret
_mwait:
	push ecx
	mov ecx,[esp+0x8]
	mov eax,[esp+0xc]
	mwait
	pop ecx
	ret
pre_mtrr_change:
	push ecx
	push edx
	cli
	mov eax,cr0
	or eax,0x40000000
	and eax,0xDFFFFFFF
	mov cr0,eax
	wbinvd
	mov eax,cr4	
	push eax
	and eax,0xFFFFFF7F
	mov cr4,eax

	mov eax,cr3
	mov cr3,eax
	
	mov ecx,IA32_MTRR_DEF_TYPE_MSR
	xor eax,eax
	xor edx,edx
	rdmsr
	and eax,0x4FF
	wrmsr
	pop eax
	pop edx
	pop ecx
	ret
post_mtrr_change:
	push ecx
	push edx
	wbinvd
	mov eax,cr3
	mov cr3,eax

	mov ecx,IA32_MTRR_DEF_TYPE_MSR
	xor eax,eax
	xor edx,edx
	rdmsr
	or eax,0x800
	wrmsr

	mov eax,cr0
	and eax,0x9FFFFFFF
	mov cr0,eax

	mov eax,[esp+0xc]
	mov cr4,eax
	mov eax,cr3
	mov cr3,eax
	wbinvd
	sti
	pop edx
	pop ecx
	ret
rtc_8259a_enable:
	push eax
	         ;设置8259A中断控制器
    mov al,0x11
    out 0x20,al                        ;ICW1：边沿触发/级联方式
    mov al,0x20
    out 0x21,al                        ;ICW2:起始中断向量
    mov al,0x04
    out 0x21,al                        ;ICW3:从片级联到IR2
    mov al,0x01
    out 0x21,al                        ;ICW4:非总线缓冲，全嵌套，正常EOI

    mov al,0x11
    out 0xa0,al                        ;ICW1：边沿触发/级联方式
    mov al,0x70
    out 0xa1,al                        ;ICW2:起始中断向量
    mov al,0x02
    out 0xa1,al                        ;ICW3:从片级联到IR2
    mov al,0x01
    out 0xa1,al                        ;ICW4:非总线缓冲，全嵌套，正常EOI

	         ;设置和时钟中断相关的硬件 
         mov al,0x0b                        ;RTC寄存器B
         or al,0x80                         ;阻断NMI
         out 0x70,al
         mov al,0x42                        ;设置寄存器B，周期性中断，开放更
         out 0x71,al                        ;新结束后中断，BCD码，24小时制

		 mov al,0x0a
		 or al,0x80
		 out 0x70,al
		 in al,0x71
		 or al,0x0c							;设置中断时间为62.5ms  0x0d=125ms, 0x0e=250ms,0x0f=500ms,0x07=1.9ms
		 out 0x71,al



         in al,0xa1                         ;读8259从片的IMR寄存器
         and al,0xfe                        ;清除bit 0(此位连接RTC)
         out 0xa1,al                        ;写回此寄存器

         mov al,0x0c
         out 0x70,al
         in al,0x71                         ;读RTC寄存器C，复位未决的中断状态
	pop eax
	ret

interrupt8259a_disable:
	push eax
	mov al,0xff 
    out 0x21,al                        ;禁用主片所有中断

    mov al,0xff                      
    out 0xa1,al                        ;禁用从片所有中断
	         ;设置8259A中断控制器
    mov al,0x11
    out 0x20,al                        ;ICW1：边沿触发/级联方式
    mov al,0x20
    out 0x21,al                        ;ICW2:起始中断向量
    mov al,0x04
    out 0x21,al                        ;ICW3:从片级联到IR2
    mov al,0x01
    out 0x21,al                        ;ICW4:非总线缓冲，全嵌套，正常EOI

    mov al,0x11
    out 0xa0,al                        ;ICW1：边沿触发/级联方式
    mov al,0x70
    out 0xa1,al                        ;ICW2:起始中断向量
    mov al,0x02
    out 0xa1,al                        ;ICW3:从片级联到IR2
    mov al,0x01
    out 0xa1,al                        ;ICW4:非总线缓冲，全嵌套，正常EOI
	pop eax
	ret	
