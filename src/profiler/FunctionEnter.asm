extern FuncEnterStub:proc
extern FuncLeaveStub:proc

_TEXT segment para 'CODE'
	align 16

	public Func EnterCallback	

FuncEnterCallback proc frame
	; rcx - funId
	; rdx - clientData
	; r8  - frameInfo
	; r9  - argInfo	
	
	push rax
	.allocstack 8

	sub rsp, 20h
	.allocstack 20h
	.endprolog

	call FuncEnterStub

	add rsp, 20h

	; restore
	pop rax

	ret

FuncEnterCallback endp

public FuncLeaveCallback
FuncLeaveCallback proc frame
	; rcx - funId
	; rdx - clientData
	; r8  - frameInfo
	; r9  - argInfo	
	
	push rax
	.allocstack 8

	sub rsp, 20h
	.allocstack 20h
	.endprolog

	call FuncLeaveStub

	add rsp, 20h

	; restore
	pop rax

	ret

FuncLeaveCallback endp
_TEXT ends
end