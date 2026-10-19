# Tile yields and improvements

`tile_yield(x, y, kind)` in `CITYMGT.C` returns food (kind 0), shields
(1) or trade (2). It first selects the base or special-resource record.
It then applies these rules in order:

1. Irrigation and mining alter land yields using `work_rules`. A road
   adds one trade on Desert, Plains and Grassland.
2. The coordinate pattern can remove shields from Grassland and River.
3. The Colossus adds one to non-zero trade in its own city.
4. Railroad adds half the current yield, rounded down, to each category.
5. Anarchy and Despotism lose one from yields above two, unless the city
   is celebrating.
6. Non-zero trade gains one under Republic and Democracy. Celebrating
   cities receive this bonus from Monarchy upwards.
7. Pollution halves the result, rounded up. Negative results become zero.

The order matters: a railroad bonus is applied before the Despotism penalty
and the final pollution reduction. Pollution reduces yields; it does not
necessarily remove them.

## Settler work

`CONTROL.C` handles road, railroad, irrigation, mining, fortress and
pollution-cleaning commands. `work_rules` supplies terrain-specific work
and transformation values. Some orders change the terrain type instead of
simply setting an improvement bit. Work progress shares the unit's `fuel`
byte with aircraft range tracking.

Road and railroad movement rules are described in [Movement](../entities/movement.md).
Rivers do not give road movement costs. A fortress doubles ordinary terrain
defence; it does not add to a unit's fortification bonus.
