
.CODE

XchgVal PROC EXPORT
xchg rdx, [rcx]
ret
XchgVal ENDP

SetAppThread PROC EXPORT
mov rax, gs:[0188h]
mov byte ptr [rax+07fh], 0aah
ret
SetAppThread ENDP


ResetAppThread PROC EXPORT
mov rax, gs:[0188h]
mov byte ptr [rax+07fh], 0
ret
ResetAppThread ENDP

SetFsGsBase PROC EXPORT
mov rax, gs:[0188h]
mov rax, [rax+96]
mov [rax+424], rdx
mov [rax+432], rcx
ret
SetFsGsBase ENDP


HandleIRETGS PROC EXPORT

; If you change anything here
; please change the PatchPico
; accordingly.

mov rax, gs:[0188h]
cmp rax, 07fffffffh
jb next

swapgs

cmp byte ptr[rax+07fh], 0aah
jne next

mov rax, [rax+96]
mov rcx, [rax+424]
wrgsbase rcx
mov rcx, [rax+432]
wrfsbase rcx

next:
mov r9, [rbp-30h]
mov r8, [rbp-38h]
mov rdx, [rbp-40h]
mov rcx, [rbp-48h]
mov rax, [rbp-50h]
mov rsp, rbp
mov rbp, [rbp+0D8h]
add rsp, 0E8h

iretq

HandleIRETGS ENDP


HandleIRET PROC EXPORT
mov r10, [rbp-28h]
mov r9, [rbp-30h]
mov r8, [rbp-38h]
mov rdx, [rbp-40h]
mov rcx, [rbp-48h]
mov rax, [rbp-50h]
mov rsp, rbp
mov rbp, [rbp+0D8h]
add rsp, 0E8h
iretq
HandleIRET ENDP

HandleSYSRET PROC EXPORT

; If you change anything here
; please change the PatchPico
; accordingly.

mov rbp, rax
mov rax, gs:[0188h]

swapgs

cmp rax, 07fffffffh
jb next

cmp byte ptr[rax+07fh], 0aah
jne next

push rcx

mov rax, [rax+96]
mov rcx, [rax+424]
wrgsbase rcx
mov rcx, [rax+432]
wrfsbase rcx

pop rcx

next:
mov rax, rbp
mov rbp,r9
mov rsp,r8

sysretq
HandleSYSRET ENDP

ReadIDT PROC EXPORT
sidt [rcx]
mov rax, [rcx+2]
ret
ReadIDT ENDP

HandleINTR PROC EXPORT
push rax
rdgsbase rax
swapgs
mov r10, qword ptr gs:[188h]

cmp r10, 07fffffffh
jb next

mov [r10+424], rax
rdfsbase rax
mov [r10+432], rax

next:
pop rax
ret
HandleINTR ENDP

HandleSYSCALL PROC EXPORT
push rax
push r10
rdgsbase rax
swapgs

mov r10, qword ptr gs:[188h]

cmp r10, 07fffffffh
jb next

mov [r10+424], rax
rdfsbase rax
mov [r10+432], rax

next:
pop r10
pop rax

add rsp, 8
mov qword ptr gs:[10h], rsp
sub rsp, 8
ret
HandleSYSCALL ENDP

END
