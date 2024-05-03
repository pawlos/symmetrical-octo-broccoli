;extern FunEnterCallback:proc

_TEXT segment para 'CODE'
	align 16

	public FunEnterCallback

FunEnterCallback:
	; rcx - funId
	; rdx - clientData
	; r8  - frameInfo
	; r9  - argInfo	

	ret

;FunEnterCallback endp

_TEXT ends
end