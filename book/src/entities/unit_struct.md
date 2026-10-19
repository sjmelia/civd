# Unit Records (`struct unit`)

Every unit in the world is a 12-byte record in
`struct unit armydata[8][128]` (`GLOBALS.C`, DGROUP `81D4`). The first
index is the civilization slot (0 for the Barbarians), the second the unit
number. A record whose `type` is −1 is free.

```c
{{#include ../../../src/CIV.H:53:65}}
```

| Offset | Field | Use |
| ---: | --- | --- |
| 0 | `status` | Order and condition bits (below) |
| 1–2 | `x`, `y` | Map position |
| 3 | `type` | Index into `utypes`, or −1 |
| 4 | `moves` | Movement left this turn, in thirds |
| 5 | `fuel` | Turns of fuel for aircraft; work progress for Settlers |
| 6–7 | `goto_x`, `goto_y` | Destination; `goto_x` is −1 when there is none |
| 8 | `f8` | Last direction moved (1–8), or −1 |
| 9 | `visibility` | One bit per civilization that can see the unit |
| 10 | `next` | Next unit in the same [stack](unit_stacking.md), or −1 |
| 11 | `home_city` | Supporting city, or −1 |

## Status bits

| Bit | Meaning |
| ---: | --- |
| `0x01` | Sentry. Also set on units carried by a ship. |
| `0x02` | Settlers building a road or railroad; aircraft returning to base |
| `0x04` | Fortifying. `ai_turn()` turns it into `0x08` at the start of the next turn. |
| `0x08` | Fortified (`UNIT_FORTIFIED`) |
| `0x10` | Busy: the AI has assigned the unit for this turn |
| `0x20` | Veteran (`UNIT_VETERAN`) |
| `0x40` | Settlers irrigating |
| `0x80` | Settlers mining |

Settler work combines bits: `0x40 | 0x80` builds a fortress and
`0x80 | 0x02` cleans pollution. While work continues, `fuel` counts the
turns spent.

## Creating units

`create_unit(owner, type, x, y)` in `UNITS.C` takes the first free record
among numbers 0–126. Number 127 is never allocated; the pathfinder uses
`armydata[0][127]` as a probe (see [Path Finding](pathfinding.md)). When
the human's records are full, the game shows the `*UMAX` message once.

The new unit:

- is placed on its tile's stack and made visible to its owner;
- has no orders, destination or last direction;
- receives `fuel` one less than its type's `fuel`;
- is homed to the nearest city if the owner holds it, otherwise to none;
- increments `forces_present[owner][type]`.

It then calls `unit_sight()` (see [Visibility](../map/fog_of_war.md)).
The human's first two units each trigger an introductory notice.

`drop_u_stack(civ, unit)` removes a unit. Despite its name it removes one
unit. A ship at sea that is removed takes its land cargo with it, and a
Carrier at sea takes its aircraft. `forces_present` is decremented, and,
during a battle, the loss is counted in `unit_losses` for the reports.

## Movement allowance

At the start of a civilization's movement phase, `ai_turn()` sets every
unit's `moves` to three times its type's `moves`. Ships get three more
thirds while their owner has the Lighthouse or Magellan's Expedition. See
[Movement](movement.md).
