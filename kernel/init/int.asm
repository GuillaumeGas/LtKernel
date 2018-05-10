[BITS 32]

;;; Processor exceptions & faults
extern divided_by_zero_isr
extern debug_isr
extern non_maskable_int_isr
extern breakpoint_isr	
extern overflow_isr
extern bound_range_exceeded_isr
extern invalid_opcode_isr
extern device_not_available_isr
extern double_fault_isr
extern invalid_tss_isr
extern segment_not_present_isr
extern stack_segment_fault_isr
extern general_protection_fault_isr
extern page_fault_isr
extern x87_floating_point_isr
extern alignment_check_isr
extern machine_check_isr
extern simd_floating_point_isr
extern virtualization_isr
extern security_isr
extern triple_fault_isr

;;; Hardware interruptions
extern default_isr
extern clock_isr
extern keyboard_isr
extern com1_isr	

;;; Processor exceptions & faults	
global _asm_divided_by_zero_isr
global _asm_debug_isr
global _asm_non_maskable_int_isr
global _asm_breakpoint_isr	
global _asm_overflow_isr
global _asm_bound_range_exceeded_isr
global _asm_invalid_opcode_isr
global _asm_device_not_available_isr
global _asm_double_fault_isr
global _asm_invalid_tss_isr
global _asm_segment_not_present_isr
global _asm_stack_segment_fault_isr
global _asm_general_protection_fault_isr
global _asm_page_fault_isr
global _asm_x87_floating_point_isr
global _asm_alignment_check_isr
global _asm_machine_check_isr
global _asm_simd_floating_point_isr
global _asm_virtualization_isr
global _asm_security_isr
global _asm_triple_fault_isr	

;;; Hardware interruptions
global _asm_default_isr
global _asm_clock_isr
global _asm_keyboard_isr
global _asm_com1_isr

%macro  SAVE_REGS 0
        pushad
        push ds
        push es
        push fs
        push gs
        push ebx
        mov bx,0x10
        mov ds,bx
        pop ebx
%endmacro

%macro  RESTORE_REGS 0
        pop gs
        pop fs
        pop es
        pop ds
        popad
%endmacro

%macro  EOI 0 		; EOI (End Of Interrupt)
	mov al, 0x20
	out 0x20, al
%endmacro

%macro  INT_PROLOG 0
	SAVE_REGS
%endmacro

%macro  INT_EPILOG 0
	EOI
	RESTORE_REGS
	iret
%endmacro
	
;;; Processor exceptions & faults
_asm_divided_by_zero_isr:
	INT_PROLOG
	call divided_by_zero_isr
	INT_EPILOG

_asm_debug_isr:
	INT_PROLOG
	call debug_isr
	INT_EPILOG

_asm_non_maskable_int_isr:
	INT_PROLOG
	call non_maskable_int_isr
	INT_EPILOG

_asm_breakpoint_isr:
	INT_PROLOG
	call breakpoint_isr
	INT_EPILOG

_asm_overflow_isr:
	INT_PROLOG
	call overflow_isr
	INT_EPILOG

_asm_bound_range_exceeded_isr:
	INT_PROLOG
	call bound_range_exceeded_isr
	INT_EPILOG

_asm_invalid_opcode_isr:
	INT_PROLOG
	call invalid_opcode_isr
	INT_EPILOG

_asm_device_not_available_isr:
	INT_PROLOG
	call device_not_available_isr
	INT_EPILOG

_asm_double_fault_isr:
	INT_PROLOG
	call double_fault_isr
	INT_EPILOG

_asm_invalid_tss_isr:
	INT_PROLOG
	call invalid_tss_isr
	INT_EPILOG

_asm_segment_not_present_isr:
	INT_PROLOG
	call segment_not_present_isr
	INT_EPILOG

_asm_stack_segment_fault_isr:
	INT_PROLOG
	call stack_segment_fault_isr
	INT_EPILOG

_asm_general_protection_fault_isr:
	INT_PROLOG
	call general_protection_fault_isr
	INT_EPILOG

_asm_page_fault_isr:
	INT_PROLOG
	call page_fault_isr
	INT_EPILOG

_asm_x87_floating_point_isr:
	INT_PROLOG
	call x87_floating_point_isr
	INT_EPILOG

_asm_alignment_check_isr:
	INT_PROLOG
	call alignment_check_isr
	INT_EPILOG

_asm_machine_check_isr:
	INT_PROLOG
	call machine_check_isr
	INT_EPILOG

_asm_simd_floating_point_isr:
	INT_PROLOG
	call simd_floating_point_isr
	INT_EPILOG

_asm_virtualization_isr:
	INT_PROLOG
	call virtualization_isr
	INT_EPILOG

_asm_security_isr:
	INT_PROLOG
	call security_isr
	INT_EPILOG

_asm_triple_fault_isr:
	INT_PROLOG
	call triple_fault_isr
	INT_EPILOG
	
_asm_default_isr:
	INT_PROLOG
	call default_isr
	INT_EPILOG

;;; Hardware interruptions
_asm_clock_isr:
	INT_PROLOG
	call clock_isr
	INT_EPILOG

_asm_keyboard_isr:
	INT_PROLOG
	call keyboard_isr
	INT_EPILOG

_asm_com1_isr:
	INT_PROLOG
	call com1_isr
	INT_EPILOG
	
