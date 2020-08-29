.686p
.model flat, stdcall
option casemap:none


.code

GetCr4 proc
	mov eax, cr4
	ret
GetCr4 endp

SetCr4 proc dw32Value_:dword
	mov eax, dw32Value_
	mov cr4, eax
	ret
SetCr4 endp

GetEflags proc
	pushfd 
	pop eax
	ret
GetEflags endp

SetEflags proc dw32Value_:dword
	push dw32Value_
	popfd
	ret
SetEflags endp

end