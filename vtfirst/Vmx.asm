.686p
.model flat, stdcall
option casemap:none

include Vmx.inc

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

_cpuid proc dw32Value_:dword, pstResult_:dword
    mov eax, dw32Value_
    cpuid

    mov esi, pstResult_
    assume esi:ptr CPUIDRESULT
    mov [esi].eax_, eax
    mov [esi].ebx_, ebx
    mov [esi].ecx_, ecx
    mov [esi].edx_, edx
    assume esi:nothing

    ret
_cpuid endp

end