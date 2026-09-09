# AuraCube-8-8-8-RGB-Custom-STM32-Effects-Firmware
AuraCube 8×8×8 RGB – Custom STM32 Effects Firmware

# AuraCube 8×8×8 RGB – Custom STM32 Effects Firmware

Custom firmware for the **AuraCube 8×8×8 RGB LED cube**, built around the STM32F103 and the original AuraCube display hardware/scan driver.

This project adds several smooth real-time effects, manual RGB control, IR remote control, and a hardware diagnostic mode.

The main goal was to keep the original cube hardware working as intended while replacing slower/choppier custom animation experiments with lightweight fixed-point effects that run well on the STM32F103.

---

## Features

Current modes:

1. **Smooth Hue Sweep**
2. **3D Cloud / Swirl**
3. **Manual Solid RGB**
4. **Layer / Row Diagnostic**
5. **Breathing / Bouncing Ball**

The IR remote is context-sensitive, so some buttons perform different functions depending on the active mode.

---

# Mode 1 – Smooth Hue Sweep

The entire cube displays one solid color while smoothly rotating through the RGB hue spectrum.

### Controls

| Button | Function |
|---|---|
| `+` / `-` | Brightness up / down |
| `FF` / `REW` | Faster / slower hue rotation |
| `PLAY` | Pause / resume |
| `POWER` | LEDs off / on |

Brightness is intentionally limited in this mode because testing showed that very high full-cube brightness can expose flicker/buzzing on some hardware.

---

# Mode 2 – 3D Cloud / Swirl

A continuously moving 3D RGB cloud/plasma effect flows through the cube.

The effect uses smooth hue changes and fixed-point math rather than expensive floating-point calculations.

### Controls

| Button | Function |
|---|---|
| `+` / `-` | Brightness up / down |
| `FF` / `REW` | Faster / slower animation |
| `PLAY` | Pause / resume |
| `POWER` | LEDs off / on |

This mode also uses the safer brightness range.

---

# Mode 3 – Manual Solid RGB

Displays the entire cube as one manually adjustable RGB color.

Each channel can be adjusted independently from `0–255`.

### Controls

| Button | Function |
|---|---|
| `4` | Red up |
| `7` | Red down |
| `5` | Green up |
| `8` | Green down |
| `6` | Blue up |
| `9` | Blue down |
| `PLAY` | Cycle pure Red → Green → Blue |
| `+` / `-` | Overall brightness |
| `POWER` | LEDs off / on |

Unlike the animated modes, **Manual RGB mode has no brightness limiter**.

This allows full output including:

```text
R = 255
G = 255
B = 255
Brightness = 100%
```

Full white at maximum brightness may expose flicker or audible buzzing on some cubes/power arrangements. That behavior is one reason the diagnostic mode was added.

---

# Mode 4 – Layer / Row Diagnostic

A hardware troubleshooting mode for testing individual physical sections of the cube.

It can display either:

- horizontal **layers**
- vertical **row planes**

You can test a single slice or build a cumulative group of slices.

### Controls

| Button | Function |
|---|---|
| `PLAY` | White → Red → Green → Blue |
| `0` | Toggle Layers / Rows |
| `4` | Add another layer / row |
| `7` | Remove a layer / row |
| `FF` | Move selected group forward |
| `REW` | Move selected group backward |
| `+` / `-` | Brightness |
| `POWER` | LEDs off / on |

Example with three layers selected:

```text
1 + 2 + 3
    ↓ FF
2 + 3 + 4
    ↓ FF
3 + 4 + 5
```

This mode is useful for checking:

- layer ghosting
- row/column wiring
- RGB channel behavior
- power/load-related flicker
- multiplex timing
- bad or intermittent LEDs/connections

---

# Mode 5 – Breathing / Bouncing Ball

A 3D sphere begins near the center of the cube, expands outward until it fills much of the cube, then contracts again.

The effect is intended to resemble slow breathing or pulsing.

The edge of the sphere is softened so LEDs fade in and out rather than switching abruptly.

The sphere can also be made to move/bounce around the cube while it continues breathing.

### Controls

| Button | Function |
|---|---|
| `+` / `-` | Brightness |
| `FF` / `REW` | Breathing speed |
| `PLAY` | Toggle stationary / bouncing |
| `0` | Toggle hue rotation / manual RGB |
| `4` / `7` | Red up / down |
| `5` / `8` | Green up / down |
| `6` / `9` | Blue up / down |
| `POWER` | LEDs off / on |

Default behavior is a centered breathing sphere with smoothly rotating hues.

---

# Switching Modes

Use the remote's **MODE / TEST** button to cycle through the effects:

```text
Smooth Hue
    ↓
Cloud / Swirl
    ↓
Manual RGB
    ↓
Diagnostic
    ↓
Breathing Ball
    ↓
back to Smooth Hue
```

---

# Hardware

Developed for an AuraCube-style 8×8×8 RGB LED cube using:

- **STM32F103**
- 512 RGB LEDs
- 8 multiplexed layers
- original AuraCube shift-register / layer-driver hardware
- IR remote receiver
- CH340 USB serial interface on the cube's mini-USB connection

The project is programmed through **SWD using an ST-Link**.

> The mini-USB port on this board is a CH340 USB-to-serial/power connection. It is not an onboard ST-Link programmer.

---

# Building and Flashing

The project is intended for **STM32CubeIDE**.

Typical workflow:

1. Open STM32CubeIDE.
2. Select **File → Import**.
3. Select **General → Projects from Folder or Archive**.
4. Choose the project ZIP/archive.
5. Import the project.
6. Click **Build**.
7. Verify the build completes with **0 errors**.
8. Connect the ST-Link through SWD.
9. Start **Debug**.
10. After programming stops at `main()`, press **Resume / F8**.

Some warnings come from the original AuraCube source tree and do not prevent a successful build.

---

# Why Fixed-Point Math?

The STM32F103 does **not** have a hardware floating-point unit.

Earlier animation experiments used functions such as:

- `powf()`
- `fmodf()`
- `fabsf()`

and large amounts of floating-point math per frame.

That made effects slower and contributed to visible stepping/choppiness.

The custom effects in this project use integer/fixed-point calculations wherever practical, which substantially improves animation performance on this MCU.

---

# Display / Scan Improvements

During development several issues were found in earlier custom code:

- rendering directly into the buffer currently being displayed
- unnecessary floating-point calculations
- low effective brightness range
- hue values being truncated too early
- multiplex timing/ghosting becoming visible at high brightness

The current effects use double-buffered rendering and lighter-weight calculations.

A diagnostic scan mode was also added to investigate layer/row ghosting and full-white load behavior.

---

# Known Hardware Behavior

On the test cube, full white at high brightness can expose flicker in some upper layers.

Interesting observations during testing included:

- normal behavior at lower brightness
- good performance in saturated hue/cloud effects
- flicker becoming much easier to see on full white
- diagnostic blanking changes reducing some ghosting
- some phantom layer activity depending on which layer pair is displayed

Because normal animated effects look good, the firmware keeps conservative brightness limits in those modes while still allowing unrestricted brightness in Manual RGB and Diagnostic modes for testing.

Your cube may behave differently depending on:

- LED current
- power supply
- wiring
- layer-driver tolerances
- PCB revision
- cube construction

---

# Project Background

This firmware grew out of experimenting with the original AuraCube source code and trying to create smoother, more natural effects without replacing the working low-level display driver.

The original hardware is capable of very fast animation, so the focus became keeping that proven scan system while improving the way custom frames are generated.

The result is a mix of visual effects and diagnostic tools that can be used both for display and for troubleshooting a hand-built RGB cube.

---

# Contributions

Issues, testing results, improvements, and new effects are welcome.

If you have another AuraCube or a compatible STM32F103 RGB cube, it would be especially useful to compare:

- high-brightness white behavior
- upper-layer flicker
- ghosting
- power-supply sensitivity
- different PCB revisions

---

# License

No license is included automatically.

Before publishing or redistributing this project, review the licensing/copyright status of the **original AuraCube firmware** that this project is based on and choose an appropriate license for your own changes.
