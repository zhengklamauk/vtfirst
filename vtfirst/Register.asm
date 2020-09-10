.686p
.model flat, stdcall
option casemap:none

include Register.inc

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

_GetCr3 proc
    mov eax, cr3
    ret
_GetCr3 endp

_SetCr3 proc dw32Value_:dword
    mov eax, dw32Value_
    mov cr3, eax
    ret
_SetCr3 endp

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

_GetCs proc
    xor eax, eax
    mov ax, cs
    ret
_GetCs endp

_GetSs proc
    xor eax, eax
    mov ax, ss
    ret
_GetSs endp

_GetDs proc
    xor eax, eax
    mov ax, ds
    ret
_GetDs endp

_GetEs proc
    xor eax, eax
    mov ax, es
    ret
_GetEs endp

_GetFs proc
    xor eax, eax
    push fs
    pop eax
    ret
_GetFs endp

_GetGs proc
    xor eax, eax
    push gs
    pop eax
    ret
_GetGs endp

_GetTr proc
    xor eax, eax
    str ax
    ret
_GetTr endp

_GetTrBase proc
    xor eax, eax
    str ax
    
    movzx ecx, ax
    push ecx
    call _GetDescriptorBaseBySelector
    
    ret
_GetTrBase endp

;;从段选择子获取描述符中的Base
_GetDescriptorBaseBySelector proc dw32Selector_:dword
    xor eax, eax
    mov eax, dw32Selector_
    ;;bit 3 0-GDT 1-LDT
    mov ecx, eax
    and eax, 4
    cmp eax, 4
    jne __GDT
    call _GetLdt
    jmp __NEXT

__GDT:
    call _GetGdt

__NEXT:
    shr ecx, 3
    shl ecx, 3
    add eax, ecx
    mov ecx, eax
    push ecx
    call _GetDescriptorBaseByAddress
    ret
_GetDescriptorBaseBySelector endp

;;从段选择子获取描述符中的Limit
_GetDescriptorLimitBySelector proc dw32Selector_:dword
    xor eax, eax
    mov eax, dw32Selector_
    ;;bit 3 0-GDT 1-LDT
    mov ecx, eax
    and eax, 4
    cmp eax, 4
    jne __GDT
    call _GetLdt
    jmp __NEXT

__GDT:
    call _GetGdt

__NEXT:
    shr ecx, 3
    shl ecx, 3
    add eax, ecx
    mov ecx, eax
    push ecx
    call _GetDescriptorLimitByAddress
    ret
_GetDescriptorLimitBySelector endp

;;从描述符的地址获取描述符中的Base
_GetDescriptorBaseByAddress proc dw32Address_:dword
    mov eax, dw32Address_
    push esi
    push ecx
    xor esi, esi

    mov ecx, [eax+4]    ;;High 32 bit
    shr ecx, 23
    shl ecx, 23
    or esi, ecx
    mov ecx, [eax+4]
    and ecx, 0FFh
    shl ecx, 16
    or esi, ecx

    mov ecx, [eax]      ;;Low 32 bit
    shr ecx, 16
    or esi, ecx
    mov eax, esi
    pop ecx
    pop esi
    ret
_GetDescriptorBaseByAddress endp

;;从描述符的地址获取描述符中的Limit
_GetDescriptorLimitByAddress proc dw32Address_:dword
    mov eax, dw32Address_
    push esi
    push ecx
    xor esi, esi

    ;;G-800000h Limit-0F0000h
    mov ecx, [eax+4]    ;;High 32 bit
    and ecx, 0F0000h
    or esi, ecx
    mov ecx, [eax]
    and ecx, 0FFFFh
    or esi, ecx

    mov ecx, [eax+4]
    and ecx, 800000h
    cmp ecx, 0
    mov eax, esi
    je __NOTG
    shl eax, 12
    or eax, 0FFFh
    
__NOTG:
    pop ecx
    pop esi
    ret
_GetDescriptorLimitByAddress endp

_ReadMsr proc dw32Index_:dword
    mov ecx, dw32Index_
    rdmsr
    ret
_ReadMsr endp

_GetLdt proc
    xor eax, eax
    sldt ax

    push eax
    call _GetDescriptorBaseBySelector
    ret
_GetLdt endp

_GetGdt proc
    local @buff[6]:byte
    sgdt @buff
    push esi
    lea esi, @buff
    movzx edx, word ptr [esi]
    mov eax, dword ptr [esi+2]
    pop esi
    ret
_GetGdt endp

_SetGdt proc dw32Value_:dword
    local @buff[6]:byte
    push esi
    push edi
    mov esi, dw32Value_
    lea edi, @buff
    mov eax, dword ptr [esi+4]
    and eax, 0FFFFh
    mov word ptr [edi], ax
    mov eax, dword ptr [esi]
    mov dword ptr [edi+2], eax
    lgdt fword ptr @buff
    ret
_SetGdt endp

_GetIdt proc
    local @buff[6]:byte
    sidt @buff
    push esi
    lea esi, @buff
    movzx edx, word ptr [esi]
    mov eax, dword ptr [esi+2]
    pop esi
    ret
_GetIdt endp

_SetIdt proc dw32Value_:dword
    local @buff[6]:byte
    push esi
    push edi
    mov esi, dw32Value_
    lea edi, @buff
    mov eax, dword ptr [esi+4]
    and eax, 0FFFFh
    mov word ptr [edi], ax
    mov eax, dword ptr [esi]
    mov dword ptr [edi+2], eax
    lidt fword ptr @buff
    ret
_SetIdt endp

end