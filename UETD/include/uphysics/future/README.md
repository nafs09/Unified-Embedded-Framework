# Future UPhysics constants

`constants.h` is a header-only outline for the SI physical-constant catalogue.
Its values are organized into two groups:

- The seven constants that define the SI, with SI units encoded in each macro
  name. Their numerical values are exact by definition; conversion to the
  configured UMATH scalar or accumulator can round them.
- Three useful derived quantities: the reduced Planck constant, molar gas
  constant, and Faraday constant. These are expressions over the defining
  constants and UMATH's canonical angular constants.

The values follow the BIPM SI defining-constants reference:
<https://www.bipm.org/en/measurement-units/si-defining-constants>.

Measured constants such as Newton's gravitational constant and the fine-
structure constant are deliberately excluded. They need a versioned CODATA
source, uncertainty metadata, and a declared rounding policy before they can
be governed here. The header is outside the stable include umbrella and the
module manifest blocks UEF generation until the unit-aware quantity binding
and unit-registry coverage are defined.
