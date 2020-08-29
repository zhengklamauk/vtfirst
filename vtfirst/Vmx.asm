.686p
.model flat, stdcall
option casemap:none

.code

_vmxon proc dw32LowPart_:dword, dw32HighPart_:dword
	push dw32LowPart_
	push dw32HighPart_
	vmxon qword ptr [esp]
	add esp, 8
	ret
_vmxon endp

_vmxoff proc
	vmxoff
	ret
_vmxoff endp

end