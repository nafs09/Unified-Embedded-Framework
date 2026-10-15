# UEF-PL unit reference

This page is generated from [`UETD/registry/units/registry.json`](../../UETD/registry/units/registry.json). The registry is authoritative; edit it and regenerate this page rather than editing the tables.

UEF-PL treats a unit as a compile-time dimension plus an exact scale and, for affine units, an offset. All dimensions below are independent dimensions in the UEF-PL type system.

## Dimension symbols

| Registry dimension | Symbol |
|---|---|
| `length` | \(\mathrm{L}\) |
| `mass` | \(\mathrm{M}\) |
| `time` | \(\mathrm{T}\) |
| `current` | \(\mathrm{I}\) |
| `temperature` | \(\Theta\) |
| `amount` | \(\mathrm{N}\) |
| `luminous_intensity` | \(\mathrm{J}\) |
| `angle` | \(\mathrm{A}_{\mathrm{angle}}\) |
| `count` | \(\mathrm{N}_{\mathrm{count}}\) |
| `information` | \(\mathrm{B}_{\mathrm{info}}\) |
| `symbol` | \(\mathrm{S}_{\mathrm{symbol}}\) |

## Registered units

The scale column gives the exact mapping from a value in that unit to its SI or canonical value. For affine units it shows the full transformation. Compound units use the registered symbols and combine their dimensions and scales; their dimensions need not be added to this table as new units.

### Length — \(\mathrm{L}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `cm` | \(\times 0.01\) |
| `km` | \(\times 1000\) |
| `m` | \(\times 1\) |
| `mm` | \(\times 0.001\) |

### Mass — \(\mathrm{M}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `g` | \(\times 0.001\) |
| `kg` | \(\times 1\) |

### Time — \(\mathrm{T}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `h` | \(\times 3600\) |
| `min` | \(\times 60\) |
| `ms` | \(\times 0.001\) |
| `ns` | \(\times 0.000000001\) |
| `s` | \(\times 1\) |
| `us` | \(\times 0.000001\) |

### Electric current — \(\mathrm{I}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `A` | \(\times 1\) |
| `mA` | \(\times 0.001\) |

### Temperature — \(\Theta\)

| Unit | Scale to SI or canonical value |
|---|---|
| `K` | \(\times 1\) |
| `degC` | \(x_{\mathrm{SI}} = x \cdot 1 + 273.15\) |
| `delta_degC` | \(\times 1\) |

### Amount of substance — \(\mathrm{N}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `mol` | \(\times 1\) |

### Luminous intensity — \(\mathrm{J}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `cd` | \(\times 1\) |

### Angle — \(\mathrm{A}_{\mathrm{angle}}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `deg` | \(\times \frac{1}{180}\,\pi\) |
| `rad` | \(\times 1\) |

### Count — \(\mathrm{N}_{\mathrm{count}}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `count` | \(\times 1\) |
| `words` | \(\times 1\) |

### Information — \(\mathrm{B}_{\mathrm{info}}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `B` | \(\times 8\) |
| `bit` | \(\times 1\) |

### Frequency — \(\mathrm{T}^{-1}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `Hz` | \(\times 1\) |
| `MHz` | \(\times 1000000\) |
| `kHz` | \(\times 1000\) |

### Angular rate — \(\mathrm{A}_{\mathrm{angle}}\,\mathrm{T}^{-1}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `rpm` | \(\times \frac{1}{30}\,\pi\) |

### Symbol rate — \(\mathrm{S}_{\mathrm{symbol}}\,\mathrm{T}^{-1}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `Mbaud` | \(\times 1000000\) |
| `baud` | \(\times 1\) |
| `kbaud` | \(\times 1000\) |

### Electric charge — \(\mathrm{I}\,\mathrm{T}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `C` | \(\times 1\) |
| `mAh` | \(\times 3.6\) |

### Force — \(\mathrm{L}\,\mathrm{M}\,\mathrm{T}^{-2}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `N` | \(\times 1\) |

### Energy and work — \(\mathrm{L}^{2}\,\mathrm{M}\,\mathrm{T}^{-2}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `J` | \(\times 1\) |

### Power — \(\mathrm{L}^{2}\,\mathrm{M}\,\mathrm{T}^{-3}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `W` | \(\times 1\) |

### Pressure — \(\mathrm{L}^{-1}\,\mathrm{M}\,\mathrm{T}^{-2}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `Pa` | \(\times 1\) |

### Electrical resistance — \(\mathrm{I}^{-2}\,\mathrm{L}^{2}\,\mathrm{M}\,\mathrm{T}^{-3}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `Ohm` | \(\times 1\) |

### Capacitance — \(\mathrm{I}^{2}\,\mathrm{L}^{-2}\,\mathrm{M}^{-1}\,\mathrm{T}^{4}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `F` | \(\times 1\) |

### Inductance — \(\mathrm{I}^{-2}\,\mathrm{L}^{2}\,\mathrm{M}\,\mathrm{T}^{-2}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `H` | \(\times 1\) |

### Magnetic flux — \(\mathrm{I}^{-1}\,\mathrm{L}^{2}\,\mathrm{M}\,\mathrm{T}^{-2}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `Wb` | \(\times 1\) |

### Magnetic flux density — \(\mathrm{I}^{-1}\,\mathrm{M}\,\mathrm{T}^{-2}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `T` | \(\times 1\) |
| `uT` | \(\times 0.000001\) |

### Electric potential — \(\mathrm{I}^{-1}\,\mathrm{L}^{2}\,\mathrm{M}\,\mathrm{T}^{-3}\)

| Unit | Scale to SI or canonical value |
|---|---|
| `V` | \(\times 1\) |
| `mV` | \(\times 0.001\) |

## Expressions and conversion

```uetd
signal distance : f32<m> = 120 mm;
signal speed : f32<m/s> = distance / 2 s;
signal angle : f32<rad> = convert_unit<rad>(90 deg);
```

Multiplication adds dimension exponents and multiplies scales; division subtracts exponents and divides scales. `^` is used for integer unit exponents, as in `m/s^2`; numeric powers use `**`. Implicit conversion is not allowed. Use `convert_unit<TARGET_UNIT>(value)` when units are compatible. Absolute temperature values such as `degC` cannot be multiplied or added to another absolute value; temperature differences use `delta_degC`.

The catalogue currently registers a finite set of named units. It does not imply that every physically dimension has a named unit; derived compound dimensions are formed in source from the registered base units.
