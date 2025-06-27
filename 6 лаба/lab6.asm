.386P
.MODEL LARGE

CODE_RM segment para use16
CODE_RM_START = $
    assume cs:CODE_RM,DS:DATA,ES:DATA
MAIN:
    mov ax,DATA
    mov ds,ax
    mov es,ax
    lea dx,PROMPT_TIME
    mov ah,9h
    int 21h
    call GET_INPUT
    mov ds:[DURATION], al
    call FORMAT_CR0_BUFFER
    lea dx, CR0_BUFFER_RM
    mov ah, 9h
    int 21h
    lea dx,WELCOME_MSG
    mov ah,9h
    int 21h
    mov ah,7h
    int 21h
ENABLE_A20:
    in al,92h
    or al,2
    out 92h,al
STORE_MASKS:
    in al,21h
    mov IRQ_MASK_M,al
    in al,0A1h
    mov IRQ_MASK_S,al
DISABLE_INTERRUPTS:
    cli
    in al,70h
    or al,10000000b
    out 70h,al
    nop
SETUP_GDT:
    mov ax,DATA
    mov dl,ah
    xor dh,dh
    shl ax,4
    shr dx,4
    mov si,ax
    mov di,dx
CONFIG_GDT:
    lea bx,GDT_GDT
    mov ax,si
    mov dx,di
    add ax,offset GDT
    adc dx,0
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
CONFIG_CODE_RM:
    lea bx,GDT_CODE_RM
    mov ax,cs
    xor dh,dh
    mov dl,ah
    shl ax,4
    shr dx,4
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
CONFIG_DATA:
    lea bx,GDT_DATA
    mov ax,si
    mov dx,di
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
CONFIG_STACK:
    lea bx, GDT_STACK
    mov ax,ss
    xor dh,dh
    mov dl,ah
    shl ax,4
    shr dx,4
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
CONFIG_CODE_PM:
    lea bx,GDT_CODE_PM
    mov ax,CODE_PM
    xor dh,dh
    mov dl,ah
    shl ax,4
    shr dx,4
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
CONFIG_IDT:
    lea bx,GDT_IDT
    mov ax,si
    mov dx,di
    add ax,OFFSET IDT
    adc dx,0
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
    mov IDTR.IDT_L,ax
    mov IDTR.IDT_H,dx
INIT_IDT:
    irpc N, 0123456789ABCDEF
        lea eax, EXC_0&N
        mov IDT_0&N.OFFS_L,ax
        shr eax, 16
        mov IDT_0&N.OFFS_H,ax
    endm
    irpc N, 0123456789ABCDEF
        lea eax, EXC_1&N
        mov IDT_1&N.OFFS_L,ax
        shr eax, 16
        mov IDT_1&N.OFFS_H,ax
    endm
    lea eax, HANDLE_TIMER
    mov IDT_TIMER.OFFS_L,ax
    shr eax, 16
    mov IDT_TIMER.OFFS_H,ax
    lea eax, HANDLE_KEYBOARD
    mov IDT_KEYBOARD.OFFS_L,ax
    shr eax, 16
    mov IDT_KEYBOARD.OFFS_H,ax
    irpc N, 234567
        lea eax,STUB_IRQ_MASTER
        mov IDT_2&N.OFFS_L, AX
        shr eax,16
        mov IDT_2&N.OFFS_H, AX
    endm
    irpc N, 89ABCDEF
        lea eax,STUB_IRQ_SLAVE
        mov IDT_2&N.OFFS_L,ax
        shr eax,16
        mov IDT_2&N.OFFS_H,ax
    endm
    lgdt fword ptr GDT_GDT
    lidt fword ptr IDTR
    mov eax,cr0
    or al,00000001b
    mov cr0,eax
RELOAD_CS:
    db 0eah
    dw offset RELOAD_SEGMENTS
    dw CODE_RM_DESC
RELOAD_SEGMENTS:
    mov ax,DATA_DESC
    mov ds,ax
    mov es,ax
    mov ax,STACK_DESC
    mov ss,ax
    xor ax,ax
    mov fs,ax
    mov gs,ax
    lldt ax
PREPARE_RETURN:
    push cs
    push offset RETURN_RM
    lea edi,START_PM
    mov eax,CODE_PM_DESC
    push eax
    push edi
REINIT_PIC_PM:
    mov al,00010001b
    out 20h,al
    out 0A0h,al
    mov al,20h
    out 21h,al
    mov al,28h
    out 0A1h,al
    mov al,04h
    out 21h,al
    mov al,02h
    out 0A1h,al
    mov al,11h
    out 21h,al
    mov al,01h
    out 0A1h,al
    mov al, 0
    out 21h,al
    out 0A1h,al
ENABLE_INTERRUPTS:
    in al,70h
    and al,01111111b
    out 70h,al
    nop
    sti
JUMP_TO_PM:
    db 66h
    retf
RETURN_RM:
    cli
    in al,70h
    or AL,10000000b
    out 70h,AL
    nop
REINIT_PIC:
    mov al,00010001b
    out 20h,al
    out 0A0h,al
    mov al,8h
    out 21h,al
    mov al,70h
    out 0A1h,al
    mov al,04h
    out 21h,al
    mov al,02h
    out 0A1h,al
    mov al,11h
    out 21h,al
    mov al,01h
    out 0A1h,al
SETUP_SEGMENTS:
    mov GDT_CODE_RM.LIMIT,0FFFFh
    mov GDT_DATA.LIMIT,0FFFFh
    mov GDT_STACK.LIMIT,0FFFFh
    db 0EAH
    dw offset CONTINUE
    dw CODE_RM_DESC
CONTINUE:
    mov ax,DATA_DESC
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ax,STACK_DESC
    mov ss,ax
SWITCH_TO_RM:
    mov eax,cr0
    and al,11111110b
    mov cr0,eax
    db 0EAH
    dw offset CONTINUE2
    dw CODE_RM
CONTINUE2:
    mov ax,STACK_A
    mov ss,ax
    mov ax,DATA
    mov ds,ax
    mov es,ax
    xor ax,ax
    mov fs,ax
    mov gs,ax
    mov IDTR.LIMIT, 3FFH
    mov dword ptr IDTR+2, 0
    lidt fword ptr IDTR
RESTORE_MASKS:
    mov al,IRQ_MASK_M
    out 21h,al
    mov al,IRQ_MASK_S
    out 0A1h,al
ENABLE_INTERRUPTS_RM:
    in al,70h
    and al,01111111b
    out 70h,al
    nop
    sti
DISABLE_A20:
    in al,92h
    and al,11111101b
    out 92h, al
TERMINATE:
    mov ax,3h
    int 10H
    lea dx,EXIT_MSG
    mov ah,9h
    int 21h
    call FORMAT_CR0_BUFFER
    lea dx, CR0_BUFFER_RM
    mov ah, 9h
    int 21h
    mov ax,4C00h
    int 21H

GET_INPUT proc near
    mov ah,0ah
    xor di,di
    mov dx,offset ds:[INPUT_BUFFER]
    int 21h
    mov dl,0ah
    mov ah,02
    int 21h
    mov si,offset INPUT_BUFFER+2
    cmp byte ptr [si],"-"
    jnz ii1
    mov di,1
    inc si
II1:
    xor ax,ax
    mov bx,10
II2:
    mov cl,[si]
    cmp cl,0dh
    jz ii3
    cmp cl,'0'
    jl er
    cmp cl,'9'
    ja er
    sub cl,'0'
    mul bx
    add ax,cx
    inc si
    jmp ii2
ER:
    mov dx, offset ERROR_MSG
    mov ah,09
    int 21h
    int 20h
II3:
    ret
GET_INPUT endp

FORMAT_CR0_BUFFER proc near
    push eax
    push esi
    push dx
    mov eax, cr0
    xor dx, dx
    mov cx, 32
    lea esi, CR0_BUFFER_RM
format_loop:
    mov dl, al
    shl dl, 7
    shr dl, 7
    shr eax, 1
    add dl, 48
    mov [esi], dl
    inc esi
    xor dl, dl
    loop format_loop
    pop dx
    pop esi
    pop eax
    ret
FORMAT_CR0_BUFFER endp

CODE_RM_SIZE = ($ - CODE_RM_START)
CODE_RM ends

CODE_PM segment para use32
CODE_PM_START = $
    assume cs:CODE_PM,ds:DATA,es:DATA
START_PM:
    call CLEAR_SCREEN
    xor edi,edi
    lea esi,PM_WELCOME
    call DISPLAY_TEXT
    add edi,160
    lea esi,KEYBOARD_PROMPT
    call DISPLAY_TEXT
    mov edi,320
    lea esi,TIME_PROMPT
    call DISPLAY_TEXT
    mov edi,480
    lea esi,COUNTER_PROMPT
    call DISPLAY_TEXT
    call FORMAT_CR0
    mov edi, 640
    lea esi, CR0_BUFFER
    call DISPLAY_TEXT
    mov DS:[COUNTER],0
WAIT_ESC:
    jmp WAIT_ESC
EXIT_PM:
    db 66H
    retf
EXIT_INT:
    popad
    pop es
    pop ds
    pop eax
    pop eax
    pop eax
    sti
    db 66H
    retf

NUM_TO_DEC proc near
    pushad
    movzx eax,ax
    xor cx,cx
    mov bx,10
loop1:
    xor dx,dx
    div bx
    add dl,'0'
    push dx
    inc cx
    test ax,ax
    jnz loop1
loop2:
    pop dx
    mov [di],dl
    inc di
    loop loop2
    popad
    ret
NUM_TO_DEC endp

FORMAT_CR0 proc near
    push eax
    push esi
    push dx
    mov eax, cr0
    xor dx, dx
    mov cx, 32
    lea esi, CR0_BUFFER
format_cr0_loop:
    mov dl, al
    shl dl, 7
    shr dl, 7
    shr eax, 1
    add dl, 48
    mov [esi], dl
    inc esi
    xor dl, dl
    loop format_cr0_loop
    pop dx
    pop esi
    pop eax
    ret
FORMAT_CR0 endp

DIGIT_TO_HEX proc near
    add al,'0'
    cmp al,'9'
    jle hex_end
    add al,7
hex_end:
    ret
DIGIT_TO_HEX endp

BYTE_TO_HEX proc near
    push ax
    mov ah,al
    shr al,4
    call DIGIT_TO_HEX
    mov [di],al
    inc di
    mov al,ah
    and al,0Fh
    call DIGIT_TO_HEX
    mov [di],al
    inc di
    pop ax
    ret
BYTE_TO_HEX endp

M = 0
IRPC N, 0123456789ABCDEF
EXC_0&N label word
    cli
    jmp EXCEPTION_HANDLER
endm
M = 010H
IRPC N, 0123456789ABCDEF
EXC_1&N label word
    cli
    jmp EXCEPTION_HANDLER
endm

EXCEPTION_HANDLER proc near
    call CLEAR_SCREEN
    lea esi, EXCEPTION_MSG
    mov edi, 40*2
    call DISPLAY_TEXT
    pop eax
    pop eax
    pop eax
    sti
    db 66H
    retf
EXCEPTION_HANDLER ENDP

STUB_IRQ_MASTER proc near
    push eax
    mov al,20h
    out 20h,al
    pop eax
    iretd
STUB_IRQ_MASTER endp

STUB_IRQ_SLAVE proc near
    push eax
    mov al,20h
    out 20h,al
    out 0A0h,al
    pop eax
    iretd
STUB_IRQ_SLAVE endp

HANDLE_TIMER proc near
    push ds
    push es
    pushad
    mov ax,DATA_DESC
    mov ds,ax
    inc ds:[COUNTER]
    lea edi,ds:[COUNTER_BUFFER]
    mov ax,ds:[COUNTER]
    call NUM_TO_DEC
    mov edi,538
    lea esi,COUNTER_BUFFER
    call DISPLAY_TEXT

    inc ds:[TICK_COUNTER]
    cmp ds:[TICK_COUNTER], 18
    jb skip_second

    mov ds:[TICK_COUNTER], 0
    dec ds:[DURATION]
    cmp ds:[DURATION], 0
    je exit_protected

    mov al, ds:[DURATION]
    xor ah, ah
    lea edi, ds:[TIME_BUFFER]
    call NUM_TO_DEC
    mov edi, 356
    lea esi, TIME_BUFFER
    call DISPLAY_TEXT
    lea esi, TIME_BUFFER
    call CLEAR_BUFFER

skip_second:
    mov al,20h
    out 20h,al
    popad
    pop es
    pop ds
    iretd

exit_protected:
    mov al,20h
    out 20h,al
    db 0eah
    dd OFFSET EXIT_INT
    dw CODE_PM_DESC
HANDLE_TIMER endp

HANDLE_KEYBOARD proc near
    push ds
    push es
    pushad
    in al,60h
    cmp al,1
    je keyboard_exit
    mov ds:[SCAN_CODE],al
    lea edi,ds:[SCAN_CODE_BUFFER]
    mov al,ds:[SCAN_CODE]
    xor ah,ah
    call BYTE_TO_HEX
    mov edi,200
    lea esi,SCAN_CODE_BUFFER
    call DISPLAY_TEXT
    jmp keyboard_return
keyboard_exit:
    mov al,20h
    out 20h,al
    db 0eah
    dd OFFSET EXIT_INT
    dw CODE_PM_DESC
keyboard_return:
    mov al,20h
    out 20h,al
    popad
    pop es
    pop ds
    iretd
HANDLE_KEYBOARD endp

CLEAR_SCREEN proc near
    push es
    pushad
    mov ax,TEXT_DESC
    mov es,ax
    xor edi,edi
    mov ecx,80*25
    mov ax,700h
    rep stosw
    popad
    pop es
    ret
CLEAR_SCREEN endp

CLEAR_BUFFER proc near
    mov al,' '
    mov [esi+0],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    ret
CLEAR_BUFFER endp

DISPLAY_TEXT proc near
    push es
    PUSHAD
    mov ax,TEXT_DESC
    mov es,ax
display_loop:
    lodsb
    or al,al
    jz display_exit
    stosb
    inc edi
    jmp display_loop
display_exit:
    popad
    pop es
    ret
DISPLAY_TEXT ENDP

CODE_PM_SIZE = ($ - CODE_PM_START)
CODE_PM ENDS

DATA segment para use16
DATA_START = $
    S_DESC struc
        LIMIT dw 0
        BASE_L dw 0
        BASE_M db 0
        ACCESS db 0
        ATTRIBS db 0
        BASE_H db 0
    S_DESC ends
    I_DESC struc
        OFFS_L dw 0
        SEL dw 0
        PARAM_CNT db 0
        ACCESS db 0
        OFFS_H dw 0
    I_DESC ends
    R_IDTR struc
        LIMIT dw 0
        IDT_L dw 0
        IDT_H dw 0
    R_IDTR ends
    GDT_BEGIN   = $
    GDT label   word                          
    GDT_0       S_DESC <0,0,0,0,0,0>                                  
    GDT_GDT     S_DESC <GDT_SIZE-1,,,10010010b,0,>                 
    GDT_CODE_RM S_DESC <CODE_RM_SIZE-1,,,10011010b,0,>             
    GDT_DATA    S_DESC <DATA_SIZE-1,,,11110010b,0,>      
    GDT_STACK   S_DESC <1000h-1,,,10010010b,0,>                    
    GDT_TEXT    S_DESC <2000h-1,8000h,0Bh,11110010b,0,0> 
    GDT_CODE_PM S_DESC <CODE_PM_SIZE-1,,,10011010b,01000000b,>    
    GDT_IDT     S_DESC <IDT_SIZE-1,,,10010010b,0,>                  
    GDT_SIZE    = ($ - GDT_BEGIN)       
    CODE_RM_DESC = (GDT_CODE_RM - GDT_0)
    DATA_DESC = (GDT_DATA - GDT_0)
    STACK_DESC = (GDT_STACK - GDT_0)
    TEXT_DESC = (GDT_TEXT - GDT_0)
    CODE_PM_DESC = (GDT_CODE_PM - GDT_0)
    IDT_DESC = (GDT_IDT - GDT_0)
    IDTR R_IDTR <IDT_SIZE,0,0>
    IDT label word
    IDT_START = $
    IRPC N, 0123456789ABCDEF
        IDT_0&N I_DESC <0, CODE_PM_DESC,0,10001111b,0>
    ENDM
    IRPC N, 0123456789ABCDEF
        IDT_1&N I_DESC <0, CODE_PM_DESC, 0, 10001111b, 0>
    ENDM
    IDT_TIMER I_DESC <0,CODE_PM_DESC,0,10001110b,0>
    IDT_KEYBOARD I_DESC <0,CODE_PM_DESC,0,10001110b,0>
    IRPC N, 23456789ABCDEF
        IDT_2&N I_DESC <0, CODE_PM_DESC, 0, 10001110b, 0>
    ENDM
    IDT_SIZE = ($ - IDT_START)
    WELCOME_MSG db "Press key to change mode to PM",13,10,"$"
    PM_WELCOME db "We are in PM. Press ESC or wait till timer ends to exit PM",0
    EXIT_MSG db "We are in RM",13,10,"$"
    KEYBOARD_PROMPT db "Scan code:",0
    TIME_PROMPT db "Go back to RM in  XXXXXXX seconds",0
    COUNTER_PROMPT db "Amount of interrupt calls:",0
    EXCEPTION_MSG db "Exception: XX",0
    PROMPT_TIME db "Enter time in protected mode: $"
    ERROR_MSG db "incorrect error$"
    HEX_TABLE db "0123456789ABCDEF"
    ESP32 dd 1 dup(?)
    IRQ_MASK_M db 1 dup(?)
    IRQ_MASK_S db 1 dup(?)
    SCAN_CODE db 1 dup(?)
    DURATION db 1 dup(10)
    COUNTER dw 1 dup(0)
    TICK_COUNTER dw 0
    COUNTER_BUFFER db 8 dup(' ')
                   db 1 dup(0)
    SCAN_CODE_BUFFER db 8 dup(' ')
                     db 1 dup(0)
    TIME_BUFFER db 8 dup(' ')
                db 1 dup(0)
    INPUT_BUFFER db 6,7 dup(?)
    CR0_BUFFER db 32 dup('?')
               db 1 dup(0)
    CR0_BUFFER_RM db 32 dup('?'), 13, 10, "$"
DATA_SIZE = ($ - DATA_START)
DATA ends

STACK_A segment para stack
    db 1000h dup(?)
STACK_A ends
end MAIN