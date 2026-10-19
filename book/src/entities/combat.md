# Combat (`BATTLE.C`)

A unit attacks by moving onto a tile held by another civilization.
`ai_turn()` checks that the attack is allowed, chooses the defender with
`stack_other()` (see [Stacks](unit_stacking.md#choosing-the-defender)) and
calls `battle(attacker_civ, attacker, defender_civ, defender, 1)`. One
random comparison decides the outcome, and the loser is destroyed.

## Who may attack

`ai_turn()` refuses:

- a land-unit attack from the sea (`*AMPHIB`);
- a Submarine attacking land;
- a computer unit with no attack factor, and any Diplomat;
- an attack on aircraft outside a city by anything except a Fighter
  (`*FIGHTER`).

A human unit with less than a full move asks "Attack at n/3 strength?"; a
computer unit needs at least two thirds. If the human is at peace with
the defender, `attack_ok()` asks whether to break the treaty, and under a
Republic or Democracy the Senate cancels the attack.

## Strengths

`battle()` computes an attack value `a` and a defence value `d`. The
terrain factor `t` is `terrains[type].defense` for the defender's tile, in
halves: 2 for open terrain, 3 for Forest, Swamp, Jungle and River, 4 for
Hills and 6 for Mountains.

```text
a = attack * 8

d = t * defense * 8     on a fortress tile (map flag 0x20)
d = t * defense * 6     fortified (12 / 2)
d = t * defense * 4     otherwise
d = defense * 8         defender is an aircraft or ship (replaces the above)
d = t * defense * 12    in a city with City Walls, unless the attacker's
                        attack factor is 12 or the defender is an aircraft
```

Then, in order:

1. **Barbarian attackers.** Against the human, `a = a * (difficulty + 1) / 4`;
   against a computer player, `a /= 2`. Against a city, `a` becomes 0 if it
   is the defender's only city and is halved if it holds the Palace.
2. **Veterans.** Each side's veteran status adds half.
3. **Partial moves.** An attacker with fewer than three thirds left has
   `a = moves * a / 3`.

With `flag = 0`, `battle()` stops here and returns `a * 8 / (d + 1)`. The AI
uses this to [evaluate attacks](../ai/unit_orders.md#attack-odds).

4. **Difficulty.** A non-barbarian attack on the human at Chieftain or
   Warlord is halved. At Chieftain, the human's attacks are doubled.

The walls test compares attack factors, so it exempts both Artillery and
Bombers. A fortified unit in a fortress gets the fortress factor only, and
City Walls replace fortification rather than adding to it. Ships in a
walled city do receive the walls factor.

## Resolution

```c
attacker_wins = rand_scaled(a) > rand_scaled(d);
```

A barbarian attack on a city faces a second roll after winning the first. A second
roll, `rand_scaled(a) < rand_scaled(d)`, can still give the city the
victory. A tie on this second comparison leaves the attacker winning.

If the human is involved or can see either unit, the attack is animated
with sounds chosen by the winner's strength. The battle then sets the
fighting bit (`0x020`) in [`treaties`](../ai/relations.md) in both
directions, reports a sneak attack if one applies, and cancels any peace
between the two.

A Nuclear unit does not fight: `battle()` calls `atom()` and returns. See
[Nuclear Weapons](nuclear_mechanics.md).

## Outcome

When the attacker wins:

- A captured barbarian leader (a Diplomat) pays 100 gold (`*LEADER`).
- Outside cities and fortresses, the whole defending stack is destroyed
  ("n units destroyed."). Because `each_stack()` visits at most ten units,
  at most ten are lost.
- In a city or fortress, only the defender dies. When the last defender of
  a city falls, nearby computer units have their orders cleared. If the
  city is the human's, nearby AI transports are sent towards it
  (`city_captured()`).
- The attacker becomes a veteran half the time.

When the defender wins, the attacker is destroyed and the defender becomes
a veteran half the time. `killed[winner][loser]` counts losses for the
reports in both cases.

Back in `ai_turn()`, a surviving attacker loses one full move (a Bomber
loses all its movement) and never advances into the tile. An attack on a
city that the attacker survives reduces its size by one, and marks a
computer player's city as attacked (status `0x10`). The size is not reduced if the city has City Walls, if
the attack came from the sea, or at Chieftain when the city is the
human's. A city reduced to size 0 is destroyed, and its owner is removed
if it has nothing left.

## Example

A veteran Phalanx (defence 2) fortified on Mountains has
`d = 6 * 2 * 6 * 1.5 = 108`. A Battleship (attack 18) with a full move
attacks with `a = 144`, and wins if `rand_scaled(144) > rand_scaled(108)`,
about 62% of the time. With one third of a move left, `a = 48`
and its chance falls to about 22%.
