# Command reference

How to build and run the executables in this project.

## Build

ROOT (CERN) must be on the environment first. `argparse` and `fkYAML` are fetched
automatically by CMake (`FetchContent`), so a network connection is needed on the
first configure:

```sh
source /usr/local/root/bin/thisroot.sh
cmake -S . -B build
cmake --build build -j
```

To build only the resolution analysis:
`cmake --build build --target analyze_resolution -j`.

## analyze_resolution

**Purpose:** Given a quadrupole Mathieu operating point, estimate the mass
resolution `m/dm` from the geometry of the first stability region. The scan line
through the origin and the operating point crosses the stability tip at `q_low`
and `q_high`; the resolution is `q_center / (q_high - q_low)`.

**Input** — exactly one of (mutually exclusive, one required):
- `-qa`, `--q_and_a q,a` : an explicit operating point, e.g. `-qa 0.70,0.23`.
- `-qova`, `--q-over-a RATIO` : the scan-line ratio `q/a` (the estimator slope is
  `a/q = 1/RATIO`), e.g. `-qova 3.0`.

**Options:**
- `-o`, `--output DIR` : output directory. Default is derived from the
  configuration: `resolution_qX.XXX_aY.YYY_qoveraZ.ZZZ` when `(q, a)` is given,
  otherwise `resolution_qoveraZ.ZZZ` (values to 3 decimals).

**Run** (outputs are written under the output directory relative to the current
directory, so run from `exec/` to keep them beside the source):

```sh
source /usr/local/root/bin/thisroot.sh
cd exec
../build/exec/analyze_resolution -qa 0.70,0.23
../build/exec/analyze_resolution --q-over-a 3.0
../build/exec/analyze_resolution -qova 3.05 -o custom_out
```

**Outputs** (inside the output directory):
- stdout: slope, `q_low`/`q_high`, mass center `(q_center, a_center)`, `m/dm`
- `result.yaml` : input configuration and result fields (fkYAML)
- `c_stability_scan.pdf`, `png/c_stability_scan.png` : stability diagram with the
  scan line and crossing markers
- `data.root` : roothelper canvas data plus a `TTree` named `result` with one
  entry holding the input and result fields

The reusable kernel lives in `core/include/qmfcalc/`:
`stability_diagram.h` (boundary edge calculators) and `resolution_estimator.h`
(`estimate_mass_resolution`, `estimate_mass_resolution_from_slope`,
`find_slope_for_mass_resolution`), the last used by `calculate_voltage` below.

## calculate_voltage

**Purpose:** The inverse of `analyze_resolution` — given a target mass center,
resolution width, field radius and RF frequency, compute the DC (U) and RF (V)
electrode voltages. It recovers the operating line from the requested resolution
(`find_slope_for_mass_resolution`), places the operating point at the transmitted
mass center, and converts the Mathieu `(q, a)` to voltages.

**Conventions** (see `core/include/qmfcalc/voltage_calculator.h`): `r0` is the
distance from the central axis to the rod surface (field radius). `U` is the
**pole-to-pole DC** (rods at `+/-U/2`), `a = 4eU/(m omega^2 r0^2)`. `V` is the
**per-rod peak-to-peak RF** (each rod swings `+/-V/2`), `q = 2eV/(m omega^2 r0^2)`.

**Input** (all required):
- `-m`, `--mass U` : mass center m [u]
- `-dm`, `--delta-mass U` : resolution width delta_m [u] (resolution R = m/delta_m)
- `-r0`, `--r0 MM` : field radius r0 (axis to rod surface) [mm]
- `-f`, `--frequency MHZ` : RF frequency f [MHz]

**Options:**
- `-o`, `--output DIR` : output directory. Default: `voltage_mX.XXX_dmY.YYY_r0Z.ZZZ_fW.WWW`.

**Run:**

```sh
source /usr/local/root/bin/thisroot.sh
cd exec
../build/exec/calculate_voltage -m 250 -dm 1 -r0 15 -f 1
```

**Outputs** (inside the output directory):
- stdout: R, operating `(q, a)`, `U` and `V` in V and kV
- `result.yaml` : input and result fields (fkYAML)
- `data.root` : roothelper canvas data plus a `TTree` named `result` with one
  entry (`mass_u, delta_mass_u, r0_mm, frequency_MHz, resolution, q, a, U_volt, V_volt`)
- `c_stability_scan.pdf`, `png/c_stability_scan.png` : stability diagram with the
  operating line and the operating-point marker
