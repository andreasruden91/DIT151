@ Mom1.asm

start:	@ Mark D as output
	LDR	R0, =0x55555555
	LDR	R1, =0x40020C00 @ Port D MODE Register
	STR	R0, [R1]
	
	LDR	R1, =0x40020C14 @ Port D's output register
	LDR	R2, =0x40021010 @ Port E's input register

main:	@ Loop: Read from input (E) store to output (D)
	LDR	R0, [R2]
	STR	R0, [R1]
	B	main
