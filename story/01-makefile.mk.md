# 1. Building the executable

Microsoft MAKE 4.07 builds the project with Microsoft C 5.1, MASM 5.10
and LINK 5.01.

{{#include src/MAKEFILE}}

## Compilation rules

`CFLAGS` supplies options shared by the C compilation commands. Each rule then
adds its own memory-model, optimisation and code-segment options. In the
`GAME.C` command, `/AM` selects the medium memory model and `/NTF01A8` names
the code segment.

The dependency lists show how widely a module uses declarations from other
files. The commands are similar, but they are not interchangeable. `CORE.C`,
for example, uses `/Os` where `GAME.C` uses `/Oas`. These choices form part of
the build that reproduces the executable.

{{#include src/MAKEFILE | "CFLAGS=/Zl /c /W3 /Fc"}}

## Compiler memory conditions

The compiler rules for `CONTROL.C` and `CITYMGT.C` use `loadfix -192` to
limit available memory. Microsoft C 5.1 emits different code for these
large modules when memory is scarce. This build condition reproduces the
executable; the original machine configuration is unknown.

The following rules also distinguish C modules from assembly modules. MASM
assembles the resident routines and graphics code. Later C commands use
segment names beginning with `O` for the overlays. An overlay occupies a
shared loading area when needed, allowing code that is not needed together to
use the same memory.

{{#include src/MAKEFILE | "#\tReconstructed low-memory MSC condition for CONTROL and CITYMGT;"}}

## Linking and the startup correction

The game link depends on the object files and `LINK.RSP`. LINK reads that
response file and writes `CIV.EXE`.

`CRTEDIT.EXE` changes the four destination bytes of one startup call,
redirecting runtime environment initialization to an empty game function.
It checks the input and result. The original executable makes this same
call, but how its developers arranged it is unknown.

{{#include src/MAKEFILE | "#\tThe game. LINK names its output CIV.EXE; the unmodified"}}

## Programs loaded separately

The final rules build the graphics driver, sound driver and `MISC.EXE` as
separate programs. These objects do not join the game link above. Each has its
own LINK command, and the driver commands do not supply C runtime libraries.

The file therefore describes more than one executable. It also records the
boundary between the game and programs it loads at run time. That boundary
remains visible when we reach `GAME.C`.

{{#include src/MAKEFILE | "#\tDrivers CIV loads at run time (separate programs, built the"}}

[← Previous](00-opening.c.html) · [Contents](00-opening.c.html) · [Next →](02-records.c.html)
