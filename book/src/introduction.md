# Reading the source

This book describes the MS-DOS version of *Sid Meier's Civilization*,
version 474.05. It follows the C and assembly in `src/`, from DOS drivers
and map storage to city simulation, unit movement and diplomacy.

The code is reverse engineered. Names are inferred, and some modules still
use local record declarations or casts that differ from the shared headers.
Descriptions here follow the operations and stored values, including quirks
that may differ from a player's expectations. The [build notes](architecture/toolchain_clues.md)
explain what the executable comparison establishes.

For a guided reading of a few complete files, see the Docco story in
`story/`. This book groups explanations by game system and ends with source
tables and shared declarations.

## Entry point

`main()` in `GAME.C` starts `MISC.EXE`, runs setup, installs break handlers
and calls `game_startup()`. It checks available memory, loads cursor images,
starts the resident timer routines and initializes the mouse.

After `game_menu()` and `start_game()`, it calls `game_turn()` until `leave`
is set. On exit it closes the mouse, stops sound, restores timer and break
handlers, and requests BIOS text mode 3.

```c
{{#include ../../src/GAME.C:490:543}}
```

`GAME.C` also defines the terrain, technology, improvement, unit and tribe
tables. The [memory chapter](architecture/memory_model.md) explains how
shared state remains available while screens and other operations use overlays.
