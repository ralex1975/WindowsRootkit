
.CODE

XchgVal PROC EXPORT
xchg rdx, [rcx]
ret
XchgVal ENDP

SetAppThread PROC EXPORT
mov rax, gs:[0188h]
mov dword ptr [rax+0996h], 1234h
ret
SetAppThread ENDP


ResetAppThread PROC EXPORT
mov rax, gs:[0188h]
mov dword ptr [rax+0996h], 1235h
ret
ResetAppThread ENDP


HandleIRETGS PROC EXPORT

; If you change anything here
; please change the PatchPico
; accordingly.

mov rax, gs:[0188h]
mov eax, [rax+0996h]

swapgs

cmp eax, 1234h
jne next
mov rax, 0FF99977770h
wrfsbase rax
mov rax, 0FF99977770h
wrgsbase rax

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

mov rbp, gs:[0188h]
mov ebp, [rbp+0996h]

swapgs

cmp ebp, 1234h
jne next

mov rbp, 0FF99977770h
wrfsbase  rbp
mov rbp, 0FF99977770h
wrgsbase  rbp

next:
mov rbp,r9
mov rsp,r8

sysretq
HandleSYSRET ENDP

END
