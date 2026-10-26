# 8. Saving and loading state

Complete file:
[`src/SAVE.C`](https://github.com/sjmelia/civd/blob/main/src/SAVE.C).

`SAVE.C` begins with drive and slot selection, then implements file transfers.
Its declarations refer to the same city, unit-type and unit arrays used in
combat. The file therefore lets us compare their in-memory representation with
the lengths transferred to disk.

The code belongs to overlay 11. It includes user-interface operations and disk
handling as well as the saved-state layout.

{{#include src/SAVE.C}}

## Previewing and saving

`read_save_header` has two paths. The preview path seeks to fixed offsets and
builds a menu entry from selected fields. The other path loads state, rebuilds
sea connections and sets reload-related flags.

The save dialogue previews four slots before choosing a destination.
`write_save` displays progress and calls `read_save`. Despite its name,
`read_save` is the writing routine later in this file. Following calls and
operations is necessary where reconstructed names remain confusing.

{{#include src/SAVE.C | "int read_save_header(file, preview)"}}

## Choosing a drive

`choose_drive` accepts a drive letter and checks the selected drive.
`disk_ready` uses BIOS verification for the first two drives, with up to three
attempts and resets between failures. Other drive numbers return success
without this floppy check.

The file keeps these steps alongside the load and save operations that need
them. Cancellation and failure paths return to the surrounding dialogue code.

{{#include src/SAVE.C | "unsigned int drive_choice = -1;"}}

## Two files per save

`load_save` reads the `.SVE` state, then changes the filename extension and
loads the companion `.MAP` file into page 2. `read_save` writes that page
first, then opens the `.SVE` destination and transfers the remaining state.

The map is therefore not contained in the transfer list alone. The
graphics-page storage used during world generation also participates in
persistence. Both files are needed to describe the load path.

{{#include src/SAVE.C | "int load_save(char *file)"}}

## The ordered state layout

`transfer_game` uses the same ordered sequence for reading and writing. Each
call supplies an address and an explicit byte count. Two-byte fields come
first, followed by names, civilisation arrays, histories and larger records.

The lengths provide checks against `CIV.H`. `townbuf` occupies `0xe00` bytes:
128 cities of 28 bytes. `utypes` occupies `0x3b8`: 28 records of 34 bytes.
`armydata` occupies `0x3000`: eight civilisation slots with 128 twelve-byte
units each.

Later calls transfer visibility, replay data, pollution and warming state,
spaceship data and palace state. The stored `map_rng_state` word is visible
near the start, but its presence alone does not establish that the save
contains the C runtime generator's complete current state.

{{#include src/SAVE.C | "void transfer_game(void)"}}

## Near and far transfers

The final helpers implement the address-space distinction used throughout the
transfer list. `transfer_near` reads or writes a buffer directly.
`transfer_far` copies through the shared `msgtext` buffer in chunks of at most
512 bytes, using `movedata` to cross segment boundaries.

These routines complete the connection between the earlier files. Rule tables,
city and unit records, far arrays and map-page data all have concrete
representations that the game must restore. The transfer lengths provide a check on the record declarations.

{{#include src/SAVE.C | "void transfer_far(char far *buffer, int length)"}}

[← Previous](06-battle.c.html) · [Contents](00-opening.c.html)
