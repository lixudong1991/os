         ;�����嵥13-3
         ;�ļ�����c13.asm
         ;�ļ�˵�����û����� 
         ;�������ڣ�2011-10-30 15:19   
         
;===============================================================================
SECTION header vstart=0

         program_length   dd program_end          ;�����ܳ���#0x00
         
         head_len         dd header_end           ;����ͷ���ĳ���#0x04

         stack_seg        dd 0                    ;���ڽ��ն�ջ��ѡ����#0x08
         stack_len        dd 1                    ;������Ķ�ջ��С#0x0c
                                                  ;��4KBΪ��λ
                                                  
         prgentry         dd start                ;�������#0x10 
         code_seg         dd section.code.start   ;�����λ��#0x14
         code_len         dd code_end             ;����γ���#0x18

         data_seg         dd section.data.start   ;���ݶ�λ��#0x1c
         data_len         dd data_end             ;���ݶγ���#0x20
             
;-------------------------------------------------------------------------------
         ;���ŵ�ַ������
         salt_items       dd (header_end-salt)/256 ;#0x24
         
         salt:                                     ;#0x28
         PrintString      db  'puts_s'
                     times 256-($-PrintString) db 0
                     
         TerminateProgram db  'exit_s'
                     times 256-($-TerminateProgram) db 0
                     
         ReadDiskData     db  'readSectors_s'
                     times 256-($-ReadDiskData) db 0
                 
header_end:

;===============================================================================
SECTION data vstart=0    
                         
         buffer times 1024 db  0         ;������

         message_1         db  0x0d,0x0a,0x0d,0x0a
                           db  '**********User program is runing**********'
                           db  0x0d,0x0a,0
         message_2         db  '  Disk data:',0x0d,0x0a,0

data_end:

;===============================================================================
      [bits 32]
;===============================================================================
SECTION code vstart=0
start:
         mov eax,ds
         mov fs,eax
     
         mov eax,[stack_seg]
         mov ss,eax
         mov esp,0
     
         mov eax,[data_seg]
         mov ds,eax
     
         push dword message_1
         call far [fs:PrintString]
                                     
         push dword 1
         push dword 150        ;�߼�������100
         push dword buffer   ;������ƫ�Ƶ�ַ
         call far [fs:ReadDiskData]          ;�μ����
     
         push dword message_2
         call far [fs:PrintString]
     
         push dword buffer 
         call far [fs:PrintString]           ;too.
          
         push dword 2
         call far [fs:TerminateProgram]       ;������Ȩ���ص�ϵͳ 
      
code_end:

;===============================================================================
SECTION trail
;-------------------------------------------------------------------------------
program_end:
        times 512 db 0