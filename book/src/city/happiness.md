# Happiness and disorder

`city_process()` in `CITYMGT.C` counts happy and unhappy citizens. The
remaining non-specialists are content. Luxury spending, buildings, troops
and wonders alter those counts before the turn pass compares them.

## Difficulty and city count

For the human, the initial unhappy count is computed with:

```text
n = (government / 2 + 2) * ((14 - 2 * difficulty) / 2)
unhappy = size + (city_index % n + city_count - n) / n + difficulty - 6
```

These are integer operations in the order shown. City index as well as
city count matters. Without the empire adjustment, the population thresholds
are six content citizens at Chieftain down to two at Emperor. For AI cities,
the initial unhappy count is `size - 3`. `fit_citizens()` adjusts the counts
to fit the population and specialist total.

Luxuries initially supply one happy citizen per two luxury points. Temples,
Cathedrals and Colosseums reduce unhappiness; their effects depend on
technology and wonders. Wonders can also add happiness or remove unrest.

## Troops

Under governments below Republic, up to three units with a non-zero attack
factor suppress one unhappy citizen each. Stored garrisons count too.

Republic and Democracy instead penalize supported units counted as abroad.
The base penalty per unit is one under Republic and two under Democracy;
Women's Suffrage reduces it by one. This uses the unit-support tests in the
city pass, rather than a general test for crossing a national border.

## Disorder

When unhappy citizens exceed happy citizens, the turn pass sets city status
bit `0x01`. A city already in disorder can collapse a Democracy into Anarchy
on a later pass. Restoring order clears that bit. Disorder stops shield
contributions and tax collection in the relevant branches; research has
its own government checks.

## Celebrations

A city of at least size three celebrates when it has no unhappy citizens,
at least half its population is happy (rounded up), and its government is
not Anarchy. Status bit `0x40` records the celebration. On a later celebrating
pass, Republic and Democracy can grow by one if food exceeds consumption
and the Aqueduct requirement is met. Celebration also changes tile-yield
rules; see [Tile yields](../map/tile_yields.md).
