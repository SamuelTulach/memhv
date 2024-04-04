.const

KTRAP_FRAME_SIZE            equ     190h
MACHINE_FRAME_SIZE          equ     28h

.code

extern HandleExit : proc

PUSHAQ macro
        push    rax
        push    rcx
        push    rdx
        push    rbx
        push    -1
        push    rbp
        push    rsi
        push    rdi
        push    r8
        push    r9
        push    r10
        push    r11
        push    r12
        push    r13
        push    r14
        push    r15
        endm

POPAQ macro
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdi
        pop     rsi
        pop     rbp
        pop     rbx
        pop     rbx
        pop     rdx
        pop     rcx
        pop     rax
        endm

LaunchVM proc frame
        mov rsp, rcx

SvLV10:
        mov rax, [rsp]
        vmload rax

        vmrun rax

        vmsave rax

        .pushframe
        sub     rsp, KTRAP_FRAME_SIZE
        .allocstack KTRAP_FRAME_SIZE - MACHINE_FRAME_SIZE + 100h

        PUSHAQ

        mov rdx, rsp
        mov rcx, [rsp + 8 * 18 + KTRAP_FRAME_SIZE]

        sub rsp, 80h
        movaps xmmword ptr [rsp + 20h], xmm0
        movaps xmmword ptr [rsp + 30h], xmm1
        movaps xmmword ptr [rsp + 40h], xmm2
        movaps xmmword ptr [rsp + 50h], xmm3
        movaps xmmword ptr [rsp + 60h], xmm4
        movaps xmmword ptr [rsp + 70h], xmm5
        .endprolog

        call HandleExit

        movaps xmm5, xmmword ptr [rsp + 70h]
        movaps xmm4, xmmword ptr [rsp + 60h]
        movaps xmm3, xmmword ptr [rsp + 50h]
        movaps xmm2, xmmword ptr [rsp + 40h]
        movaps xmm1, xmmword ptr [rsp + 30h]
        movaps xmm0, xmmword ptr [rsp + 20h]
        add rsp, 80h

        test al, al
        POPAQ

        jnz SvLV20
        add rsp, KTRAP_FRAME_SIZE
        jmp SvLV10

SvLV20:
        mov rsp, rcx
        mov ecx, 'NNNN'
        jmp rbx
LaunchVM endp

        end
