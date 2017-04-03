.CODE

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

ExecuteNop PROC EXPORT
nop
ret
ExecuteNop ENDP

END
