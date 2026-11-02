# USEC first-pass contracts

Typed draft interfaces and unsupported source definitions; these are not qualified runtime implementations.

## First-pass source scaffolds

All rows below are draft interfaces with source definitions returning
`UETD_NOT_SUPPORTED`. Generation remains blocked. Include them explicitly;
the established module umbrellas do not make these stubs usable.

| Header | Contract | Operations |
|---|---|---:|
| [provider.h](provider.h) | Shared entropy, key and crypto provider boundary | 5 |

## Header file structure

~~~text
usec/
├── provider.h
└── README.md
~~~
