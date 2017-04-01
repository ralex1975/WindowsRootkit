
.CODE

XchgVal PROC EXPORT
xchg rdx, [rcx]
ret
XchgVal ENDP

SetAppThread PROC EXPORT
mov rax, gs:[0188h]
mov dword ptr [rax+0630h], 1234h
ret
SetAppThread ENDP


HandleIRETGS PROC EXPORT
mov         rax,0FFFFF77770h
wrfsbase    rax

mov  rax, gs:[0188h]

mov  eax, [rax+0630h]
cmp  eax, 1234h
jne  next
mov  rax,0FF99977770h

wrfsbase    rax
;mov         ecx,0C0000101h
;rdmsr

next:
mov r9, [rbp-30h]
mov r8, [rbp-38h]
mov rdx, [rbp-40h]
mov rcx, [rbp-48h]
mov rax, [rbp-50h]
mov rsp, rbp
mov rbp, [rbp+0D8h]
add rsp, 0E8h
swapgs

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
mov rbp, 0FF99977770h
wrfsbase rbp
mov rbp, gs:[0188h]
mov ebp, [rbp+0630h]
cmp ebp, 1234h
jne next
mov rbp, 0FF99977770h

wrfsbase  rbp
push rcx
push rax
push rdx
;mov   ecx,0C0000101h
;rdmsr
pop rdx
pop rax
pop rcx

next:
mov rbp,r9
mov rsp,r8
swapgs
sysretq
HandleSYSRET ENDP

END
