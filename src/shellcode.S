.text
.syntax	unified

.pool 



.set prepare_and_jump, 0x4FF19265

.set JUMPADDR, 0x41002750

.global _prepare_and_jump
    

_prepare_and_jump:
	MOVS R0, #0x0
    LDR R1, =JUMPADDR
    MOV R2, R0
    LDR R4, =prepare_and_jump
    BX R4       