.686p
.model flat, stdcall
option casemap:none


.code

_GetCr0 proc
    mov eax, cr0
    ret
_GetCr0 endp

_SetCr0 proc dw32Value_:dword
    mov eax, dw32Value_
    mov cr0, eax
    ret
_SetCr0 endp

_GetCr4 proc
	mov eax, cr4
	ret
_GetCr4 endp

_SetCr4 proc dw32Value_:dword
	mov eax, dw32Value_
	mov cr4, eax
	ret
_SetCr4 endp

_GetEflags proc
	pushfd 
	pop eax
	ret
_GetEflags endp

_SetEflags proc dw32Value_:dword
	push dw32Value_
	popfd
	ret
_SetEflags endp

_ReadMsr proc dw32Index_:dword
    mov ecx, dw32Index_
    rdmsr
    ret
_ReadMsr endp

end