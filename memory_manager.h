#ifndef MICRO32_MEMORY_MANAGER_H
#define MICRO32_MEMORY_MANAGER_H

// memory_manager.h
// Public API for a minimal memory manager used by the kernel to "reserve"
// RAM for the allocator/OS. This header declares a simple interface to:
//  - compute the RAM region available at runtime (using a linker symbol if present)
//  - reserve all memory except the first 8 KiB (so the first 8 KiB remain unreserved)
//  - query the reserved region
//
// Design decisions and fallback behavior:
//  - Preferred: If a linker symbol marking the top of RAM is provided (e.g. `__ram_end`),
//    the implementation will use that symbol to determine the RAM end at runtime.
//    You (or your linker script) can define such a symbol, e.g.:
//
//      PROVIDE(__ram_start = 0x3F800000);
//      PROVIDE(__ram_end   = 0x3F800000 + 32M);
//
//    The implementation expects a symbol named `__ram_end` (weakly referenced).
//
//  - Fallback: If the linker symbol is not available, the manager will use a compile-time
//    fallback base and size. By default this header provides:
//
//      DEFAULT_RAM_BASE = 0x3F800000  (PSRAM base fallback, change as needed)
//      DEFAULT_RAM_SIZE = 32 * 1024*1024 (32 MiB)
//
//  - Reservation semantics: "Reserve" here is logical: the manager records the reserved
//    region boundaries and optionally zeroes the memory. The header declares an API;
//    the corresponding .cpp implements the behaviour (zeroing is optional).
//
// Thread-safety / reentrancy: this is minimal boot-time code expected to run before
// concurrency is enabled; no locking is provided in the header-level API.
//
// NOTE: Implementation (memory_manager.cpp) must be provided and compiled into the
// final binary. This header only declares the interface.

#include <cstdint>
#include <cstddef>

namespace MemoryManager {

// Simple region descriptor (half-open interval: [start, end) )
struct Region {
    uintptr_t start; // inclusive
    uintptr_t end;   // exclusive
};

// Defaults you may override in your build or by calling set_ram_bounds
constexpr uintptr_t DEFAULT_RAM_BASE = 0x3F800000u;           // default PSRAM base (change if needed)
constexpr std::size_t DEFAULT_RAM_SIZE = 32u * 1024u * 1024u; // 32 MiB fallback

// The initial reserved prefix size (8 KiB)
constexpr std::size_t RESERVED_PREFIX = 8u * 1024u; // 8 KiB

// Configure RAM bounds explicitly at runtime before calling reserve_all_except_first_8kb.
// If you call this, it overrides the linker-symbol detection and the compile-time fallback.
void set_ram_bounds(uintptr_t ram_base, std::size_t ram_size);

// Compute and reserve all RAM except the first 8 KiB.
// Parameters:
//   zero_memory - if true, the function will zero the reserved memory region.
//                 Zeroing can be expensive for large RAM sizes; choose accordingly.
// Returns:
//   true on success, false on failure (e.g. size too small).
//
// Behavior:
//   - If a linker symbol (e.g. __ram_end) is available and used by the implementation,
//     the implementation will prefer that to determine the RAM top.
//   - Otherwise uses the RAM base/size set via set_ram_bounds or the compile-time defaults.
bool reserve_all_except_first_8kb(bool zero_memory = false);

// Query the last reserved region. Valid after reserve_all_except_first_8kb() returns true.
// If no reservation was done yet, returns {0,0}.
Region get_reserved_region();

// Convenience: get usable RAM region (the memory available for allocation).
// This returns the region that was reserved for use (the same as get_reserved_region()).
inline Region get_usable_region() {
    return get_reserved_region();
}

// Utility: helper to convert pointer/size to Region (for implementations/tests)
inline Region make_region(uintptr_t base, std::size_t size) {
    Region r;
    r.start = base;
    r.end = base + size;
    return r;
}

} // namespace MemoryManager

#endif // MICRO32_MEMORY_MANAGER_H