# AI Diplomacy Between Rivals

Rivals make treaties and wars among themselves without the human, through
`diplomacy_event()` and `city_phase()` in `DIPLOMAC.C`. The same war test
also decides whether two rivals stay at peace.

## Meetings

`UNITS.C` calls `diplomacy_event(a, b, x, y)` when units of two
civilizations meet. Contact (`0x011`) is set in both directions. If neither
is the human:

- A repeated meeting is ignored three times in four
  (`(a + b + turn_tick) & 3`).
- A pair in which either side holds a grievance does nothing.
- Otherwise each side runs the war test against the other. A side that
  passes does not go to war if the target owns the Great Wall or if its own
  government is a Republic or Democracy.

If neither side goes to war, `trade_advances()` swaps one advance each way
when both have something the other lacks, and a pair not already at peace
signs a treaty. If either side goes to war, a pair at peace cancels its
treaty and a pair meeting for the first time declares war; a pair already
at war carries on. Messages ("… sign a peace treaty", "… cancel peace
treaty", "… declare war on each other") appear only if the human has an
embassy with one of them.

## The war test

`alliance_ok(a, b)` (the name is provisional) returns 1 when `a` is willing
to fight `b`:

1. If `a` is at war with the human, it leaves alone a `b` weaker than the
   human, unless `b` is allied with the human. The same rule applies with
   `a` and `b` reversed.
2. `a` does not attack a civilization with more nuclear weapons.
3. On every continent where `a` has towns, half of `a`'s attack plus its
   defence must exceed `b`'s attack there.
4. `a` compares the attack it can bring against `b`'s strength on the
   continents where both have units:

   ```text
   wc   = a's other wars + (1 if a and b are at peace) - a's aggression aggressive
   x    = sum of a's attack on continents where b has defenders
   den  = 1 + sum of (b's attack / 2 + b's defence) on those continents
   pass = x * 4 / den >= wc + 4, or a has no towns on those continents
   ```

Other wars and an existing treaty raise the bar; an Aggressive leader
lowers it by one. Clearing bit 1 of `rule_options` would make the test
always pass, but `new_game()` sets all four rule bits.

## Requests to the human

Once every 32 turns (on the turn where `turn_tick & 31` equals four times
its slot number), each AI checks its recent fighting. For each civilization
`i` that it fought this period, while it has not fought the human and the
human is at peace with `i`:

- If `i`'s military total is more than twice its own, it asks for help
  (`*HELPME`) and offers `clamp(treasury / 100, 1, 10) * 50` gold.
- If `i` is stronger, but no more than twice as strong, it proposes a joint
  war (`*CRUSADE`), worded "agree to a" or "extend our" depending on
  whether it already has a treaty with the human.

Either way, accepting gives the AI a treaty with the human, cancels the
human's treaty with `i`, and gives `i` a grievance against the human. In
the `*HELPME` case the code adds the gold to both treasuries rather than
moving it from the AI to the human.

## Ganging up on the leader

`rate_civs()` in `TURN.C` ranks the civilizations each turn by

```text
treasury / 32 + citizens * 8 + advances + sum of unit cost over all units
```

and stores the rank (7 for the leader) in `nation_ratings`. When the human
leads, has more than four cities and no nuclear weapons, and more than 200
turns have passed, it sets `gangmode`. Every AI with more than one city
that is at peace with the human marks its treaty for cancellation
(`0x100`); the others are put in contact with the human, and so at war. The same condition makes AIs hostile in
[audiences](audience.md#the-ais-assessment). While `gangmode` is set, AI
cities on continents planned for expansion (plan 5) choose production as if
they were at war.

## Government

`unit_phase()` reconsiders an AI's government every eighth turn (staggered
by slot). It chooses Despotism while its troops exceed what its cities can
support. Otherwise it prefers, in order:

1. Republic, if it knows The Republic and `margin_democracy` is not
   negative (see [Leaders and Personalities](leader_personalities.md));
2. Communism, if it knows Communism and has more than ten cities;
3. Monarchy, if it knows Monarchy.

An AI does not choose Democracy, although a hostile Republic or Democracy
in an audience may revert to Monarchy. After a revolution, `city_phase()`
moves an AI from Anarchy to Despotism on the next turn divisible by four,
or at once with the Pyramids.
