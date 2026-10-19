# Unit AI (`UNITAI.C`)

The computer players' unit planning is in `src/UNITAI.C`, which compiles to
the resident segment `F15FB` (16,266 bytes). It has two halves:

- `unit_phase(player)` runs once per civilization per turn. It measures
  strength on every continent, decides a plan for each continent, chooses
  the government and sends idle units towards targets.
- `ai_order(player, unit)` runs for each unit as it moves. It returns the
  key the unit would press: see [Unit Orders](unit_orders.md).

`game_turn()` in `TURN.C` processes each civilization in turn:
`civ_economy()` for its cities, then `city_phase()`, `unit_phase()`,
`unit_cleanup()` and finally `ai_turn()` in `CONTROL.C`, which moves the
units. `ai_turn()` is used for the human too; for a computer player it
calls `ai_order()` instead of reading the keyboard.

## Unit roles

Each unit type has a `role` in `utypes` (`GAME.C`). The AI matches units to
targets by role.

| Role | Meaning | Unit types |
| ---: | --- | --- |
| 0 | Settle | Settlers |
| 1 | Attack | Legion, Cavalry, Knights, Catapult, Cannon, Chariot, Armor, Artillery, Bomber, Nuclear |
| 2 | Defend | Militia, Phalanx, Musketeers, Riflemen, Mech. Inf. |
| 3 | Naval combat | Ironclad, Cruiser, Battleship, Submarine, Carrier |
| 4 | Air superiority | Fighter |
| 5 | Transport | Trireme, Sail, Frigate, Transport |
| 6 | Special | Diplomat, Caravan |

## Strength tables

`unit_phase()` first rebuilds per-continent tables for the civilization.
Continents are the region numbers 1–14 from [world generation](../map/worldgen.md).

| Table | Contents |
| --- | --- |
| `defenders_island[civ][region]` | Sum of the defence factors of land-based units and city garrisons |
| `civ_cont_attackers[civ][region]` | Sum of the attack factors of the same units |
| `ai_continent_towns[civ][region]` | Number of cities |
| `military_table[civ]` | Sum of attack plus defence over all units except Nuclear |
| `troops_total[civ]`, `civ_settlers_n[civ]` | Unit counts |

A land unit at sea is marked busy (`status` bit `0x10`) and left out of the
continent tables. The pass also builds `fog_map`, a 20 × 13 grid of 4 × 4
tile blocks. Bit 0 marks a block containing one of the civilization's
cities and bit `1 << (role + 1)` a block containing a unit of that role.
`civ_held_cells` counts the occupied blocks.

The same loop over cities adds targets. Each foreign city that the
civilization may attack becomes an attack target (kind 1). Its priority is
3 with City Walls, otherwise 5. A city with no units on its tile and no
garrison becomes a defence target (kind 2): priority 4 for the
civilization's own cities, 2 for others. A foreign city qualifies as an
attack target if it is not covered by peace, and if it is visible, belongs
to another AI, or `turn_tick * difficulty` exceeds 200. One city in four
is skipped on each turn.

## Continent plans

For each continent, `unit_phase()` sets `civ_area_plan[civ][region]`:

| Plan | Meaning | Condition |
| ---: | --- | --- |
| 0 | Develop | None of the conditions below |
| 5 | Expand overseas | The civilization knows Map Making, and the continent is crowded: total defence is more than half its land area, or towns × 6 + 2 exceed its city sites |
| 1 | Attack | An enemy (at war, or with cancellation planned) is present that the civilization can outfight: its defence is below the civilization's attack, or its attack is below the civilization's defence. Barbarian cities also count. |
| 2 | Defend | An enemy is present that the civilization cannot outfight, or the human, although at peace, has more attack there than half the civilization's attack plus its defence |
| 1 | Attack | The civilization has no units or cities on the continent, and does not share it only with civilizations at peace with it |

The rules are applied in this order, and a later rule overrides an earlier
one. Enemies count only if they have defenders on the continent and either
cities there or more than a quarter of the civilization's attack. When a plan changes,
`retarget()` makes every city of the civilization on that continent choose
its production again.

The tables also set `near_human` when the civilization has defenders on a
continent where the human has more than one city.

After the plans, Barbarians stop here, and so do human players.
A computer civilization with no cities or settlers is removed with
`kill_civ()`. Government is then reconsidered every eighth turn (see
[AI Diplomacy](ai_diplomacy.md#government)).

## Targets and sites

Each civilization has two lists:

- **Targets**: 32 entries of position, kind (a unit role) and priority
  (`civ_targets_x`, `civ_targets_y`, `objectives_type`, `aim_count`).
  `add_target()` keeps them sorted by priority and ignores a duplicate of
  equal or lower priority. Units add targets as they move.
- **Sites**: 16 remembered entries (`camp_xpos`, `civ_lot_y`,
  `spot_classes`, `ai_camp_site_weights`), mostly city sites found by
  settlers. `unit_cleanup()` clears the targets at the end of each
  `unit_phase()` and re-adds every site of kind 0–2 as a target.

## Sending units to targets

`unit_phase()` then looks at each idle unit. Units that are not busy, and
not Nuclear, are considered.

First, a unit may be disbanded. Every sixteenth turn for a given unit
(`(unit + turn_tick) & 15`), the AI disbands it when its type is obsolete
for the civilization, or when it is Militia and the civilization is beyond
Despotism or the continent's plan is 5. A unit is kept if it is blocked by
enemies, is alone in a city, is at sea, or is on a continent planned for
defence.

Otherwise, the unit considers every target of its role on its own
continent. Bombers consider attack and naval targets on any continent, but
only odd-priority targets on alternating turns, with a cost that doubles
when `map_dist()` exceeds the unit's movement allowance. For other units the cost of a
target is

```text
cost = slots[target] * distance / (priority + 1)
```

where `slots` starts at `max(3, troops / 8)` for every target and grows by
one each time a unit is sent. Units that are fortifying or fortified leave
only for targets with a priority of at least two that are not already
well served, and never leave a city. A target is skipped when four times its priority is less than its
cost divided by the starting slot value. The unit takes the cheapest
target as its destination (`goto_x`, `goto_y`) and is marked busy.
