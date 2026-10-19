# 3. The main module

Complete file:
[`src/GAME.C`](https://github.com/sjmelia/civd/blob/main/src/GAME.C).

`GAME.C` contains the entry point, but most of its first half is data.
Terrain, technology, improvement, unit and civilisation tables precede `main`.
Screen and input helpers follow it. The final declarations reserve far-memory
arrays for shared state.

The local record declarations do not all use the descriptive field names
from `CIV.H`. Their fields occupy the same positions.

{{#include src/GAME.C}}

## Rule tables

The terrain table contains twelve base terrain types followed by twelve
entries for their resource variants. Their names make the correspondence
visible: Desert is followed twelve entries later by Oasis, and Ocean by Fish.

The subsequent tables hold work rules, technology prerequisites, improvement
properties and related values. They are initialised data in this module. A
reader can inspect an entire table here and then follow the code that indexes
it elsewhere.

{{#include src/GAME.C | "/* 0-11 the terrain types, 12-23 the same with their special resource. */"}}

## Units and civilisations

`utypes` supplies the values shared by each unit type. Its local declaration
still calls several fields `w2` through `w10`. `CIV.H` gives those positions
more descriptive names. Both declarations need to preserve the compiled
representation.

The tables that follow include civilisation and leader data. Keeping these
tables beside the entry point shows the responsibilities assigned to this
translation unit; the filename alone does not describe all of them.

{{#include src/GAME.C | "/* Unit types and civilization/leader tables. */"}}

## Starting and stopping the game

`main` runs `misc.exe`, calls setup, changes a BIOS data-area byte, and
installs break handlers before entering game startup. The far address
`0x00400017L` identifies the keyboard-status byte; clearing `0x20` clears its
Num Lock state bit.

The memory check measures free space in sixteen-byte DOS paragraphs. The
thresholds 8,000 and 4,000 correspond to 128,000 and 64,000 bytes. This branch
displays a warning. It does not immediately return from `main`.

After cursor and mouse setup, the program calls `game_menu` and `start_game`,
then repeatedly calls `game_turn` until `leave` changes. On exit it closes the
mouse, stops sound, restores the timer and break handlers, and requests BIOS
video mode 3.

{{#include src/GAME.C | "int main(void)"}}

## Screen, mouse and disk helpers

The helpers following `main` manage screen pages and mouse visibility.
`mouse_on` and `mouse_off` update a counter and show or hide the pointer when
that counter crosses the relevant boundary. Their effect therefore depends on
previous calls.

`disk_insert` checks for a representative file and prompts for a disk when it
cannot find it. The surrounding filename table identifies the assets used for
these checks. The break-handler routines retain the old interrupt vectors and
restore them later. `sound` passes commands onwards only when bit `0x10` of
`debug` is set.

{{#include src/GAME.C | "void empty_hook(void)"}}

## Choosing a game

`game_menu` handles a new game, a saved game, Earth, a customised world and
the Hall of Fame. The customisation branch fills `custom_params` through menus
for land mass, temperature, climate and age, then enters the same generation
path as a normal new game.

The Earth branch sets `beginner_help` before calling `make_world`. In the next
file, that flag selects the supplied map instead of generated land. The
saved-game branch calls `load_game` and returns to the menu if loading is
cancelled.

`start_game` checks `turn_tick` before calling `new_game`. This lets the
startup path continue with state loaded from a save as well as state prepared
for a new game.

{{#include src/GAME.C | "void game_menu(void)"}}

## Far-memory state

The closing arrays include visibility, replay data, technology ownership and
histories. The `far` qualifier allows access outside the shared data segment.

The comment records a linker constraint: these uninitialised communal arrays
are allocated in a name-dependent order. Renaming one can change the linked
layout even when its C type stays the same. A complete build comparison is needed after changing such identifiers.
This helps explain some of the unusual names.

{{#include src/GAME.C | "/* Far world state. These uninitialized far arrays are communals, which LINK"}}

[← Previous](02-records.c.html) · [Contents](00-opening.c.html) · [Next →](04-world.c.html)
