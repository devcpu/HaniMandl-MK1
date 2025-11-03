# Simple Automatic Honey Filling Machine

## Configuration & Calibration Persistence

The device keeps its runtime configuration (including scale calibration OFFSET &
SCALE) in `/hmconfig.json` on SPIFFS. To avoid accidentally overwriting a valid
calibration when flashing a new filesystem image, the repository now ships only
a template file:

- `data/hmconfig_template.json` – template defaults that are copied on first
  boot if `/hmconfig.json` does not yet exist.
- `/hmconfig.json` – created (or rewritten) at runtime only. It is intentionally
  **not** versioned and should not be added back to the repo.

### Boot / load sequence

1. Try to read `/hmconfig.json`.
1. If missing or empty: read `/hmconfig_template.json` and seed an in‑memory
   config.
1. After validation / upgrade the runtime config is written once to
   `/hmconfig.json`.
1. Normal operation only ever updates `/hmconfig.json` (never the template).

### Calibration workflow

1. Open the web UI and perform calibration (tare, known weight, empty glass
   weight).
1. The UI writes OFFSET & SCALE into `/hmconfig.json` via the backend.
1. After a restart `setupLoadcell()` applies these values automatically (logged
   on serial as: `Calibration loaded: SCALE=... OFFSET=...`).

### Important: Do not reintroduce `hmconfig.json` into `data/`

If you place a real `/hmconfig.json` back into `data/` and run `uploadfs`, you
will overwrite the runtime file on the device. Keep only the template under
version control.

### Recovering from corruption

If `/hmconfig.json` becomes corrupt the firmware falls back to the template (if
present) or compiled defaults, then rewrites a new runtime file. You will need
to recalibrate only in that rare case.

### Ignoring the runtime file

Add (or ensure) the following entry in your global or project `.gitignore` if
you ever export SPIFFS contents locally:

```
hmconfig.json
```

## Build & Flash (quick reference)

Typical sequence (PlatformIO tasks):

1. Build firmware: `platformio run`
1. (Only if web/data assets changed) Build FS image: `platformio run -t buildfs`
1. Upload firmware: `platformio run -t upload`
1. (Only when FS changed) Upload FS image: `platformio run -t uploadfs`

Avoid step 4 unless you intentionally want to refresh static assets.

### Calibration epsilon (CAL_EPS)

In the load cell initialization the code tests whether a stored SCALE value
differs meaningfully from the neutral default 1.0. A tiny epsilon
(`CAL_EPS = 1e-6`) is used to avoid treating floating point round‑off as a
"real" calibration change. The identifier is deliberately named `CAL_EPS` (not
just `EPS`) because low‑level Xtensa headers define `EPS` as a macro, which
previously caused a compile error when a local constant shared the same name.

### System Information page

The web UI "System Information" page aggregates chip and runtime data from
`ESPHelper::getSystemInfoTable()`, including now:

- Software / build timestamp
- Chip revision, model, core count
- Feature set (WiFi / BT / BLE / embedded vs external Flash / PSRAM presence)
- Flash mode, size, speed, high‑level flash layout (sketch used/free)
- Heap metrics (current / min / max alloc)
- Reset reasons (per CPU)
- Sketch checksum (MD5)

This replaces the need for a separate early serial print of chip info.

### Native build environment

A `native` environment exists but is not part of the default build
(`default_envs = esp32`) because several modules depend on ESP32‑specific
headers (`esp_log.h`, `freertos/FreeRTOS.h`, `ESP32Servo.h`, `Preferences.h`).
To experiment with host‑based tests or mocks you would need to provide stub
headers or conditionally compile out those features. Re‑enable it ad‑hoc via:

```
platformio run -e native
```

(Will currently fail until proper stubs / `#ifdef`s are added.)

______________________________________________________________________

More project documentation to follow.

## Doxygen Documentation (Developer Notes)

The source now contains English Doxygen comments (headers + selected .cpp files)
for:

- Web server / WebSocket layer (`WebServerX.h/.cpp`)
- Event / snapshot dispatch (`freertos_setup.cpp` selected functions)

### Quick Start

1. Install doxygen (host): `sudo apt install doxygen graphviz`
1. Generate a default config (once):
   ```
   doxygen -g Doxyfile
   ```
1. Recommended minimal edits in `Doxyfile`:
   - `PROJECT_NAME           = "Simple Automatic Honey Filling Machine"`
   - `OUTPUT_DIRECTORY       = docs/doxygen`
   - `INPUT                  = src lib include` (add others if needed)
   - `FILE_PATTERNS          = *.h *.hpp *.cpp`
   - `RECURSIVE              = YES`
   - `EXTRACT_ALL            = NO` (only documented symbols)
   - `GENERATE_LATEX         = NO`
   - `GENERATE_XML           = NO` (unless you need CI tooling)
   - `HAVE_DOT               = YES` and `CALL_GRAPH = YES` if you installed
     graphviz
1. Run:
   ```
   doxygen Doxyfile
   ```
1. Open `docs/doxygen/html/index.html` in a browser.

## JavaScript Linting (ESLint)

Web UI scripts in `data/` are linted using ESLint (replacing the previous JSHint
hook).

Setup:

1. Install Node.js (>= 18)
1. Run `npm install`

Commands:

- `npm run lint` – run ESLint (errors fail pre-commit)
- `npm run lint:fix` – attempt automatic fixes

Configuration: flat config in `eslint.config.js` (ESLint v9+). Legacy
`.eslintrc.*` / `.eslintignore` removed. Firmware build does not depend on Node
tooling; this is developer convenience only.

### Keeping Docs Focused

Only core API surfaces are documented right now. Add comments near logic you
expect to reuse externally; avoid noise (no Doxygen for trivial getters, obvious
constants, or one‑off local helpers).

### Optional CI Idea

Add a GitHub Action job that runs `doxygen` and uploads the HTML as an artifact
(or publishes via GitHub Pages). Not added here to keep the repo lean.

______________________________________________________________________
