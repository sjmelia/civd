# Buildings and wonders

`improvements[46]` in `GAME.C` contains a sentinel, 21 city buildings,
three spaceship parts and 21 wonders. Each record holds a 24-byte name and
three 16-bit integers: cost, maintenance and prerequisite. Costs are in
tens of shields for the human player.

Buildings use bits in the city's `buildings` field. Improvement n uses bit
n−1. `CIV.H` names the Palace, City Walls and SDI masks. Spaceship parts
and wonders have separate state; they are not extra city-building bits.

A wonder can be built once. Improvement IDs 25–45 map to wonder IDs 1–21
in `marvel_site`, which records the city that built each wonder. Effects
are tested at their use sites, so their scope varies: some affect one city,
some its continent, and some the owning civilization.

`civ_wonder()` and `active_wonder_city()` in `CITYMGT.C` check obsolescence
against `improvement_words[22]`. A wonder expires when any non-barbarian
civilization knows its obsoleting technology. The sentinel 127 means no
obsoleting technology.

## Production table

```c
{{#include ../../../src/GAME.C:259:306}}
```

## Obsolescence table

Values are technology IDs; see the [technology table](../reference/enum_tech.md).

```c
{{#include ../../../src/GAME.C:308:310}}
```
