# Game Record Declarations

`src/CIV.H` defines the shared city, unit, unit-type and terrain records shown in the following pages. In this DOS game, `int` occupies two bytes and `long` occupies four.

Cities and units use compact records with byte-sized coordinates and status fields. Cities name their food and shield reserves, worked-tile mask, specialist types, name index, trade routes and stored garrison units directly. Unit `f8` retains its provisional name and has uses including pathfinding direction.

Shared declarations are organized in headers named after the module defining the symbols. The [function reference](api_signatures.md) groups them by subsystem.
