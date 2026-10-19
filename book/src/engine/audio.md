# Audio Subsystem & Driver Dispatch

CIV.EXE contains no sound code. The player picks a sound card in the setup
menu (`sound_names` in `SETUP.C`), and CIV loads the matching driver as an
overlay at start-up, just as it loads the graphics driver:

| Choice | Driver |
| --- | --- |
| No sounds please | `NSOUND.CVL` |
| IBM sounds | `ISOUND.CVL` (PC speaker) |
| Tandy sounds | `TSOUND.CVL` |
| AdLib/Sound Blaster | `ASOUND.CVL` |
| Roland MT-32 MIDI board | `RSOUND.CVL` |

`run_program` copies the driver's entry points into slots 100–110 of
`driver_slots` (`DRIVER.INC`). CIV then uses five calls:

| Call | Slot | When |
| --- | ---: | --- |
| `sound_init()` | 100 | Start-up (`INTRO.C`) |
| `sound(n, variant)` → `sound_command` | 101 | Every sound and tune; `sound` in `GAME.C` checks that sound is on (`debug & 10h`) |
| `sound_tick()` | 103 | Timer interrupt, about 60 Hz; may ask for a faster or slower timer |
| `sound_fast_tick()` | 104 | Timer interrupt at the fast rate |
| `sound_shutdown()` | 102 | Exit |

`sound_quiet_ticks()` (slot 105) counts timer ticks since the last command,
and CIV also uses it as a source of random numbers.

The commands and the driver's internals are described in
[The AdLib Sound Driver](sound_driver.md); `src/ASOUND.ASM` is its recovered
source.
