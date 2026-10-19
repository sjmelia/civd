# Production and growth

`civ_economy()` in `ECONOMY.C` calls `city_process(city, 0L)` for each
city. `CITYMGT.C` contains both this turn processing and the interactive
city screen. The mode argument chooses which work to perform.

## Food and growth

The turn pass first resolves the food box left by the previous turn. If
it is negative, a supported Settler is removed if one can be found.
Otherwise the city loses one population point. A city reduced to zero is
removed. The food box is then set to zero.

The growth threshold is `(size + 1) * cost_scale`. For the human,
`cost_scale` is 10. For other owners it starts at `16 - 2 * difficulty`,
with a further reduction of two from AD 1 when the human leads the power
ranking and difficulty exceeds Warlord. The same scale applies to production.

At the threshold the city grows. Its food box becomes zero without a
Granary, or `new_size * 5 + 5` with one. If growth would take it above size
10 without an Aqueduct, and rule bit 4 is enabled, the size increase is
cancelled after resetting the food box.

Later in the pass, tile food is added and consumption subtracted: two per
citizen, plus one per supported Settler under Anarchy or Despotism, or two
under later governments. That balance is resolved on the next city pass.

## Shields and completion

Worked tiles supply shields. Factory and power bonuses are applied, then
unit support is subtracted. Existing disorder sets the shield contribution
to zero. The remainder goes into `shield_box`.

A non-negative `production` selects a unit in `utypes[28]`; a negative
value selects `improvements[-production]`. Completion requires the table
cost times `cost_scale`.

Unit completion subtracts the cost and normally leaves the excess in the
box. Settlers can reduce population; a size-one city on Chieftain postpones
Settler completion. Buildings set their improvement bit, wonders record
their city in `marvel_site`, and spaceship parts update the ship. The human
completion path clears the remaining shields for these projects. AI
building completion can retain the excess and choose another project.

## Buying production

For the human, let `r` be missing shields, clamped to 0–999. Unit purchases
cost `r * r / 20 + 2 * r`. Improvements cost `2 * r`, doubled when the
improvement ID exceeds 21, which includes spaceship parts and wonders.
An empty shield box doubles the price again. Integer division truncates.

For example, a 40-shield unit with one shield stored costs
`39 * 39 / 20 + 78 = 154` gold. With none stored it costs 320 gold.

AI purchases follow separate conditions. They add a chosen number of
shields and charge two gold per shield; treasury fractions limit how much
is bought. They do not use the human unit-price formula.
