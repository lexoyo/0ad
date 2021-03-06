; Copyright (c) 2010 Wildfire Games
;
; Permission is hereby granted, free of charge, to any person obtaining
; a copy of this software and associated documentation files (the
; "Software"), to deal in the Software without restriction, including
; without limitation the rights to use, copy, modify, merge, publish,
; distribute, sublicense, and/or sell copies of the Software, and to
; permit persons to whom the Software is furnished to do so, subject to
; the following conditions:
; 
; The above copyright notice and this permission notice shall be included
; in all copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
; IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
; CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
; TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
; SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

; For the sym() macro
%include "../ia32/ia32.inc"
; For arg0..3
%include "amd64_abi.inc"

BITS 64

; extern "C" void CALL_CONV amd64_asm_cpuid(x86_x64_CpuidRegs* reg);
; reference: http://softwarecommunity.intel.com/articles/eng/2669.htm
global sym(amd64_asm_cpuid)
	ALIGN 8
sym(amd64_asm_cpuid):
	push		rbx			; rbx is the only caller-save register we clobber

	mov			r8, arg0
	mov			eax, DWORD [r8+0]
	mov			ecx, DWORD [r8+8]
	cpuid
	mov			DWORD [r8+0], eax
	mov			DWORD [r8+4], ebx
	mov			DWORD [r8+8], ecx
	mov			DWORD [r8+12], edx

	pop			rbx

	ret
	ALIGN 8
