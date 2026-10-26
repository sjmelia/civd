# Leader Audiences (`AUDIENCE.C`)

A leader audience is a conversation between the human and one AI ruler.
`leader_audience(civ, x, y, mode)` runs it. The function is in overlay 6,
the largest overlay at 9,642 bytes. It opens in two ways:

- `diplomacy_event()` calls it with `mode = 0` when units of the human and
  `civ` meet for the first time, or meet at least sixteen turns after the
  previous audience.
- A human Diplomat's "Meet with King" order in `DIPLOMAT.C` calls it with
  `mode = 1`.

The ruler's text comes from keyed sections of the game's `KING.TXT`, which
`named_msg()` loads (`*TRIBUTE`, `*PEACE` and so on). The data file is not
part of this repository, so this chapter names the sections rather than
quoting them.

## Receiving the emissary

`audience_open()` shows "An emissary from the … wishes to speak with you.
Will you receive him?" with Yes and No, plus "Check Intelligence" if the
human has an embassy. Intelligence opens the dossier and asks again.
Refusing does not end the decisions: the AI still makes them, and a
declaration of war is still shown as a plain message.

Once the emissary is received, the portrait loads, `anthem` plays and
`leader_greeting()` chooses one of four greetings. A ruler with a grievance
begins "O most untrustworthy leader of the infidels, hear now…", and one
with nuclear weapons adds "Our words are backed with NUCLEAR WEAPONS!"

## The AI's assessment

Before the first line of dialogue, the AI computes four values from
per-continent strength tables built by
[`unit_phase()`](unit_ai.md#strength-tables). Here "attack" and "defence"
are the summed attack and defence factors of a civilization's units on a
continent, and the military total is the sum over all its units.

| Variable | Meaning |
| --- | --- |
| `k` | Fear. On each continent where the AI has towns and the human's attack exceeds the AI's defence, `k` grows by attack ÷ (defence + 1), times 4 at Chieftain or 2 otherwise. |
| `b` | Tribute. Summed attack-minus-defence margins over the other continents. |
| `y` | Hostility. Set when both have units on a continent the AI does not fear. |
| `c` | Willingness to ask for anything: 0, 1, or 2 when hostile. |

The values are then adjusted:

1. **The leader is the target.** The human may rank first, with more than
   four cities and no nuclear weapons, after turn 200, while the AI has more
   than one city. Or the AI may hold a grievance and have a military total
   more than a third of the human's. In either case, if the human has no
   nuclear weapons, the AI becomes hostile, forgets its fear and asks at
   least ten times its own rank in tribute.
2. **Nuclear weapons.** An AI with nuclear weapons sets `0x080` and, if the
   human has none, becomes hostile with a tribute of at least 100. If only
   the human has them, the AI's fear is set to 4. If both do, the tribute
   is scaled by the ratio of their arsenals.
3. **Fear wins.** Any fear clears hostility and `c`.
4. **Wonders.** If the human owns the Great Wall or the United Nations, the
   AI is not hostile.
5. **Price.** A grievance doubles the tribute. The tribute then becomes
   `clamp((difficulty + 1) * b / 32, 0, 20) * 50`, so 0 to 1,000 gold. If
   that exceeds the human's treasury but not twice it, the demand is cut to
   the treasury, rounded down to fifty.
6. **Weakness.** An AI with no tribute to ask, or whose military total is
   under a third of the human's, is not hostile.

Next, the AI counts reasons to be conciliatory. It starts at −2 with a
grievance, adds one for each war in which it is not overwhelmingly
stronger and one for each war against a stronger rival, subtracts its
aggression trait `aggressive`, and subtracts one if the human is weaker than it.
If the count exceeds 0 (at peace) or 1 (otherwise), and the leader is not
the target, the AI drops its hostility, and half the time its tribute.

A hostile AI under a Republic or Democracy without a grievance backs down
when the human's military total exceeds two thirds of its own. Otherwise
it changes its government to Monarchy and carries on.

Finally, an AI at peace with the human that is still hostile, but weaker
or holding a grievance, makes no demand now. It marks the treaty for later
cancellation (`0x100`, see [relations](relations.md#planned-cancellation-and-sneak-attacks)).

## Technology exchange

The AI looks for an advance it knows and the human lacks, and an advance
the human knows that it values most (`advance_value()`). If it is not
hostile and has not fought the human this period, or if it knows six
advances or fewer, and the human knows at least as many advances as it
does, it offers an exchange (`*EXCHANGE`) naming one of its advances.

On acceptance each side takes one advance from the other through
`steal_advance()`: the AI takes the one it values most, and the human
chooses from a menu of the AI's advances. The offer then repeats until it
is refused or nothing is left to trade.

## Demands

In order, a willing AI may:

1. **Ask the human to cancel a treaty** (`*CANCEL`) with a third
   civilization that the AI is at war with. It "requests" or, when hostile,
   "demands". Agreeing cancels that treaty, gives the third civilization a
   grievance, ends the AI's hostility, and half the time its tribute.
2. **Demand tribute** (`*TRIBUTE`) when hostile and the human can pay.
3. **Demand an advance** (`*TAKECIV`) if it made no tribute demand.

Paying or handing over the advance ends the hostility.

## War

If the AI is still hostile, the audience ends. If one of these applies, it
first declares war:

| Section | When |
| --- | --- |
| `*PROVOKE` | The two were at peace and the tribute was over 50. |
| `*REJECT` | A demand was refused. |
| `*RID` | First contact, or an audience called by a Diplomat. |

Each calls `accord(civnow, civ, 2)`. A hostile AI already at war says
nothing more.

## Peace

An AI that is not hostile, and not already at peace with the human (or
meeting a Diplomat), proposes peace (`*PEACE`). Accepting shows the treaty
text (`*TREATY`) and calls `make_peace()`, which signs the treaty and delays
the next audience by sixteen turns. At Chieftain level `make_peace()` also
disbands the AI's units whose nearest city is human-owned and at `town_gap <= 2`.

A human under a Republic or Democracy who refuses is told "You are
overruled by the Senate. Treaty signed." A refusal under another government
ends an existing treaty.

A frightened AI that was refused then raises its offer, while there is no
treaty or grievance:

1. `*GIVECIV`: one of its advances, when its fear is at least 4.
2. `*GIVECASH`: `clamp(2k − 4, 0, treasury / 50) * 50` gold.
3. `*GROVEL`: when it has at most one city, all its gold and every advance
   the human lacks.

If no peace results, `*MOBILIZE` closes the audience at war.

## Closing at peace

A friendly AI at peace either states its attitude (`*ATTITUDE`) or
replies "We welcome the friendship of the … people and their most wise and
munificent leader". The human then gets a menu (`*THREAT`):

- **Threaten.** A frightened AI pays `clamp(k, 0, treasury / 50) * 50` gold
  (`*GIVEMORE`). One that was hostile at the start declares war
  (`*PROVOKE`). Otherwise: "We ignore your feeble threats." The option is
  dimmed under a Republic or Democracy, and within sixteen turns of the
  previous audience or of a gift in this one.
- **Ask for an attack on a third civilization.** The AI refuses without
  contact (`*NOCONTACT`) or because it is already at war (`*ATWAR`).
  Otherwise it names a price (`*MERCENARY`) based on the target's military
  total and its own treasury. The recovered code moves the gold and reports
  "… declare war on the …", but does not change `treaties`. The option is
  dimmed when only the two civilizations remain.

The audience then sets `0x100` if cancellation was planned, clears the
fighting bits (`0x020`) in both directions and closes the portrait.
