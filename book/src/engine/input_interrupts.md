# Input and interrupts

The game combines keyboard polling, mouse callbacks and a hardware timer.
`key_cmd()` and menu routines wait for commands; `CONTROL.C` interprets
commands during unit movement. Keyboard helpers include entries loaded from
`MISC.EXE`, whose source is `MISC.ASM`.

The numeric keypad supplies eight movement directions. Letter commands
include B to found a city, F to fortify, S for sentry, R for roads, I for
irrigation, M for mining and G for a destination. Context determines which
commands are available. The tests and resulting orders are in `ai_turn()`.

## Mouse

`MOUSE.INC` talks to the installed DOS mouse driver through INT `33h`.
The game uses its own sprite cursor through graphics-driver calls.
`mouse_on()` and `mouse_off()` in `GAME.C` keep a nesting count, so paired
calls preserve the visibility state across nested drawing operations.
Cursor drawing saves the pixels underneath and restores them on erasure.
The MCGA driver limits that saved area to 512 bytes.

## Timer

`TIMER.INC`, included by `RESIDENT.ASM`, installs an INT `08h` handler and
programs the hardware timer. It uses a private interrupt stack, calibrates
against vertical retrace, and chains to the saved handler as accumulated
time requires. The sound driver can request a faster rate for pitch sweeps.
Normal timer processing services sound and palette animation.

`restore_timer()` restores the saved vector and timer programming. This
is a direct IRQ0 hook, rather than an INT `1Ch` callback running only at
the standard DOS timer rate.

## Break handling

`install_break_handlers()` in `GAME.C` saves the INT `1Bh` and INT `23h`
vectors and replaces them with an empty handler. `restore_break_handlers()`
restores them at exit. Disk operations have separate error paths in
`FILEIO.C` and `SAVE.C`; they should be read separately from break handling.
