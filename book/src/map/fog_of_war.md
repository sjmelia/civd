# Exploration and visibility

Exploration and visible units are tracked separately. `map_visible_to`
remembers explored terrain for each civilization; each unit's `visibility`
byte records which civilizations can see it. Cities retain a `visible_size`
for the human's map display. Remembered improvement flags live on page 2.

`reveal()` in `MAP.C` fills unexplored tiles with black. Explored tiles
are drawn through `map_note()`, which checks unit visibility before drawing
foreign stacks. Debug mode `xray_on` bypasses these display checks.

## Unit sight

`unit_sight()` in `UNITS.C` examines a unit's neighbours, updates exploration
and unit visibility, and handles contact. Ordinary sight covers the adjacent
eight tiles. Unit types with sight bit 2 also examine the outer ring of a
five-by-five square. For ships, these extra tiles must be Ocean; Submarines
have additional visibility exceptions.

The same routine updates remembered city sizes and improvement flags for
the human, cancels some destinations when units encounter one another, and
adds AI targets. Sight is therefore part of movement and diplomacy as well
as drawing.

## Map sharing

Audience map exchanges combine exploration masks. Other events can reveal
cities or terrain; their effects must be read at the relevant call site.
The Great Library's technology-sharing effect does not imply a general
map-revelation effect.
