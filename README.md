# qmfcalc

Quadrupole mass filter calculation utilities.

## Python

The Python package builds a native extension with `pybind11`:

```sh
python -m pip install .
```

Python 3.12 and 3.13 are supported targets.

or directly from a Git checkout:

```sh
python -m pip install git+https://github.com/<owner>/<repo>.git
```

Use it from Python:

```python
import qmfcalc

result = qmfcalc.calculate_voltages(
    r0_mm=15.0,
    target_m_over_q_amu_per_z=250.0,
    resolution=250.0,
    rf_frequency_mhz=1.0,
)

print(result["rf_voltage_v"], result["dc_voltage_v"])

inverse = qmfcalc.calculate_mass_resolution(
    r0_mm=15.0,
    rf_voltage_v=result["rf_voltage_v"],
    dc_voltage_v=result["dc_voltage_v"],
    rf_frequency_mhz=1.0,
)

print(inverse["m_over_q_amu_per_z"], inverse["resolution"])
```

`rf_voltage_v` is the per-rod peak-to-peak RF voltage. `dc_voltage_v` is the
pole-to-pole DC voltage. `calculate_mass_resolution` uses the same voltage
conventions and returns the transmitted mass-to-charge value in amu/Z plus the
resolving power.

The GitHub Actions workflow in `.github/workflows/python-package.yml` builds and
tests Python 3.12 and 3.13 Linux and Windows wheels on pushes and pull requests.

## PyPI release

Configure a PyPI pending trusted publisher with:

- PyPI project name: `qmfcalc`
- Owner: `kenji0923`
- Repository: `qmfcalc`
- Workflow name: `publish-pypi.yml`
- Environment name: `pypi`

Then publish a GitHub Release from a version tag, for example:

```sh
git tag v0.1.2
git push origin v0.1.2
```

The `.github/workflows/publish-pypi.yml` workflow builds the Python 3.12 and
3.13 Linux and Windows wheels, builds the source distribution, and publishes
them to PyPI.
