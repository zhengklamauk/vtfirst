.686p
.model flat, stdcall
option casemap:none

include Vmx.inc

.code

_vmxon proc dw32LowPart_:dword, dw32HighPart_:dword
	push dw32HighPart_
	push dw32LowPart_
	vmxon qword ptr [esp]
	add esp, 8
	ret
_vmxon endp

_vmxoff proc
	vmxoff
	ret
_vmxoff endp

_cpuid proc dw32Value_:dword, pstResult_:dword
	pushad
    mov eax, dw32Value_
    cpuid

    mov esi, pstResult_
    assume esi:ptr CPUIDRESULT
    mov [esi].eax_, eax
    mov [esi].ebx_, ebx
    mov [esi].ecx_, ecx
    mov [esi].edx_, edx
    assume esi:nothing
	popad

    ret
_cpuid endp

_vmwrite proc dw32Field_:dword, dw32Value_:dword
    mov eax, dw32Field_
    mov ecx, dw32Value_
    vmwrite eax, ecx
    ret
_vmwrite endp

_vmread proc dw32Field_:dword
    mov eax, dw32Field_
    vmread ecx, eax
    mov eax, ecx
    ret
_vmread endp

_vmclear proc dw32LowPart_:dword, dw32HighPart_:dword
	push dw32HighPart_
	push dw32LowPart_
	vmclear qword ptr [esp]
	add esp, 8
	ret
_vmclear endp

_vmptrld proc dw32LowPart_:dword, dw32HighPart_:dword
	push dw32HighPart_
	push dw32LowPart_
	vmptrld qword ptr [esp]
	add esp, 8
	ret
_vmptrld endp

_vmlaunch proc
    vmlaunch
    ret
_vmlaunch endp

_vmcall proc
    vmcall
    ret
_vmcall endp

end