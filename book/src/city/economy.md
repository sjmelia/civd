# Taxes and science

Each civilization has a treasury (`money`), a science rate (`sci`) and a
tax rate (`civ_toll`). Rates are in tenths. Luxury spending is the remainder,
`10 - sci - civ_toll`. `civ_research_tally` stores research progress;
`spaceship_arrive_dates` stores spaceship arrival years.

`CITYMGT.C` calculates city trade, corruption, route income, taxes,
luxuries and research. `ECONOMY.C` processes the cities and adjusts AI
rates, chooses research and records discoveries.

## Corruption

The city pass finds distance to an owned Palace, starting with a maximum
of 32. A city with a Palace starts at distance zero. Communism replaces
the distance with 10. For government ID `g`, the loss is:

```text
loss = trade * palace_distance * 3 / (g * 20 + 80)
```

A Courthouse halves it; Democracy sets it to zero. The calculation is run
before and after route income is added. This is a trade deduction; the
source does not apply this formula to shields.

## Splitting trade

After corruption and route income, luxuries are rounded with `+5` before
dividing by ten. Taxes are rounded the same way and limited to the trade
left over. Science receives the remainder. Each specialist then adds two
to its chosen category.

A Marketplace adds half the current taxes and luxuries. A Bank then adds
half of those updated amounts, so the bonuses compound. A Library and a
University each add half the original science amount. Wonders can modify
science further.

During turn processing, a city in order adds its taxes unless its government
is Anarchy. Building maintenance is charged later in `city_process()`.
Military support is deducted from city shields, not paid as a general
one-gold-per-unit treasury charge.

## Research

`give_advance()` records discoveries in `advances_known`, updates discovery
counts and sources, and handles effects such as obsolete Barracks and
Great Library transfers. `has_tech()` tests the packed bits. Technology
prerequisites and names come from `advances[72]` in `GAME.C`.

Research thresholds depend on advances already known, difficulty, the
calendar and other modifiers in `city_process()`. The human research
selection is `selected_index`; the AI chooses with weights from
`advance_value()`, including the leader's personality.
