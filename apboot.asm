apargstart       equ  0x7c00
mainentry        equ  0
jumpok           equ  4
gdt_size         equ  6
gdt_base         equ  8
logcpumenutex    equ  12
logcpucount      equ  16
logcpuesp        equ  20

cr3_data         equ  0x8018
loadAddr         equ  0x4b000  ;kernel =0x3b000 大小为128个扇区 所以AP代码从0x3b000+128*521开始 ap代码在144扇区
IA32_MTRR_DEF_TYPE_MSR equ 0x2FF



    mov ax,0xb800
    xor bx,bx
    mov ds,ax
    mov cx,80                         ;32位程序应该使用ECX
    mov dx,0x0720
.cls:
    mov [bx],dx
    add bx,2
    loop .cls

    xor ax,ax
    xor bx,bx
    mov ds,ax
    
    mov bx,apargstart

    mov eax,cr3_data
    mov cr3,eax
    lgdt [apargstart+gdt_size]

   ; in al,0x92                         ;南桥芯片内的端口 
    ;or al,0000_0010B
   ; out 0x92,al                        ;打开A20
		 
    cli                                ;保护模式下中断机制尚未建立，应 
                                           ;禁止中断 

    mov eax,cr0
    or eax,0x80000001  ;开启分页
    mov cr0,eax                        ;设置PE位
						;以下进入保护模式... ...
	jmp dword 0x0008:loadAddr+start32
[bits 32]
start32:
    mov eax,0000000000010_000B         ;加载数据段选择子(0x10)
    mov es,eax
    mov ds,eax
    mov gs,eax
    mov fs,eax	
    

	mov ecx,IA32_MTRR_DEF_TYPE_MSR
	xor eax,eax
	xor edx,edx
	rdmsr
	or eax,0x800
	wrmsr

	mov eax,cr0
	and eax,0x9FFFFFFF
	mov cr0,eax

	mov eax,cr4
	or eax,0x80 ;Enables global pagesPGE designated with G flag
	mov cr4,eax
	mov eax,cr3
	mov cr3,eax
	wbinvd

spin_lock:
    mov eax,1
    xchg  eax, [apargstart+logcpumenutex]
    test eax,eax
    jnz   spin_lock
    mov ebx,[apargstart+logcpucount]
    LOCK inc dword [apargstart+logcpucount]
    mov   eax,0
    xchg  eax, [apargstart+logcpumenutex]

canjump:
    cmp word [apargstart+jumpok],1
    jnz canjump

    shl ebx,3
    mov eax,[ebx+apargstart+logcpuesp+4]
    mov ss,eax
    mov eax,[ebx+apargstart+logcpuesp]
    mov esp,eax
    jmp [apargstart+mainentry]

apmesg  db 'apmesg =============',0

times 1024 -($-$$) db 0



    
