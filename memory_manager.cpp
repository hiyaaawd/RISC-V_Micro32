/*
 * micro32/memory_manager.cpp
 *
 * Minimal memory manager implementation for Micro32.
 *
 * Behavior:
 *  - Prefer a linker-provided symbol `__ram_end` (weak). If present and non-null,
 *    this is used together with the DEFAULT_RAM_BASE to derive the RAM end.
 *    This avoids probing memory at runtime.
 *  - If the symbol is not present, uses the compile-time defaults:
 *      DEFAULT_RAM_BASE (0x3F800000) and DEFAULT_RAM_SIZE (32 MiB)
 *    or the values previously configured through set_ram_bounds().
 *
 * Reservation semantics:
 *  - reserve_all_except_first_8kb() computes a usable region starting at
 *    (ram_base + RESERVED_PREFIX) up to ram_base + ram_size, records it
 *    internally and optionally zeroes it.
 *
 * Notes:
 *  - The symbol `__ram_end` is declared weakly. If you provide a linker
 *    symbol with this name, it will be used. If you prefer a different
 *    linker symbol name, either modify this file or call set_ram_bounds()
 *    before calling reserve_all_except_first_8kb().
 */

#include "memory_manager.h"
#include <cstdint>
#include <cstddef>

namespace MemoryManager {

// Weak reference to a linker-provided symbol marking the end of RAM.
// If the symbol is not provided by the linker, the pointer will be nullptr.
extern "C" {
    extern char __ram_end[] __attribute__((weak));
}

// Internal storage for RAM bounds and the reserved region
static uintptr_t s_ram_base = DEFAULT_RAM_BASE;
static std::size_t s_ram_size = DEFAULT_RAM_SIZE;
static Region s_reserved_region = {0, 0};
static bool s_reserved = false;
static bool s_explicit_bounds_set = false;

// Helper: compute ram_end using linker symbol if available and meaningful.
static uintptr_t compute_ram_end_from_linker() {
    if (&__ram_end == nullptr) {
        // Weak symbol resolved to null pointer - not available.
        return 0;
    }

    // If the symbol exists, but the address equals zero, treat as not available.
    uintptr_t addr = reinterpret_cast<uintptr_t>(__ram_end);
    if (addr == 0) return 0;

    return addr;
}

void set_ram_bounds(uintptr_t ram_base, std::size_t ram_size) {
    // Save explicitly provided bounds and mark as explicitly set.
    s_ram_base = ram_base;
    s_ram_size = ram_size;
    s_explicit_bounds_set = true;
}

bool reserve_all_except_first_8kb(bool zero_memory) {
    // If reservation already happened, return true (idempotent).
    if (s_reserved) return true;

    uintptr_t ram_base = s_ram_base;
    std::size_t ram_size = s_ram_size;

    // If explicit bounds not set, try linker symbol first.
    if (!s_explicit_bounds_set) {
        uintptr_t linker_ram_end = compute_ram_end_from_linker();
        if (linker_ram_end != 0 && linker_ram_end > ram_base) {
            // Use DEFAULT_RAM_BASE as base unless user provided explicit base earlier.
            // If the user wants a different base, they should call set_ram_bounds().
            ram_size = linker_ram_end - ram_base;
        } else {
            // Keep the compile-time defaults already in s_ram_base / s_ram_size
            ram_base = s_ram_base;
            ram_size = s_ram_size;
        }
    }

    // Sanity checks
    if (ram_size <= RESERVED_PREFIX) {
        // Not enough memory to reserve after the first 8 KiB
        return false;
    }

    // Compute usable region [usable_start, usable_end)
    uintptr_t usable_start = ram_base + RESERVED_PREFIX;
    uintptr_t usable_end = ram_base + ram_size;

    // Ensure 32-bit word alignment for zeroing loop
    constexpr uintptr_t ALIGN = 4;
    if (usable_start % ALIGN) {
        usable_start = (usable_start + (ALIGN - 1)) & ~(ALIGN - 1);
    }

    if (usable_end % ALIGN) {
        usable_end &= ~(ALIGN - 1);
    }

    if (usable_start >= usable_end) {
        return false;
    }

    // Record the reserved/usable region
    s_reserved_region.start = usable_start;
    s_reserved_region.end = usable_end;

    // Optionally zero memory in the region. Use volatile stores to avoid
    // optimizations that might elide writes.
    if (zero_memory) {
        volatile uint32_t* p = reinterpret_cast<volatile uint32_t*>(usable_start);
        volatile uint32_t* p_end = reinterpret_cast<volatile uint32_t*>(usable_end);
        while (p < p_end) {
            *p++ = 0u;
        }
    }

    s_reserved = true;
    return true;
}

Region get_reserved_region() {
    return s_reserved_region;
}

} // namespace MemoryManager