.CODE

NUM_ITER equ 0300000000h
;NUM_ITER equ 01h

ConfigureMPX PROC EXPORT
mov rax, 18h
mov rdx, 0
byte 48h, 0fh, 0aeh, 29h  ;xrstor64 [rcx] 
ret
ConfigureMPX ENDP

LoopHead macro
mov rdx, 018fh
mov rax, 0; rcx;064h
byte 0f3h, 0fh, 01bh, 4, 010h  ;bndmk  bnd0,[rax+rdx*1]
mov rdx, NUM_ITER
next:
endm

LoopTail macro
sub rdx, 1
jne next
ret
endm


MPXLoopEmpty PROC EXPORT
LoopHead
nop
nop
nop
nop
mov rax, [rcx]
LoopTail
MPXLoopEmpty ENDP


MPXLoopReg PROC EXPORT
LoopHead
byte 0f3h, 0fh, 01ah, 0c1h ;bndcl  bnd0,rcx 
byte 0f3h, 0fh, 01ah, 0c1h ;bndcl  bnd0,rcx 
byte 0f3h, 0fh, 01ah, 0c1h ;bndcl  bnd0,rcx 
byte 0f3h, 0fh, 01ah, 0c1h ;bndcl  bnd0,rcx 
mov rax, [rcx]
LoopTail
MPXLoopReg ENDP


MPXLoopMem PROC EXPORT
LoopHead
byte 0f3h, 0fh, 01ah, 1  ;bndcl  bnd0,[rcx] 
byte 0f3h, 0fh, 01ah, 1  ;bndcl  bnd0,[rcx] 
byte 0f3h, 0fh, 01ah, 1  ;bndcl  bnd0,[rcx] 
byte 0f3h, 0fh, 01ah, 1  ;bndcl  bnd0,[rcx] 
mov rax, [rcx]
LoopTail
MPXLoopMem ENDP


MPXLoop PROC EXPORT
LoopHead
shl rcx, 6
shr rcx, 6
shl rcx, 6
shr rcx, 6
shl rcx, 6
shr rcx, 6
shl rcx, 6
shr rcx, 6
mov rax, [rcx]
LoopTail
MPXLoop ENDP


WriteFSBase PROC EXPORT
wrfsbase rcx
ret
WriteFSBase ENDP


WriteGSBase PROC EXPORT
wrgsbase rcx
ret
WriteGSBase ENDP


ReadFSBase PROC EXPORT
rdfsbase rax
ret
ReadFSBase ENDP


ReadGSBase PROC EXPORT
rdgsbase rax
ret
ReadGSBase ENDP

END
