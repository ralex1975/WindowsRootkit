
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
push rcx
rdgsbase rax
swapgs
mov r10, qword ptr gs:[188h]

cmp r10, 07fffffffh
jb next

cmp byte ptr[r10+07fh], 0aah
jne next

mov rcx, [r10+96]
cmp rcx, 07fffffffh
jb next

mov [rcx+424], rax
rdfsbase rax
mov [rcx+432], rax

next:
pop rcx
pop rax
ret
HandleINTR ENDP

HandleSYSCALL PROC EXPORT
pop rcx
add rcx, 2
push rcx
push rcx

push r10

mov r10, qword ptr gs:[188h]
cmp r10, 07fffffffh
jb next
cmp byte ptr[r10+07fh], 0aah
jne next
mov r10, [r10+96]
cmp r10, 07fffffffh
jb next

push rax
push rdx

mov rcx, 102
rdmsr
shl rdx, 32
or rdx, rax
mov [r10+424], rdx
rdfsbase rax
mov [r10+432], rax

pop rdx
pop rax


next:

pop r10
mov rcx, r10
ret
HandleSYSCALL ENDP

END
