# Diplomatic Relations

Relations between civilizations are bit flags in `int treaties[8][8]`
(`GLOBALS.C`). `treaties[a][b]` describes how civilization `a` regards `b`.
Contact and peace are always set and cleared in both directions; most other
bits are one-sided.

| Bit | Meaning |
| ---: | --- |
| `0x001` | Contact. Without peace (`(t & 3) == 1`) the pair is at war. |
| `0x002` | Peace treaty. |
| `0x004` | Alliance. Reports print "ALLIED" for peace plus this bit, and cancelling peace clears it, but no recovered code sets it. |
| `0x008` | Grievance: `a` regards `b` as untrustworthy. |
| `0x010` | Contact during the current sixteen-turn period. |
| `0x020` | Fighting during the current period. |
| `0x040` | `a` has an embassy with `b`. |
| `0x080` | Nuclear weapons have entered the relationship. |
| `0x100` | `a` intends to cancel its treaty with `b`. |
| `0x200` | `a` has cancelled the treaty and its next attack is a sneak attack. |

Row 0 is used differently. Bit `0x004` of `treaties[civ][0]` marks a
government that has collapsed into anarchy; `city_phase()` and
`set_government()` in `DIPLOMAC.C` use it to time the revolution and its
announcement.

## Contact and the sixteen-turn period

When units of two civilizations meet, `UNITS.C` calls `diplomacy_event()`,
which sets `0x011` in both directions. If the human is one of the pair, a
first contact, or a meeting at least sixteen turns after the last audience,
opens a [leader audience](audience.md).

Every sixteenth turn, `city_phase()` checks each pair. A pair without
`0x010` loses contact; if it was at war, a civilization with whom the human
has an embassy reports "The war between … has ended." The `0x010` bits are
then cleared for the next period. War therefore lapses after sixteen turns
without contact.

## Setting and breaking peace

Two helpers in `DIPLOMAC.C` change the relationship in both directions:

- `relation_set(a, b, 2)` signs peace, posts a replay event and clears the
  fighting bits.
- `accord(a, b, 2)` cancels peace. It posts a replay event when a treaty
  existed and also clears the alliance bit.

`BATTLE.C` sets `0x020` in both directions after every combat. If the two
civilizations were at peace, it reports "… cancel peace treaty with …" and
calls `accord()`. When the human is the attacker, the victim also records a
grievance (`0x008`). Before an attack, `attack_ok()` asks the human to
confirm breaking a treaty; under a Republic or Democracy the Senate refuses.

## Grievances

`0x008` makes an AI less forgiving: it greets the human with "O most
untrustworthy leader of the infidels" and doubles the tribute it demands.
The bit is set when:

- the human attacks a civilization at peace with it;
- the human agrees to an AI's request to cancel a treaty with a third
  civilization, or joins an AI's war against one (the third civilization
  records the grievance);
- a nuclear weapon strikes the civilization (`atom()` sets `0x088`);
- a civil war creates a new civilization (`treaties[new][old] |= 9`).

`set_government()` clears all of a civilization's grievances when its
government changes.

## Planned cancellation and sneak attacks

An AI at peace with the human that is hostile in an audience, but weaker
than the human or holding a grievance, makes no demand. It sets `0x100`
instead, deciding to end the treaty later. On each later turn,
`city_phase()` has a one-in-three chance of clearing `0x100` and peace and
setting `0x200`. The next battle in which that civilization attacks is
announced as "Sneak attack by … forces!", and the bit is cleared. The same
message appears whenever a civilization attacks the human while the human
still regards it as at peace.
`unit_phase()` treats a civilization with `0x100` set as an enemy when it
plans its continents.

## Embassies

Bit `0x040` in `treaties[civnow][civ]` gives the human intelligence about
`civ`. A Diplomat sets it (`DIPLOMAT.C`). It enables the "Check
Intelligence" option when an emissary arrives, the dossier in the foreign
affairs report, and messages about that civilization's discoveries, wars,
treaties and conquests.

`give_advance()` in `ECONOMY.C` was meant to exchange ambassadors
automatically at Chieftain level when a rival learns Writing. Its test is
`(!treaties[civnow][civ] & 0x40)`: `!` binds first and yields 0 or 1, so the
test is always false. The recovered source keeps that expression because
the original binary contains it.
