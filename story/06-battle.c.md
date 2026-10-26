# 7. Resolving combat

Complete file:
[`src/BATTLE.C`](https://github.com/sjmelia/civd/blob/main/src/BATTLE.C).

`BATTLE.C` combines combat evaluation, combat resolution, stack updates,
treaty checks and nuclear effects. It uses the records from `CIV.H`, so an
individual unit's type index leads to the shared strengths in `utypes`.

The main `battle` function has an older C parameter declaration. Its comment
records a calling-interface constraint: some callers pass long arguments to
int parameters. The source retains that interface. Some local names also retain letter
suffixes because spelling can affect variable allocation in the compiler.

{{#include src/BATTLE.C}}

## Strength calculations

`battle` reads the two unit types and the defender's terrain, then calculates
integer attack and defence values. The branches account for a fortress,
fortification, unit domain, city walls and barbarian conditions. Veteran
status adds half the current strength. An attacker with fewer than three
movement thirds receives a proportional reduction.

When `flag` is zero, the function returns a strength estimate at this point.
That path does not run the later difficulty adjustments, treaty check or
random combat decision. Reading the full function keeps the estimate separate
from the result of an actual attack.

{{#include src/BATTLE.C | "attacker_type_j = armydata[attacker_civ_b][attacker_unit_m].type;"}}

## Decision and animation

For an actual attack, further difficulty conditions can change attack
strength. A treaty check can cancel the current player's action before a
combat roll.

The first random comparison uses a strict greater-than test, so equal results
do not give the attacker a win. A barbarian attack on a city can require a
second comparison. That comparison uses a different inequality; preserving the
operators matters when describing ties.

The result is chosen before the animation block. Visibility and display state
decide whether the game centres the map, plays sound and draws the moving unit
and combat frames.

{{#include src/BATTLE.C | "if (attacker_civ_b != 0) {"}}

## Applying the result

After the animation, the function changes treaty state and handles messages
about attacks and cancelled treaties. Nuclear attacks enter `atom` and return
through a separate path.

For ordinary combat, an attacker victory can remove an entire defending stack
outside a city or fortress. Other branches remove one defender and perform
city-related updates. The losing attacker is removed on the defeat path. A
further random choice can give the surviving unit veteran status.

{{#include src/BATTLE.C | "treaties[attacker_civ_b][defender_civ_e] |= 0x20;"}}

## Stack links and treaty permission

`stack_add` and `stack_remove` explain the `next` field from `CIV.H`. A lone
unit uses `-1`. Adding a second unit makes the links refer back to each other;
larger stacks retain a circular chain. Removing a unit repairs that chain and
resets the removed unit's link.

`attack_ok` handles permission to break a peace treaty. It can reject the
action when the player cancels or when the government allows the Senate to
overrule it. This is the function called before the combat roll above.

{{#include src/BATTLE.C | "int stack_add(int p, int i, int head)"}}

## Nuclear effects

`atom` examines the target and its eight neighbours. It checks city SDI
protection before removing unit stacks, records losses and changes relations.
It then calls `nuke_fallout`.

The final routine reduces affected city sizes or attempts to add pollution to
other non-ocean tiles, then updates the map. These effects remain part of the
combat file even though they bypass the ordinary strength calculation and loss
path.

{{#include src/BATTLE.C | "int atom(int pl, int x, int y)"}}

[← Previous](05-turn.c.html) · [Contents](00-opening.c.html) · [Next →](07-save.c.html)
