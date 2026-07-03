# qmfcalc

Quadrupole mass filter calculation utilities.

## Python

The Python package builds a native extension with `pybind11`:

```sh
python -m pip install .
```

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
```

`rf_voltage_v` is the per-rod peak-to-peak RF voltage. `dc_voltage_v` is the
pole-to-pole DC voltage.

The GitHub Actions workflow in `.github/workflows/python-package.yml` builds and
tests Linux and Windows wheels on pushes and pull requests.
