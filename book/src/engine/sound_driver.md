# The AdLib Sound Driver (ASOUND.CVL)

Sound, like graphics, lives in a driver loaded at start-up. CIV has
`NSOUND.CVL` (none), `ISOUND.CVL` (PC speaker), `TSOUND.CVL` (Tandy),
`RSOUND.CVL` (Roland MT-32) and `ASOUND.CVL`. `src/ASOUND.ASM` is the
recovered AdLib driver. It also drives a Sound Blaster Pro's two OPL chips
and a Pro AudioSpectrum, and `just verify` rebuilds it byte for byte.
`ASOUND.CVL` and `RSOUND.CVL` carry the same identification
(`RLND Cvlzatn12-03-91`): they share the sequencer and differ in output.

The driver has the same header as the graphics driver (see
[the MCGA driver](graphics_driver.md)) and fills slots 100–110.

## Slots

| Slot | CIV name | Effect |
| ---: | --- | --- |
| 100 | `sound_init(arg)` | Detect the card and silence it (`INTRO.C` at start-up) |
| 101 | `sound_command(n, variant)` | Start sound command n; reached through `sound()` in `GAME.C`, which plays only while sound is on (`debug & 10h`) |
| 102 | `sound_shutdown()` | Silence the card; set the DOS time from the real-time clock (CIV's fast timer made the DOS clock drift) |
| 103 | `sound_tick()` | One music step. Called from the timer interrupt at about 60 Hz; returns a timer request: 1 for fast while a pitch sweep runs, −1 for slow again, 0 for no change |
| 104 | `sound_fast_tick()` | Advance the pitch sweeps (at the fast timer rate) |
| 105 | `sound_quiet_ticks()` | Ticks since the last command; CIV uses it as a source of random seeds |
| 106–110 | | Return at once |

`TIMER.INC` calls slots 103 and 104 and changes the timer rate when
slot 103 asks for it.

## Commands

| n | Use |
| ---: | --- |
| 0 | Silence everything |
| 1 | End the music: every channel goes to the rest sequence |
| 2 | Nonzero while any channel plays |
| 3 | Title and intro music |
| 4 | World creation |
| 5–18 | Civilization anthems (`civt[].f4`): American, Aztec, Egyptian, Zulu, French, Roman, Russian, Greek, English, Babylonian, Chinese, Mongol, Indian, German. The second argument (0–3) chooses how many parts play; 3 is the full arrangement |
| 19–32 | Short motifs for the same civilizations, in the same order (`civt[].f3`) |
| 33 | Audience interlude: pause the channels, play one of seven phrases in turn, then resume |
| 34 | Celebration (We Love the King Day, Hall of Fame) |
| 35 | City grows in rapture (two tunes, alternately) |
| 36 | Bad news (famine, civil disorder, nuclear incident); also the theme of the two unused tribe slots |
| 37 | A land unit moves |
| 38, 39 | Battle won, battle lost: light units |
| 40, 41 | Battle won, battle lost: heavy units (attack above 4 or defence above 2) |
| 42 | Nuclear explosion |
| 43 | A bomber wins its attack |
| 44 | City-view street sounds (random notes) |

## Sequencer

Music is a byte code in the data segment, about 29 KB in all. Each command
starts a sequence on some of the nine channels, one for each OPL2 voice. The
recovered source labels every sequence after the command that plays it, for
example `roman_anthem_part1`.

Every tick, `channels_step` runs `channel_step` once for each channel. This
part of the driver was written in C and compiled without optimisation; the
reload of `channel_now` before each field access gives it away. A channel
record (30 bytes) holds:

- the duration counter and the note-off timer;
- the sequence pointer, two loop points and their repeat counts;
- the volume and instrument.

Bytes `F3h`–`FFh` are commands (loops, repeats, instrument, volume);
the rest encode notes and durations. Two extra voices can sweep their pitch; they run
from the fast timer through slot 104.

The driver keeps a copy of every OPL register it writes (`opl_shadow`).
Pitches come from a table of the twelve semitones' F-numbers. Volumes go
through a 128-step attenuation table. `sound_random` is the driver's own
generator, x = (x + 9248h) rotated right by 3, starting from 1234. Command 44
uses it for the city sounds.

## Hardware

`card_detect` first runs the AdLib timer test on port `388h`. It then looks
for a Sound Blaster Pro at `220h` by writing and reading back mixer register
26h (FM volume). Last it probes the Pro AudioSpectrum's registers at `B8Ah`
and `B8Bh`. On a Sound Blaster Pro, each register write goes to both OPL
chips (left and right).
