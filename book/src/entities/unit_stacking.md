# Stacks and City Garrisons

Units of one civilization on the same tile form a stack. Only one
civilization can occupy a tile.

## Finding the stack on a tile

Page 2 of the [map storage](../map/memory_layout.md) holds an occupancy byte
for every tile at `(x + 160, y)`. Bit 3 means units are present and bits
0–2 give their owner. `map_owner(x, y)` returns the owner or −1.
`map_stack(x, y)` then scans that civilization's 128 records for the first
unit at `(x, y)`, records the owner in `the_enemy`, and returns the unit
number. If none is found, it clears the stale occupancy byte.

`put_unit()` and `lift_unit()` in `MAP.C` keep the occupancy byte and the
stack links up to date as units move.

## The ring of `next` links

A stack is a circular list. A lone unit has `next = −1`. `stack_add()` in
`BATTLE.C` makes two units point at each other, and inserts further units
after the head. `stack_remove()` finds the predecessor, bypasses the
removed unit, and sets a unit left alone back to −1.

`each_stack(civ, unit, callback)` in `UNITS.C` calls a function for each
unit in the ring, stopping when it returns to the starting unit. It stops
after ten units, so every stack operation built on it (counts, strengths,
and the destruction of a defeated stack) sees at most ten units. The
helpers built on it are:

| Function | Result |
| --- | --- |
| `stack_len(civ, unit, mode)` | Sum of cost (0), defence (1), count (2) or attack (3); mode 4 counts defenders before `unit` |
| `count_u(civ, unit, type)` | Units of one type |
| `stack_strength(civ, unit, role)` | Units of one role |
| `stack_cap(civ, unit)` | Free transport capacity: ship capacity minus land units |

## Choosing the defender

When a stack is attacked, `stack_other()` picks the defender with the
highest score from `score_group_record()`:

```text
land unit: (fortified ? 3 : 2) * defence * terrain defence * 8
other:     defence * 16
veteran:   + score / 2
```

Land units at sea are skipped. City Walls and fortresses do not enter the
choice, although they affect the battle.

## Packing garrisons into cities

Unit records are limited, so the human's cities store part of their
garrison in the city record. When a unit finishes fortifying in its home
city, `ai_turn()` calls `garrison_in()` for that city. It keeps the
first fortified unit homed to the city on the map and packs up to two more
into `townbuf[city].garrison[]`. Each byte holds the unit type in bits 0–5
and the veteran flag in bit 6, and the packed units are removed from
`armydata`. `garrison_in()` clears status bit `0x04` of the city, and
unpacking sets it again.

When another civilization's unit comes next to the city, `unit_sight()`
calls `unpack_garrison()`. It recreates the stored units as fortified
units (veterans keep their status) and clears the bytes. Both functions
return immediately for cities not owned by the human. Even so, the AI's
strength tables count any garrison bytes they find.
