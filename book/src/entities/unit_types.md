# Unit Types (`struct unit_type`)

The 28 unit types are 34-byte records in `utypes[28]` (`GAME.C`, DGROUP
`112A`).

```c
{{#include ../../../src/CIV.H:13:26}}
```

| Field | Meaning |
| --- | --- |
| `obsolete_by` | Advance that makes the type obsolete; 127 for none |
| `domain` | 0 land, 1 air, 2 sea |
| `moves` | Moves per turn; `ai_turn()` multiplies by 3 |
| `fuel` | Turns an aircraft may stay airborne (Fighter 1, Bomber 2, Nuclear 1) |
| `attack`, `defense` | Combat factors |
| `cost` | Production cost in tens of shields |
| `sight` | Bit 1 (value 2) widens sight from the 3 × 3 to the 5 × 5 square |
| `capacity` | Land units a ship carries |
| `role` | Category used by the AI ([roles](../ai/unit_ai.md#unit-roles)) |
| `prereq` | Advance needed to build the type; −1 for none |

`GAME.C` declares its own copy of the structure with fields `w2`–`w10` in
the same positions. The table, in ID order:

```c
{{#include ../../../src/GAME.C:328:357}}
```

## Building a type

`find_slot()` in `ECONOMY.C` offers a unit type when the civilization
knows its prerequisite and does not know its obsolescence advance. Ships
need a coastal city (status bit `0x02`). Nuclear units also need the
Manhattan Project to have been built by someone and the owner to know
Nuclear Fission.

The AI disbands obsolete units over time (see
[Unit AI](../ai/unit_ai.md#sending-units-to-targets)). The game does not
upgrade them.

## Special abilities

The table gives few abilities directly; most are tests on type numbers in
the code:

| Type | Behaviour |
| --- | --- |
| Settlers (0) | Only type that builds cities, joins them and changes terrain |
| Fighter (14) | Only type that may attack aircraft outside a city |
| Bomber (15) | Its movement ends after an attack |
| Trireme (16) | A human Trireme that ends its move with no land next to it is lost half the time (`*TRIREME`) |
| Submarine (22) | Cannot attack land. Not seen beyond adjacent tiles, and does not see beyond them itself |
| Carrier (23) | Carries any number of aircraft (its `capacity` is 0), and refuels them |
| Nuclear (25) | Entering a foreign city detonates it (`atom()`) |
| Diplomat (26) | Enters foreign cities for the Diplomat menu and bribes units by moving onto them. Ignores zones of control. The Barbarians' leader is a Diplomat. |
| Caravan (27) | Establishes trade routes or helps build a wonder. Ignores zones of control. |

Attack factor 12 (Artillery and Bomber) ignores City Walls; see
[Combat](combat.md).

Capacities are Trireme 2, Sail 3, Frigate 4 and Transport 8.
Fighters, Bombers and the Cruiser, Battleship, Submarine and Carrier have
sight bit 1; aboard ships, the wider sight covers only ocean.
