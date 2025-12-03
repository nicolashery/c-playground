# C Playground

Repo with multiple small projects as I play around with C and learn low-level programming and manual memory management.

## Projects

- `bean/`: Small CLI tool that parses a subset of [Beancount](https://github.com/beancount/beancount) plain-text accounting
- `scratch/`: Scratch project to try out various things that don't warrant their own project
- `dodge/`: Dodge the Blocks game using Raylib

The `libs/` directory contains any shared libraries between projects:
- `libs/math_utils/`: Example shared library, doesn't really do anything useful

## Dependencies

- `clang`
- `clangd`
- `clang-format`
- `clang-tidy`
- `cmake`
- [`task`](https://taskfile.dev/)

On macOS:

```bash
xcode-select --install # libc

brew update
brew install llvm # clang, clangd, lldb, clang-format, clang-tidy
brew install cmake # to generate compile_commands.json for clangd
brew install go-task/tap/go-task # to run tasks in Taskfile.yaml

# use newer version of toolchain:
echo 'export PATH="$(brew --prefix)/opt/llvm/bin:$PATH"' >> ~/.zshrc
# for cmake to use newer version of compiler
echo 'export CC=clang' >> ~/.zshrc
echo 'export CXX=clang++' >> ~/.zshrc
```

## Development

Run CMake configuration step (and generate `compile_commands.json` for `clangd`):

```bash
task configure
```

Build all projects:

```bash
task build
```

Format using `clang-format`:

```bash
task format
```

Lint using `clang-tidy`:

```bash
task lint
```

Run tests:

```bash
task test
```

For all available commands see [`Taskfile.yaml`](`Taskfile.yaml`) or run `task` in the terminal.

## Conventions

See [STYLE.md](STYLE.md) for C style conventions used in this repo.
