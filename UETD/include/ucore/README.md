# UCORE platform-independent foundations

UCORE provides low-level value types and utility contracts shared across UETD modules.

## Contents

- <code>types.h</code>, <code>constants.h</code>, and <code>limits.h</code> define shared types, canonical constants, and capacity bounds.
- <code>status.h</code> defines common result codes; <code>assert.h</code>, <code>bitops.h</code>, and <code>checksum.h</code> provide assertion, bit-operation, and checksum interfaces.
- <code>time.h</code> declares the shared time API whose implementation is selected by the hardware layer. UCORE has no single module umbrella; include the specific headers an application needs.

## Files by scope

Each row describes one header. The file link opens its source; the overview summarizes its purpose, and key declarations list the principal API symbols.

Type names may be defined in the linked header or supplied by an included dependency. Status guide: **Public contract** describes an API surface but does not guarantee every target has an implementation; **Future / planned** is a design or backlog header; **Fail-closed scaffold** declares development operations that are not implemented; **Target-family scaffold** is not a part or board profile; **Internal helper** is not an application-facing include; **Build configuration** supplies compile-time selection and capability macros.

| Scope | File | Overview | Key declarations and details | Status |
|---|---|---|---|---|
| module root | [assert.h](assert.h) | Report a failed assertion and halt through the platform fault path. | Functions: <code>uetd_assert_fail</code>; Defines: <code>UETD_ASSERT</code>, <code>UETD_ASSERT_MSG</code>, <code>UETD_STATIC_ASSERT</code> | Public contract |
| module root | [bitops.h](bitops.h) | Portable integer bit and byte-order helpers. | Functions: <code>uetd_byteswap16</code>, <code>uetd_byteswap32</code>, <code>uetd_log2_u32</code>, <code>uetd_next_pow2_u32</code>; Types used or declared: <code>uetd_u16_t</code>, <code>uetd_u32_t</code> | Public contract |
| module root | [checksum.h](checksum.h) | Incremental CRC functions with explicit initial register values. | Functions: <code>uetd_crc16_ccitt</code>, <code>uetd_crc32</code>, <code>uetd_crc8</code>; Types used or declared: <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code> | Public contract |
| module root | [constants.h](constants.h) | Canonical mathematical constants shared across UETD layers. | Defines: <code>UETD_EULER_E</code>, <code>UETD_HALF_PI</code>, <code>UETD_INV_SQRT2</code>, <code>UETD_LN10</code>, <code>UETD_LN2</code>, <code>UETD_PI</code>, <code>UETD_SQRT2</code>, <code>UETD_TAU</code> | Public contract |
| module root | [limits.h](limits.h) | Clamp a floating-point value to the inclusive bounds. | Functions: <code>uetd_absf</code>, <code>uetd_clampf</code>, <code>uetd_clampi</code>, <code>uetd_maxf</code>, <code>uetd_minf</code>, <code>uetd_signf</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_i32_t</code> | Public contract |
| module root | [status.h](status.h) | All UETD functions that can fail return uetd_status_t. | Functions: <code>uetd_status_str</code>; Types used or declared: <code>uetd_status_t</code> | Public contract |
| module root | [time.h](time.h) | Time types. UETD public time unit is microseconds throughout. | Functions: <code>uetd_delay_ms</code>, <code>uetd_delay_us</code>, <code>uetd_time_now_us</code>; Types used or declared: <code>uetd_dur_us_t</code>, <code>uetd_time_us_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>; Defines: <code>UETD_MS</code>, <code>UETD_S</code>, <code>UETD_US</code> | Public contract |
| module root | [types.h](types.h) | Foundational fixed-width and primitive aliases for portable UETD code. | Types used or declared: <code>uetd_f32_t</code>, <code>uetd_f64_t</code>, <code>uetd_i16_t</code>, <code>uetd_i32_t</code>, <code>uetd_i64_t</code>, <code>uetd_i8_t</code>, <code>uetd_q15_t</code>, <code>uetd_q16_t</code>, <code>uetd_q31_t</code>, <code>uetd_q63_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>; Defines: <code>UETD_ALIGNAS</code>, <code>UETD_NORETURN</code>, <code>UETD_WEAK</code> | Public contract |

## First-pass source scaffolds

All rows below are draft interfaces with source definitions returning
`UETD_NOT_SUPPORTED`. Generation remains blocked. Include them explicitly;
the established module umbrellas do not make these stubs usable.

| Header | Contract | Operations |
|---|---|---:|
| [span.h](span.h) | Checked byte spans and size arithmetic | 4 |
| [deadline.h](deadline.h) | Clock-domain stamps and checked finite deadlines | 4 |
| [codec.h](codec.h) | Bounded binary cursors and wire values | 4 |

## Header file structure

~~~text
ucore/
├── assert.h
├── bitops.h
├── checksum.h
├── codec.h
├── constants.h
├── deadline.h
├── limits.h
├── README.md
├── span.h
├── status.h
├── time.h
└── types.h
~~~
