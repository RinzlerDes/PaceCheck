# PC-1000 PaceCheck — One-Page Quick Reference

*A digest. Spec §7.2 stays the authoritative bench reference. The demo
sheet cites the TA-published bench-addendum values.*

## Pin facts (authoritative — do not "verify" against UM mirrors)

| Signal | Pin | Notes |
|---|---|---|
| SENSE input | **Arduino D5 = PB10 = TIM2_CH3, AF1** | Internal **pull-down mandatory** (§7.1). 1 kΩ series R. UM2825 Table 19 has a known misprint on D4 — this handout is authoritative |
| LOOPBACK out | **Arduino D2 = PD12** | Provided `loopback.c`, ~50% duty. Jumper D2 → D5 for at-home capture testing |
| **Protected pins (§7.3)** | PA1 · PA7 · PH0 · PH1 · PC8 · PC9 · PB6 · PB7 | The provided drivers own them exclusively. Use read-modify-write on each shared register |

## R4 limits and defaults

| Parameter | Legal range |
|---|---|
| E (expected interval) | 100 … 2,000 µs (the 500 Hz to 10 kHz band) |
| T (tolerance) | 10 … 500 µs, **and T < E** |
| N (interval count) | **fixed at 1000** (a qualification is full-count) |

Defaults: **E = 1000, T = 50**.
Reject out-of-range and malformed entries with a message that gives the
cause. Echo the window before each acquisition.

## Demo-row index (§9)

D0 bench setup (graded) · D1 POST + retry · D2 ground-truth chain · D3 illegal entries, then the
CAL defaults acquisition (TOTAL=1000, ±2 µs against the oscilloscope, R12 ≤ 5 s cycle) · D4 dial
nudge (PSC+1 catch) · D5 persistence jitter · D6 ~500 Hz slow drift → FAIL · D7 dial step →
bimodal · D8 10 kHz high rate · DG 663 duty-cycle histogram (symmetry turn) · Q&A two questions.

## Generator setup checklist (§7.2 steps 1–5, compressed)

1. **Warm-up** ≥ 30 min before the first slot. The generator stays on
   between slots.

2. **Square wave** near 1 kHz. Adjust while you monitor the Period
   readout of the oscilloscope. Never monitor the dial.

3. **Levels with the attenuator where it will stay:** after power-on, an analog unit gives a bipolar output.
   Set the DC offset until the oscilloscope shows **LOW ≈ 0 V, HIGH ≈ 3.0–3.3 V**.
   Check the levels on the oscilloscope again after *each* amplitude or attenuator change.
   Do this check before the lead touches the board.
   You can use the TTL jack (≤ 5.5 V, declared at D0).

4. **Power sequence:** apply USB power to the board **before** you
   connect the generator lead.

5. **Record the dial and attenuator positions** on the demo sheet. The
   oscilloscope re-check *is* your preset.

Set the oscilloscope to DC-coupled, 1 MΩ. Check the probe attenuation.
Clear the Period statistics after each dial change.

## The two traps, in writing

1. **PSC + 1.** Counter clock = `f_CK / (PSC + 1)`. 64 MHz → 1 MHz needs **PSC = 63**, not 64.
   D4 catches the off-by-one error. Derive the value. Do not inherit it.

2. **32-bit σ accumulator overflows.** Sum of squared intervals at the 500 Hz end: 1000 × 2500² ≈ 6.3 × 10⁹ ≈ 1.5 × `UINT32_MAX`.
   Use `uint64_t` accumulators or mean-centered sums.
   An incorrect σ passes D3 at 1 kHz with no warning and fails D6/D7 at 500 Hz (this is deliberate).
