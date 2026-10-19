# Building the same executable

The sources build with Microsoft C 5.1, MASM 5.10 and LINK 5.01 under
Microsoft MAKE 4.07. The compiler options, segment names, declarations and
link order matter: changing apparently equivalent C can change the resulting
machine code or data layout.

Names are inferred unless preserved in the executable. They are not
claimed to be MicroProse's original names. Under this toolchain, identifier
spelling can affect local-variable allocation and the order of uninitialized
global storage. Some awkward names and repeated declarations remain to keep
the compiled layout unchanged.

## Compiler memory

The rules for `CONTROL.C` and `CITYMGT.C` use `loadfix -192` to limit the
memory available to the compiler. Available memory can affect the code
Microsoft C 5.1 emits for these large modules. The checked build includes
that condition; the original developers' machine configuration is unknown.

## Startup call

The build keeps raw linker output as `CIVRAW.EXE`. A small program built
from `CRTEDIT.C` then changes the destination of one far call, leaving
four bytes different in `CIV.EXE`.

| | Call destination | Function |
| --- | --- | --- |
| Raw LINK output | `2045:0442` | Runtime environment initialization, `__setenvp` |
| Final executable | `01a8:01c1` | `empty_hook`, which immediately returns |

The call opcode is at file offset `0x204c9`; its four destination bytes
occupy `0x204ca–0x204cd`. The editor checks length, CRC and expected bytes
before making the change, then checks the result. The reason the original
executable bypasses environment initialization is unknown.

`just verify` builds from scratch and checks SHA-256 hashes for both the
raw and final executables, plus the separate graphics driver, sound driver
and `MISC.EXE`. Matching those bytes establishes the build result. It does
not establish the original spelling of symbols or the historical build
procedure.
