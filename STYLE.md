# C style conventions

## Goals

- Small language surface area
- Readability > cleverness
- Predictability > portability edge-cases
- Prefer explicit lifetime + memory ownership
- Inspiration: “handmade” community style C

## Tooling and build

- Language level: C11 (`-std=c11`)
- Warnings:
  - keep builds clean (no warnings)
  - use `-Wall -Wextra -Wpedantic`
  - optionally add `-Werror` when you want max strictness
- Tooling:
  - `clang-format` enforces formatting
  - `clang-tidy` enforces naming and code-quality rules
  - CMake generates `compile_commands.json` for clangd

## Language profile

### Allowed

- C89 core + “nice C99” features:
  - `//` comments
  - declarations anywhere in a block
  - `<stdint.h>` fixed-width integers
  - `<stdbool.h>` booleans
  - designated initializers
  - compound literals
  - flexible array members
  - `inline`, `restrict` (use intentionally)
- Small C11 slice:
  - `_Static_assert`
  - anonymous structs/unions (when it improves API ergonomics)
  - `alignof` / `alignas` (only when needed)

### Avoided or banned

- VLAs (variable-length arrays)
- `alloca`
- C11 threads (`<threads.h>`) and C11 atomics in app code (prefer platform layer)
- `_Generic` and macro metaprogramming “type tricks”
- C23-only features (even if the compiler supports them)
- Exotic/rare standard library areas unless required:
  - `<complex.h>`, `<tgmath.h>`, `<fenv.h>`, `<setjmp.h>`, `<signal.h>`, `<locale.h>`

### Undefined behavior policy

- UB is a bug, always
- Forbidden patterns:
  - out-of-bounds access (incl. “works on my machine” pointer math)
  - signed integer overflow
  - strict-aliasing violations
  - use-after-free / use-after-scope
  - data races
- Prefer explicit “checked math” helpers in project code when relevant

## Formatting

- Base style: LLVM, with small tweaks
- Indentation: 4 spaces, no tabs
- Braces: K&R / attached (`if (...) {`)
- One statement per line; no single-line `if/for/while` bodies
- Line length: 100 characters
- Function calls and definitions do not “bin-pack”:
  - arguments and parameters split one-per-line if they don’t fit

## Naming

- functions/variables/params: `snake_case`
- typedef’d types/structs/enum types: `CamelCase`
- enum constants: `UPPER_CASE`
- macros: `UPPER_CASE`

## Types and declarations

- Pointers: star binds to the variable (`void *ptr`)
- Struct typedefs:
  - prefer `typedef struct { ... } Custom_Type;`
  - use tagged structs only for forward-decls
- Integer sizes:
  - prefer `uint8_t/uint32_t/size_t` etc. types when size matters
  - `int` etc. allowed for general in-memory code; be consistent within a module
- Prefer `const` correctness on API boundaries:
  - `const T *` for read-only inputs
  - return ownership rules documented per function/module

## Memory and ownership

- Heap allocation is allowed, but must be owned + freed by a clear owner
- Prefer arenas / regions for bulk lifetime management when practical
- Avoid hidden allocations in “utility” helpers (unless explicitly documented)
- Every allocation site has a matching free path (or is arena-scoped)

## Project structure conventions

- Keep a small “platform layer” boundary:
  - OS / threads / filesystem / timing live there
  - core logic stays platform-agnostic where feasible
- Keep dependencies minimal
- Prefer simple data layouts and explicit invariants
