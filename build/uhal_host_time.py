from pathlib import Path
root = Path(__file__).resolve().parents[1]
p = root / 'UETD/src/uhal/platform/host/core.c'
s = p.read_text(encoding='utf-8')
s = s.replace('#include "uhal/core.h"', '#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)\n#define _POSIX_C_SOURCE 200809L\n#endif\n#include "uhal/core.h"')
s = s.replace('    return (uetd_u64_t)counter.QuadPart;', '''    const uetd_u64_t ticks = (uetd_u64_t)counter.QuadPart;
    const uetd_u64_t hz = (uetd_u64_t)frequency.QuadPart;
    /* Divide before multiplying the uptime: no truncated ticks/us scale. */
    return (ticks / hz) * 1000000u + ((ticks % hz) * 1000000u) / hz;''')
s = s.replace('timespec_get(&now, TIME_UTC) != TIME_UTC', 'clock_gettime(CLOCK_MONOTONIC, &now) != 0')
s = s.replace('(uetd_u64_t)now.tv_sec * 1000000000u + (uetd_u64_t)now.tv_nsec', '(uetd_u64_t)now.tv_sec * 1000000u + (uetd_u64_t)now.tv_nsec / 1000u')
begin = s.index('static uetd_u64_t host_frequency(void)')
end = s.index('uetd_u32_t uhal_core_id', begin)
s = s[:begin] + s[end:]
s = s.replace('uetd_u32_t uhal_cpu_freq_hz(void) { return uhal_cycles_per_us() * 1000000u; }', 'uetd_u32_t uhal_cpu_freq_hz(void) { return 0u; } /* Host CPU frequency is not simulated. */')
begin = s.index('uetd_u32_t uhal_cycles_per_us(void)')
end = s.index('/* Host barriers', begin)
s = s[:begin] + 'uetd_u32_t uhal_cycles_per_us(void) { return 1u; } /* Simulated counter ticks, not CPU cycles. */\n\n' + s[end:]
p.write_text(s, encoding='utf-8', newline='\n')
for name in ['clock', 'gpio', 'memory', 'timing']:
    src = root / 'UETD/src/uhal/arch/avr' / f'{name}.c.todo'
    dst = root / 'UETD/src/uhal/platform/avr_atmega' / src.name
    dst.parent.mkdir(parents=True, exist_ok=True)
    if src.exists(): src.rename(dst)
print('Corrected host monotonic time conversion and AVR architecture/platform boundary.')
