	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
@feat.00 = 0
	.file	"lib.cpp"
	.def	main;
	.scl	2;
	.type	32;
	.endef
	.text
	.globl	main                            # -- Begin function main
	.p2align	4
main:                                   # @main
.seh_proc main
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	callq	__main
	xorl	%eax, %eax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	_GLOBAL__sub_I_lib.cpp;
	.scl	3;
	.type	32;
	.endef
	.p2align	4                               # -- Begin function _GLOBAL__sub_I_lib.cpp
_GLOBAL__sub_I_lib.cpp:                 # @_GLOBAL__sub_I_lib.cpp
.seh_proc _GLOBAL__sub_I_lib.cpp
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	movl	$32800, %eax                    # imm = 0x8020
	callq	___chkstk_ms
	subq	%rax, %rsp
	.seh_stackalloc 32800
	.seh_endprologue
	leaq	.L.str(%rip), %rcx
	leaq	.L.str.1(%rip), %rdx
	callq	fopen
	testq	%rax, %rax
	je	.LBB1_2
# %bb.1:
	leaq	32(%rsp), %rsi
	movl	$1, %edx
	movl	$32768, %r8d                    # imm = 0x8000
	movq	%rsi, %rcx
	movq	%rax, %r9
	callq	fread
	movq	%rsi, %rcx
	callq	strdup
	movq	%rax, %rcx
	callq	puts
.LBB1_2:
	movl	$0, coc_main(%rip)
	.seh_startepilogue
	addq	$32800, %rsp                    # imm = 0x8020
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.bss
	.globl	coc_main                        # @coc_main
	.p2align	2, 0x0
coc_main:
	.long	0                               # 0x0

	.section	.rdata,"dr"
.L.str:                                 # @.str
	.asciz	"linear.coc"

.L.str.1:                               # @.str.1
	.asciz	"rb"

	.section	.ctors,"dw",unique,0
	.p2align	3, 0x0
	.quad	_GLOBAL__sub_I_lib.cpp
	.section	.debug$S,"dr"
	.p2align	2, 0x0
	.long	4                               # Debug section magic
	.long	241
	.long	.Ltmp1-.Ltmp0                   # Subsection size
.Ltmp0:
	.short	.Ltmp3-.Ltmp2                   # Record length
.Ltmp2:
	.short	4353                            # Record kind: S_OBJNAME
	.long	0                               # Signature
	.byte	0                               # Object name
	.p2align	2, 0x0
.Ltmp3:
	.short	.Ltmp5-.Ltmp4                   # Record length
.Ltmp4:
	.short	4412                            # Record kind: S_COMPILE3
	.long	1                               # Flags and language
	.short	208                             # CPUType
	.short	21                              # Frontend version
	.short	1
	.short	8
	.short	0
	.short	21018                           # Backend version
	.short	0
	.short	0
	.short	0
	.asciz	"clang version 21.1.8 (https://github.com/msys2/MSYS2-packages 6774da8e8412d17da344f6b18223f20f4f9ba38f)" # Null-terminated compiler version string
	.p2align	2, 0x0
.Ltmp5:
.Ltmp1:
	.p2align	2, 0x0
	.addrsig
	.addrsig_sym _GLOBAL__sub_I_lib.cpp
