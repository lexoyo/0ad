; =========================================================================
; File        : ia32_asm.asm
; Project     : 0 A.D.
; Description : optimized assembly code for IA-32. not provided as
;             : inline assembly because that's compiler-specific.
; =========================================================================

; license: GPL; see lib/license.txt

%include "ia32.inc"

; note: pure asm functions prevent inlining but also avoid redundant
; store/loads generated by VC inline asm (ugh).


;-------------------------------------------------------------------------------
; CPUID support
;-------------------------------------------------------------------------------

; extern "C" void __cdecl ia32_asm_cpuid(x86_x64_CpuidRegs* regs);
global sym(ia32_asm_cpuid)
sym(ia32_asm_cpuid):
	push	ebx							; (clobbered by CPUID)
	push	edi							; (need a register other than eax..edx)

	mov		edi, [esp+8+4]				; -> regs

	mov		eax, [edi+0]				; eax (function)
	mov		ecx, [edi+8]				; ecx (count)
	cpuid
	stosd
	xchg		eax, ebx
	stosd
	xchg		eax, ecx
	stosd
	xchg		eax, edx
	stosd

	pop		edi
	pop		ebx
	ret


;-------------------------------------------------------------------------------
; lock-free support routines
;-------------------------------------------------------------------------------

; extern "C" void __cdecl ia32_asm_AtomicAdd(volatile intptr_t* location, intptr_t increment);
global sym(ia32_asm_AtomicAdd)
sym(ia32_asm_AtomicAdd):
	mov		edx, [esp+4]				; location
	mov		eax, [esp+8]				; increment
db		0xf0							; LOCK prefix
	add		[edx], eax
	ret


; notes:
; - a 486 or later processor is required since we use CMPXCHG.
;   there's no feature flag we can check, and the ia32 code doesn't
;   bother detecting anything < Pentium, so this'll crash and burn if
;   run on 386. we could fall back to simple MOVs there (since 386 CPUs
;   aren't MP-capable), but it's not worth the trouble.
; - nor do we bother skipping the LOCK prefix on single-processor systems.
;   the branch may be well-predicted, but difference in performance still
;   isn't expected to be enough to justify the effort.
; extern "C" bool __cdecl ia32_asm_CAS(volatile uintptr_t* location, uintptr_t expected, uintptr_t new_value);
global sym(ia32_asm_CAS)
sym(ia32_asm_CAS):
	mov		edx, [esp+4]				; location
	mov		eax, [esp+8]				; expected
	mov		ecx, [esp+12]				; new_value
db		0xf0							; LOCK prefix
	cmpxchg	[edx], ecx
	sete	al
	movzx	eax, al
	ret


;-------------------------------------------------------------------------------
; FPU
;-------------------------------------------------------------------------------

[section .data]

; to conform with the fallback implementation (a C cast), we need to
; end up with truncate/"chop" rounding. subtracting does the trick,
; assuming RC is the IA-32 default round-to-nearest mode.
round_bias		dd 0.4999999

__SECT__

; extern "C" u32 __cdecl ia32_asm_control87(u32 new_cw, u32 mask);
global sym(ia32_asm_control87)
sym(ia32_asm_control87):
	push	eax
	fnstcw	[esp]
	pop		eax							; old_cw
	mov		ecx, [esp+4]				; new_val
	mov		edx, [esp+8]				; mask
	and		ecx, edx					; new_val & mask
	not		edx							; ~mask
	and		eax, edx					; old_cw & ~mask
	or		eax, ecx					; (old_cw & ~mask) | (new_val & mask)
	push	eax							; = new_cw
	fldcw	[esp]
	pop		eax
	xor		eax, eax					; return value
	ret


; possible IA-32 FPU control word flags after FXAM: NAN|NORMAL|ZERO
FP_CLASSIFY_MASK	equ 0x4500

; extern "C" size_t __cdecl ia32_asm_fpclassifyd(double d);
global sym(ia32_asm_fpclassifyd)
sym(ia32_asm_fpclassifyd):
	fld		qword [esp+4]
	fxam
	fnstsw	ax
	fstp	st0
	and		eax, FP_CLASSIFY_MASK
	ret

; extern "C" size_t __cdecl ia32_asm_fpclassifyf(float f);
global sym(ia32_asm_fpclassifyf)
sym(ia32_asm_fpclassifyf):
	fld		dword [esp+4]
	fxam
	fnstsw	ax
	fstp	st0
	and		eax, FP_CLASSIFY_MASK
	ret


; extern "C" float __cdecl ia32_asm_rintf(float);
global sym(ia32_asm_rintf)
sym(ia32_asm_rintf):
	fld		dword [esp+4]
	frndint
	ret

; extern "C" double __cdecl ia32_asm_rint(double);
global sym(ia32_asm_rint)
sym(ia32_asm_rint):
	fld		qword [esp+4]
	frndint
	ret


; extern "C" float __cdecl ia32_asm_fminf(float, float);
global sym(ia32_asm_fminf)
sym(ia32_asm_fminf):
	fld		dword [esp+4]
	fld		dword [esp+8]
	fcomi	st0, st1
	fcmovnb	st0, st1
	fxch
	fstp	st0
	ret

; extern "C" float __cdecl ia32_asm_fmaxf(float, float);
global sym(ia32_asm_fmaxf)
sym(ia32_asm_fmaxf):
	fld		dword [esp+4]
	fld		dword [esp+8]
	fcomi	st0, st1
	fcmovb	st0, st1
	fxch
	fstp	st0
	ret


; extern "C" i32 __cdecl ia32_asm_i32FromFloat(float f);
global sym(ia32_asm_i32FromFloat)
sym(ia32_asm_i32FromFloat):
	push		eax
	fld			dword [esp+8]
	fsub		dword [round_bias]
	fistp		dword [esp]
	pop			eax
	ret

; extern "C" i32 __cdecl ia32_asm_i32FromDouble(double d);
global sym(ia32_asm_i32FromDouble)
sym(ia32_asm_i32FromDouble):
	push		eax
	fld			qword [esp+8]
	fsub		dword [round_bias]
	fistp		dword [esp]
	pop			eax
	ret

; extern "C" i64 __cdecl ia32_asm_i64FromDouble(double d);
global sym(ia32_asm_i64FromDouble)
sym(ia32_asm_i64FromDouble):
	push		edx
	push		eax
	fld			qword [esp+12]
	fsub		dword [round_bias]
	fistp		qword [esp]
	pop			eax
	pop			edx
	ret


;-------------------------------------------------------------------------------
; misc
;-------------------------------------------------------------------------------

; write the current execution state (e.g. all register values) into
; (Win32::CONTEXT*)pcontext (defined as void* to avoid dependency).
; optimized for size; this must be straight asm because ; extern "C"
; is compiler-specific and compiler-generated prolog code inserted before
; inline asm trashes EBP and ESP (unacceptable).
; extern "C" void ia32_asm_GetCurrentContext(void* pcontext);
global sym(ia32_asm_GetCurrentContext)
sym(ia32_asm_GetCurrentContext):
	pushad
	pushfd
	mov		edi, [esp+4+32+4]	; pcontext

	; ContextFlags
	mov		eax, 0x10007		; segs, int, control
	stosd

	; DRx and FloatSave
	; rationale: we can't access the debug registers from Ring3, and
	; the FPU save area is irrelevant, so zero them.
	xor		eax, eax
	push	byte 6+8+20
	pop		ecx
rep	stosd

	; CONTEXT_SEGMENTS
	mov		ax, gs
	stosd
	mov		ax, fs
	stosd
	mov		ax, es
	stosd
	mov		ax, ds
	stosd

	; CONTEXT_INTEGER
	mov		eax, [esp+4+32-32]	; edi
	stosd
	xchg	eax, esi
	stosd
	xchg	eax, ebx
	stosd
	xchg	eax, edx
	stosd
	mov		eax, [esp+4+32-8]	; ecx
	stosd
	mov		eax, [esp+4+32-4]	; eax
	stosd

	; CONTEXT_CONTROL
	xchg	eax, ebp			; ebp restored by POPAD
	stosd
	mov		eax, [esp+4+32]		; return address
	sub		eax, 5				; skip CALL instruction -> call site.
	stosd
	xor		eax, eax
	mov		ax, cs
	stosd
	pop		eax					; eflags
	stosd
	lea		eax, [esp+32+4+4]	; esp
	stosd
	xor		eax, eax
	mov		ax, ss
	stosd

	; ExtendedRegisters
	xor		ecx, ecx
	mov		cl, 512/4
rep	stosd

	popad
	ret