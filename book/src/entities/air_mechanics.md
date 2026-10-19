# Aircraft and Fuel

Fighters, Bombers and Nuclear units have domain 1. They pay three thirds
per step on any terrain, ignore zones of control, and cannot enter a
foreign city (`*OCCUPY`), except a Nuclear unit, which explodes there.

## Fuel

`utypes[type].fuel` is the number of turns an aircraft may end away from a
base: 1 for the Fighter and Nuclear unit, 2 for the Bomber. The unit's
own `fuel` byte counts down:

- `create_unit()` sets it to `fuel - 1`, and so does being carried by a
  moving Carrier.
- When the aircraft has used all its movement, `ai_turn()` refills it to
  `fuel` if the aircraft is in a city or with a Carrier, then subtracts
  one. A result below 0 destroys the aircraft (`*FUEL`).
- Ending a move in a city, or on a Carrier, stops the aircraft for the
  turn; on a Carrier it is also refuelled.

A Fighter therefore has to finish every turn at a base. A Bomber may
spend one turn in the air between bases, and the AI sends it home (`'h'`)
when its fuel is spent. A Bomber's movement ends after it attacks.

## Returning to base

The human's `'h'` order on an aircraft away from a city calls
`near_base()` in `UNITS.C`. It finds the nearest of the owner's cities and
Carriers by the larger of the two coordinate distances, then picks the
neighbouring step that most reduces `distance_twice()` to it, avoiding
foreign units and cities. While the order lasts the unit carries status
`0x02`. If no step helps, a computer aircraft is disbanded.

## Carriers

A Carrier (type 23) has capacity 0 in `utypes`, but the movement code
treats it specially: when it moves, aircraft in its stack move with it,
with no limit except the ten-unit cap of stack walks. Aircraft carried this
way are refuelled to `fuel - 1`. A Carrier destroyed at sea takes its
aircraft with it.

## Air combat

Only a Fighter may attack an aircraft outside a city. Defending aircraft
and ships use `defense * 8`, without terrain or fortification. A city's
walls do not help an aircraft. See [Combat](combat.md).
