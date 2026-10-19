# 2. Shared game records

Complete file:
[`src/CIV.H`](https://github.com/sjmelia/civd/blob/main/src/CIV.H).

Before reading the larger C files, we can read one complete header. `CIV.H`
declares records used for terrain types, unit types, cities and individual
units.

The field names are inferred. Their widths and signed types preserve the
DOS representation. Some names elsewhere in the source are awkward because
identifier spelling can affect the old compiler's allocation of variables
and the linker's order of global storage.

{{#include src/CIV.H}}

## Unit types

A unit type contains properties shared by every unit of that type: its name,
movement allowance, attack and defence strengths, cost and other values. Under
the period compiler, each `int` occupies two bytes. The twelve-byte name and
eleven integers therefore occupy 34 bytes.

An individual unit stores a type index rather than repeating this record. We
will see `BATTLE.C` use that index to obtain the attacker's and defender's
strengths.

{{#include src/CIV.H | "/* DGROUP 112A: 28 unit types, 34 bytes each. */"}}

## Cities

The city declaration accounts for all 28 bytes. `food_box` and `shield_box`
hold accumulated food and production. `workers` combines the worked-tile
mask with a specialist count, while the separate `specialists` word stores
eight two-bit specialist types.

`name_id` selects a city name, and `routes` holds three destination city
indices. The two `garrison` bytes store unit types and veteran flags for
units packed into the city record. Empty route and garrison slots use `-1`.

`name_id` is stored as a `char`, with explicit unsigned casts when used
as an index. Those casts matter on a compiler with signed plain characters.

`buildings` is a bit mask stored in a `long`. The masks below identify a
palace, city walls and SDI defence. Battle code can test a particular
improvement without searching a list.

{{#include src/CIV.H | "/* DGROUP 70EC: 128 cities, 28 bytes each. */"}}

## Individual units

The twelve-byte unit record contains its location, type, remaining movement,
status and other state. Movement uses thirds of a movement point. Fortified
and veteran status occupy separate bits in the same byte.

The signed `next` field links units in a stack and uses `-1` as a sentinel.
Its complete interpretation depends on the stack routines, which we will read
in `BATTLE.C`. A declaration establishes the representation; those routines
establish how the program changes it.

{{#include src/CIV.H | "/* DGROUP 81D4: 128 units per civilization, 12 bytes each. */"}}

[← Previous](01-makefile.mk.html) · [Contents](00-opening.c.html) · [Next →](03-game.c.html)
