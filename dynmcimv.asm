;　Windows 2000用 スタティックリンクの必須オブジェクトファイル　;
TITLE dynmcimv.asm
.686
.XMM
.MODEL FLAT

EXTERN _IslEncodePointer@4:DWORD
EXTERN _IslDecodePointer@4:DWORD

PUBLIC __imp__EncodePointer@4
PUBLIC __imp__DecodePointer@4

_data SEGMENT
__imp__EncodePointer@4	dd	_IslEncodePointer@4
__imp__DecodePointer@4	dd	_IslDecodePointer@4
_data ENDS

;ArgEcho PROC
;	or eax, 0FFFFFFFFh
;	xor eax, [esp+04h]
;	ret 04h
;ArgEcho ENDP

;__imp__EncodePointer@4 PROC
;	dd ArgEcho
;__imp__EncodePointer@4 ENDP

;__imp__DecodePointer@4 PROC
;	dd ArgEcho
;__imp__DecodePointer@4 ENDP
END
