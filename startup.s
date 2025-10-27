
.equ STACK_START, 0x20002000 // 8KB of RAM for a minimal stack
.equ HEAP_START, 0x20001000  // Start of the conceptual memory area to clear
.equ HEAP_END, 0x20001800    // End of the conceptual memory area to clear (2KB chunk cleared)

// RISC-V-specific constants
.equ BSS_START, 0x20000800   // Start of BSS section
.equ BSS_END, 0x20001000     // End of BSS section

// BSS section boundaries (for zero-initialization)
.equ BSS_START, 0x20000800   // Start of BSS section
.equ BSS_END, 0x20001000     // End of BSS section

// Exit codes for system shutdown dispatcher
.equ EXIT_FULL_SHUTDOWN, 6046
.equ EXIT_LOW_POWER_SLEEP, 6045
.equ EXIT_PANIC_REBOOT, 0001

.section .vectors
.word STACK_START        // 1. Initial Stack Pointer
.word Reset_Handler + 1  // 2. Initial Program Counter (Jump to Reset_Handler)

.section .text
.global Reset_Handler
Reset_Handler:
    // 1. Initialize stack pointer
    la sp, STACK_START       // Load stack pointer with STACK_START

    // 2. Initialize the BSS section (Zero-fill uninitialized variables)
    li t0, 0                 // Value to write (zero)
    la t1, BSS_START         // Load address of BSS_START into t1
    la t2, BSS_END           // Load address of BSS_END into t2

.bss_init_loop:
    bge t1, t2, .bss_init_done // If t1 >= t2, we are done
    sw t0, 0(t1)              // Store 0 into memory location pointed by t1
    addi t1, t1, 4            // Increment address by 4 bytes
    j .bss_init_loop          // Jump back to loop

.bss_init_done:
    // 3. Jump to the C entry point (main)
    jal main                  // Jump and link to the 'main' function

// --- System Exit Dispatcher ---
// This is reached ONLY if main() returns or system_exit() forces a jump here.
End_Loop:
    cmp r8, EXIT_FULL_SHUTDOWN    // Check for FULL SHUTDOWN code (6046)
    beq .full_shutdown

    cmp r8, EXIT_LOW_POWER_SLEEP    // Check for LOW-POWER SLEEP code (6045)
    beq .low_power_sleep

    cmp r8, EXIT_PANIC_REBOOT
    beq .clear_states // Jumps here if r8 == 1 (The Panic/Clean Reboot code)

    // Default action if r8 is not 6045, 6046, or 0001: REBOOT
    B Reset_Handler

// --- Special Reset Routine (Kernel Panic/Clean Reboot) ---
.clear_states:
    // Step 1: Clear General Purpose Registers x1 through x31
    li t0, 0                 // Load 0 into temporary register t0
    mv ra, t0                // Clear return address register
    mv sp, t0                // Clear stack pointer
    mv gp, t0                // Clear global pointer
    mv tp, t0                // Clear thread pointer
    mv t1, t0
    mv t2, t0
    mv t3, t0
    mv t4, t0
    mv t5, t0
    mv t6, t0
    mv s0, t0
    mv s1, t0
    mv s2, t0
    mv s3, t0
    mv s4, t0
    mv s5, t0
    mv s6, t0
    mv s7, t0
    mv s8, t0
    mv s9, t0
    mv s10, t0
    mv s11, t0
    mv a0, t0
    mv a1, t0
    mv a2, t0
    mv a3, t0
    mv a4, t0
    mv a5, t0
    mv a6, t0
    mv a7, t0

    // Step 2: Clear Heap Memory Area
    // t0 is already 0 (the value to write)
    la t1, HEAP_START        // Load starting address into t1
    la t2, HEAP_END          // Load end address into t2

.clear_heap_loop:
    bge t1, t2, .done_clearing // If t1 >= t2, we are done
    sw t0, 0(t1)              // Store 0 (t0) into the memory location pointed to by t1
    addi t1, t1, 4            // Increment address by 4 (to clear one 32-bit word)
    j .clear_heap_loop        // Loop back

.done_clearing:
    // Step 3: Force a complete system reboot with clean state.
    j Reset_Handler

// --- Halt Routines ---

.low_power_sleep:
    // CPU sleeps but leaves interrupts ENABLED, allowing peripherals to wake it.
    wfi             // Wait For Interrupt (CPU goes into low power)
    j .low_power_sleep // Jump to re-enter sleep if woken

.full_shutdown:
    // CPU shuts down and requires a physical reset to wake.
    csrci mstatus, 0x8 // Disable ALL maskable interrupts
    wfi                // Wait For Interrupt (CPU goes into deep sleep)
    j .full_shutdown   // Jump to re-enter shutdown if woken
