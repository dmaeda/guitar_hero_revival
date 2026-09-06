# DIY Guitar Hero Guitar Controller
## Build Guide, BOM, Mechanical Design, Wiring, Firmware, and Design Notes

This document describes a practical way to build a **five-fret Guitar Hero-style guitar controller from scratch**.

The recommended baseline is:

- 5 fret buttons
- 2 strum directions
- analog whammy bar
- tilt / Star Power sensor
- D-pad
- Start / Select / Home buttons
- Raspberry Pi Pico / RP2040
- wired USB
- rhythm-game controller firmware such as Santroller

The **Guitar Hero: World Tour** guitar is used as an important mechanical reference, especially for its lever-microswitch strum mechanism.

This is a custom-controller design guide, not an exact electrical clone of every original Guitar Hero revision.


---

# 1. Control Layout

```text
NECK

[ GREEN ][ RED ][ YELLOW ][ BLUE ][ ORANGE ]


BODY

                 STRUM UP
                    ^
                    |
                [ STRUM ]
                    |
                    v
                STRUM DOWN

        D-PAD        START / SELECT

               HOME / GUIDE

                   WHAMMY
                      \
                       \____
```

Recommended inputs:

| Input | Type |
|---|---|
| Green fret | Digital |
| Red fret | Digital |
| Yellow fret | Digital |
| Blue fret | Digital |
| Orange fret | Digital |
| Strum Up | Digital |
| Strum Down | Digital |
| D-pad Up/Down/Left/Right | Digital |
| Start | Digital |
| Select / Back | Digital |
| Home / Guide | Digital |
| Tilt / Star Power | Digital or IMU |
| Whammy | Analog |

A full implementation needs approximately **14 digital inputs and 1 analog input**.


---

# 2. Recommended BOM

## Main electronics

| Item | Qty. | Suggested Part | Notes |
|---|---:|---|---|
| Microcontroller | 1 | Raspberry Pi Pico / RP2040 | Main controller |
| Fret switches | 5 | Kailh Choc V1 Red | Recommended modern fret switch |
| Strum switches | 2 | Kailh BOX Navy | Modern strum option |
| Alternative strum switches | 2 | KW11-3Z-compatible lever microswitch | World Tour-style option |
| Menu buttons | 3 | 6 x 6 mm tactile / Omron B3F | Start, Select, Home |
| D-pad buttons | 4 | 6 x 6 mm tactile / Omron B3F | Up, Down, Left, Right |
| Whammy sensor | 1 | B10K 10 kOhm linear potentiometer | Analog input |
| Tilt sensor | 1-2 | SW-520D | Simple Star Power sensing |
| Optional IMU | 1 | MPU6050 / LSM6DS3 | Advanced tilt |
| Neck connector | 1 | 8-pin JST-PH/JST-XH or similar | Detachable neck |
| Strum connector | 1 | 4-pin JST | Serviceable subassembly |
| Whammy connector | 1 | 3-pin JST | 3.3 V, ADC, GND |
| Wire | As needed | 26-28 AWG stranded | Flexible |
| Main PCB / perfboard | 1 | Custom | Controller board |
| Fret PCB | 1 | Custom | Highly recommended |
| Optional strum PCB | 1 | Custom | Recommended |
| USB cable | 1 | Data-capable | Board-dependent |
| 100 nF capacitors | Several | Ceramic | Decoupling |

## Mechanical parts

| Item | Qty. |
|---|---:|
| Body front/rear shell | 1 set |
| Neck front/rear shell | 1 set |
| Fret caps | 5 |
| Strum bar | 1 |
| 3-5 mm metal strum pivot shaft | 1 |
| Pivot bushings/bearings | 2 |
| Rubber/TPU strum stops | 2 |
| Whammy bar | 1 |
| Whammy pivot | 1 |
| Whammy return spring | 1 |
| Whammy linkage/cam | 1 |
| Strap buttons | 2 |
| M3 screws | Several |
| M3 heat-set inserts | Several |
| M4/M5 pivot hardware | As needed |


---

# 3. Recommended Architecture

```text
                         USB
                          |
                   +-------------+
                   |    RP2040   |
                   |             |
GREEN  ------------| GPIO        |
RED    ------------| GPIO        |
YELLOW ------------| GPIO        |
BLUE   ------------| GPIO        |
ORANGE ------------| GPIO        |
                   |             |
STRUM UP ----------| GPIO        |
STRUM DOWN --------| GPIO        |
                   |             |
D-PAD -------------| GPIOs       |
START -------------| GPIO        |
SELECT ------------| GPIO        |
HOME --------------| GPIO        |
TILT --------------| GPIO / I2C  |
                   |             |
WHAMMY ------------| ADC         |
                   +-------------+
```

No external ADC is normally required because only the whammy needs an analog input.


---

# 4. Fret Switches

## Recommended modern choice: Kailh Choc V1 Red

Low-profile Choc switches are well suited to a thin guitar neck.

Advantages:

- low profile;
- short travel;
- linear feel;
- compact;
- easy to mount on a custom PCB;
- replaceable.

Suggested layout:

```text
[ GREEN ][ RED ][ YELLOW ][ BLUE ][ ORANGE ]
    |        |       |       |        |
  CHOC     CHOC    CHOC    CHOC     CHOC
   RED      RED     RED     RED      RED
```

The player should press a **fret cap**, not the switch stem directly.

```text
Finger
  |
  v
+-----------+
| Fret cap  |
+-----------+
      |
   actuator
      |
+-----------+
| Choc Red  |
+-----------+
      |
+-----------+
| Fret PCB  |
+-----------+
```

Important mechanical goals:

- minimal sideways movement;
- no binding;
- equal travel on all five frets;
- little or no lateral force on the switch stem;
- consistent preload;
- enough clearance for rapid sliding between frets.

Useful starting dimensions:

```text
Fret cap width: roughly 18-22 mm
Gap:            roughly 1-3 mm
Visible travel: roughly 1-3 mm
```

Measure the actual switches before finalizing CAD.


---

# 5. Fret PCB and Neck Wiring

Use one GPIO per fret.

```text
GREEN  ---- switch ---- GND
RED    ---- switch ---- GND
YELLOW ---- switch ---- GND
BLUE   ---- switch ---- GND
ORANGE ---- switch ---- GND
```

Enable RP2040 internal pull-ups:

```text
Released = HIGH
Pressed  = LOW
```

A minimum neck harness needs:

```text
GND
GREEN
RED
YELLOW
BLUE
ORANGE
```

An 8-pin connector is preferable:

```text
1 GND
2 Green
3 Red
4 Yellow
5 Blue
6 Orange
7 Spare
8 Spare
```

A simple wired JST connector is recommended for the first revision. Pogo pins can be added later for a tool-less detachable neck.


---

# 6. Strum Bar

The strum mechanism is the most mechanically critical part of the guitar.

```text
               UP switch
                  ^
                  |
            +-----+-----+
            |           |
            | STRUM BAR |
            |           |
            +-----O-----+
                  ^
             center pivot
                  |
                  v
              DOWN switch
```

The bar should:

- return cleanly to center;
- activate only one direction at a time;
- have low sideways play;
- provide a clear activation point;
- survive very rapid repeated operation;
- avoid excessive switch overtravel.


---

# 7. Modern Strum Option: Kailh BOX Navy

A strong modern DIY choice is:

```text
2 x Kailh BOX Navy
```

Advantages:

- pronounced click;
- compact;
- PCB-friendly;
- consistent;
- replaceable;
- good for rapid alternate strumming.

```text
                  UP
                   |
             [BOX NAVY]
                   ^
                   |
        +----------+----------+
        |      STRUM BAR      |
        +----------O----------+
                   |
                   v
             [BOX NAVY]
                   |
                 DOWN
```

The geometry must activate each switch before the bar reaches its physical end stop.


---

# 8. World Tour-Style Strum Option

The Guitar Hero: World Tour guitar uses a **lever-microswitch-style strum mechanism**, unlike several other classic Guitar Hero guitars that use different strum-switch formats.

A commonly used compatible replacement family is:

```text
KW11-3Z-style SPDT lever microswitch
```

Typical terminals:

```text
COM
NO
NC
```

For a custom RP2040 controller:

```text
COM -> GND
NO  -> GPIO
NC  -> unused
```

with an internal pull-up enabled.

Important note:

**KW11-3Z is widely used as a compatible World Tour replacement family. Do not treat that code as proof that every original World Tour production revision used an OEM switch carrying that exact part number.**


---

# 9. World Tour-Style Strum Geometry

```text
             Microswitch UP
             +-----------+
             |           |
             +-----^-----+
                   |
        +----------+----------+
        |      STRUM BAR      |
        +----------O----------+
                   |
             +-----v-----+
             |           |
             +-----------+
            Microswitch DOWN
```

Lever microswitches tolerate mechanical alignment variation well.

Do not let the microswitch itself become the hard travel stop.

Use a separate:

- rubber bumper;
- TPU stop;
- silicone pad;
- adjustable stop screw.


---

# 10. BOX Navy vs. Lever Microswitch

| Characteristic | Kailh BOX Navy | KW11-3Z-Style Lever Microswitch |
|---|---|---|
| Feel | Strong keyboard-style click | Classic lever click |
| World Tour-like | No | Yes |
| PCB mounting | Excellent | Good |
| Alignment tolerance | Moderate | High |
| Compactness | Excellent | Good |
| Easy replacement | Excellent | Excellent |
| Best use | Modern custom guitar | WT-style strum |

Choose **BOX Navy** for a modern custom design.

Choose a **lever microswitch** if the objective is to reproduce the World Tour strum feel.


---

# 11. Strum Pivot and Centering

Use a metal shaft if possible:

```text
3-5 mm steel or aluminum shaft
```

A durable pivot:

```text
shell
 |
bushing
 |
metal shaft
 |
strum bar
 |
metal shaft
 |
bushing
 |
shell
```

Possible centering systems:

1. switch force;
2. torsion spring;
3. silicone/rubber elastomer.

For a high-quality design, the switches should detect input while separate bumpers handle the end-stop impact.

Avoid printed-plastic-on-printed-plastic pivots for long-term heavy use when a simple shaft/bushing solution is possible.


---

# 12. Whammy Bar

Use:

```text
B10K 10 kOhm linear potentiometer
```

Electrical connection:

```text
3.3 V -------- potentiometer end
GND ---------- potentiometer end
WIPER -------- RP2040 ADC
```

Optional:

```text
WIPER -> 1 kOhm -> ADC
ADC -> 10-100 nF -> GND
```

The potentiometer is safe for the ADC when powered from the same 3.3 V rail.

Do not use the potentiometer shaft as the mechanical return spring.


---

# 13. Whammy Mechanics

Convert whammy movement into potentiometer rotation using:

- linkage;
- cam;
- eccentric arm;
- small gear.

```text
Whammy bar
     \
      \
       O  <- pivot
       |
       +---- linkage ---- potentiometer
```

Use a dedicated return spring:

- torsion spring;
- extension spring;
- compression spring;
- elastomer.

The mechanism should return reliably to the same rest point.


---

# 14. Whammy Calibration

Do not assume the mechanism uses the full ADC range.

Example:

```text
Rest:          ADC 780
Fully pressed: ADC 3150
```

Firmware maps the measured range:

```text
780  -> 0%
3150 -> 100%
```

Add a small dead zone around the rest point.

Example:

```text
780-830 -> 0% whammy
```

A light software low-pass filter can reduce jitter, but excessive filtering makes the whammy sluggish.


---

# 15. Tilt / Star Power

## Simple option: SW-520D

```text
GPIO ---- SW-520D ---- GND
```

Use the internal pull-up.

Advantages:

- cheap;
- simple;
- no ADC;
- easy to replace.

Disadvantages:

- contact bounce;
- mounting-angle sensitivity;
- less precise activation.

One or two sensors can be used.

Using two sensors at slightly different angles can make activation less dependent on exact guitar orientation.


---

# 16. Accelerometer Option

For more precise tilt detection use an IMU such as:

```text
MPU6050
LSM6DS3
```

via I2C.

Advantages:

- configurable activation angle;
- hysteresis;
- no mechanical contact bounce;
- more consistent Star Power behavior.

Example:

```text
Activate:   55 degrees
Deactivate: 40 degrees
```

The different thresholds create hysteresis and prevent rapid toggling near the trigger angle.


---

# 17. Start, Select, Home, and D-Pad

Use ordinary tactile switches:

```text
6 x 6 mm tactile
```

or a higher-quality equivalent such as Omron B3F.

Required menu controls:

```text
Start
Select / Back
Home / Guide
```

D-pad:

```text
          UP
           O

LEFT   O   +   O   RIGHT

           O
         DOWN
```

Wire every digital control as:

```text
GPIO ---- switch ---- GND
```

with internal pull-up enabled.


---

# 18. Example RP2040 Pin Assignment

```text
GP2  -> Green
GP3  -> Red
GP4  -> Yellow
GP5  -> Blue
GP6  -> Orange

GP7  -> Strum Up
GP8  -> Strum Down

GP9  -> D-pad Up
GP10 -> D-pad Down
GP11 -> D-pad Left
GP12 -> D-pad Right

GP13 -> Start
GP14 -> Select / Back
GP15 -> Home / Guide

GP16 -> Tilt

GP26 / ADC0 -> Whammy
```

If using an I2C IMU, move tilt to an I2C pair such as:

```text
GP16 -> SDA
GP17 -> SCL
```

and relocate any conflicting digital input.


---

# 19. Debouncing

Mechanical switches bounce.

Suggested starting software debounce values:

```text
Frets:  1-3 ms
Strum:  1-2 ms
Menu:   5-15 ms
Tilt:   10-30 ms
```

These are only starting points.

The strum debounce value is especially important:

- too long -> missed fast alternate strums;
- too short -> possible double strums.

Fix mechanical rebound first, then apply the minimum firmware filtering required.


---

# 20. Firmware Architecture

The input loop should remain non-blocking:

```text
main loop
  |
  +-> read frets
  +-> read strum
  +-> read D-pad/menu
  +-> read tilt
  +-> sample whammy
  +-> update controller report
```

Avoid blocking `delay()` calls in the gameplay input path.

Use timers and state machines for debounce and tilt timing.


---

# 21. Firmware Diagnostics

A diagnostic mode should report:

```text
Green:      0/1
Red:        0/1
Yellow:     0/1
Blue:       0/1
Orange:     0/1

Strum Up:   0/1
Strum Down: 0/1

Whammy:     raw ADC
Tilt:       0/1

D-pad
Start
Select
Home
```

Verify every control outside the game before diagnosing game compatibility.


---

# 22. Firmware Platform

Possible implementation stacks include:

- Pico SDK;
- Arduino-Pico;
- TinyUSB;
- custom USB HID.

For a rhythm-game controller, evaluate **Santroller** before implementing the full USB/controller stack yourself.

Santroller is designed to program supported microcontrollers to emulate multiple rhythm-game controller types on PC and consoles.


---

# 23. PC and Console Compatibility

## PC

PC is the easiest first target.

Test the controller first with:

- operating-system controller diagnostics;
- Clone Hero;
- YARG;
- emulator/controller tools.

## Xbox 360

Do not assume that generic USB HID firmware will work directly on Xbox 360.

Use a controller firmware/mode that explicitly supports the console.

Current Santroller documentation lists Guitar Hero guitar support for Xbox 360 for relevant Guitar Hero titles.

## Newer Xbox consoles

Xbox One / Series compatibility is not identical to Xbox 360 compatibility.

Authentication requirements and game compatibility can differ.

Verify the current Santroller compatibility documentation before finalizing console-specific hardware.


---

# 24. Body and Neck Construction

Recommended 3D-print materials:

```text
PETG
ABS
ASA
```

PLA is fine for prototypes and lightly stressed covers, but it is less suitable for parts held under continuous mechanical stress.

Use M3 heat-set inserts where the shell will be opened repeatedly.

A long neck benefits from reinforcement such as:

- aluminum flat bar;
- steel rod;
- carbon-fiber rod;
- strong internal ribs.

Example:

```text
+----------------------------------+
| neck shell                       |
|                                  |
| [aluminum reinforcement bar]     |
|                                  |
| fret PCB                         |
+----------------------------------+
```


---

# 25. Ergonomics and Weight

Prototype these before final body production:

- fret reach;
- neck angle;
- strum location;
- whammy location;
- Start/Select location;
- strap-button position;
- body thickness;
- balance.

A cardboard or foam-board body mock-up is useful.

Avoid making the neck unnecessarily heavy.

Keep heavier components close to the body center so the guitar does not become neck-heavy when worn with a strap.


---

# 26. Connector Strategy

Modular internal wiring is strongly recommended.

```text
NECK
  |
  +---- 8-pin JST ---- MAIN PCB

STRUM PCB
  |
  +---- 4-pin JST ---- MAIN PCB

WHAMMY
  |
  +---- 3-pin JST ---- MAIN PCB

BUTTON PCB
  |
  +---- multi-pin JST ---- MAIN PCB

TILT / IMU
  |
  +---- 2-pin or 4-pin JST ---- MAIN PCB
```

Useful main-board test points:

```text
3.3 V
GND
Green
Red
Yellow
Blue
Orange
Strum Up
Strum Down
Whammy ADC
Tilt
```

Strain-relieve all wires near moving parts.


---

# 27. Build Sequence

## Phase 1 - Breadboard electronics

Connect:

```text
RP2040
one fret switch
one strum switch
one B10K potentiometer
one tilt sensor
```

Verify input logic and USB behavior.

## Phase 2 - Fret mechanism

Build a temporary five-fret rail.

Test:

- travel;
- return;
- simultaneous frets;
- slides;
- no binding.

## Phase 3 - Strum mechanism

Prototype it independently.

Test:

- up-strum;
- down-strum;
- rapid alternate strumming;
- centering;
- overtravel;
- double triggering;
- mechanical noise.

## Phase 4 - Whammy

Test:

- pivot;
- return spring;
- ADC range;
- dead zone;
- smooth response.

## Phase 5 - PCB and harness

Only after switch choices are final.

## Phase 6 - Full body

Test ergonomics, balance, cable routing, and service access.

## Phase 7 - Final enclosure

Produce cosmetic parts only after mechanics are stable.


---

# 28. Bring-Up Checklist

- [ ] Verify 3.3 V rail.
- [ ] Verify common ground.
- [ ] Test each fret individually.
- [ ] Test all fret combinations.
- [ ] Test fast fret transitions.
- [ ] Test strum up.
- [ ] Test strum down.
- [ ] Test rapid alternate strumming.
- [ ] Check for double strums.
- [ ] Check for missed strums.
- [ ] Verify strum returns to center.
- [ ] Verify physical end stops protect switches.
- [ ] Read raw whammy ADC.
- [ ] Calibrate whammy rest point.
- [ ] Calibrate full whammy travel.
- [ ] Verify spring return.
- [ ] Test tilt.
- [ ] Test Start / Select / Home.
- [ ] Test all D-pad directions.
- [ ] Test USB recognition on PC.
- [ ] Test in controller diagnostics.
- [ ] Test in target rhythm game.
- [ ] Test console-specific mode only after PC validation.


---

# 29. Common Problems

## Fret feels stiff

Possible causes:

- switch too heavy;
- excessive actuator preload;
- fret cap rubbing;
- neck shell flex;
- switch not centered.

## Fret sticks down

Possible causes:

- guide tolerance too tight;
- warped print;
- off-center switch;
- excessive sideways force.

## Strum double-triggers

Possible causes:

- contact bounce;
- mechanical rebound;
- loose pivot;
- insufficient damping;
- debounce too short.

Fix mechanics first.

## Strum misses fast input

Possible causes:

- debounce too long;
- switch not fully actuating;
- too much travel;
- slow/blocking firmware.

## Strum feels loose

Possible causes:

- oversized pivot bore;
- worn bushing;
- weak centering;
- flexible strum bar.

## Whammy signal jitters

Possible causes:

- poor potentiometer;
- mechanical linkage movement;
- long noisy wiring;
- ADC noise.

## Whammy does not fully return

Possible causes:

- weak spring;
- linkage friction;
- pivot misalignment;
- potentiometer carrying mechanical load.

## Tilt triggers randomly

Possible causes:

- bad sensor angle;
- contact bounce;
- aggressive motion;
- insufficient debounce.

Consider an IMU for more deterministic behavior.


---

# 30. Optional Upgrades

After the base controller is reliable:

- pogo-pin detachable neck;
- RGB fret lighting;
- touch slider;
- IMU tilt sensing;
- bearing-supported strum pivot;
- quieter TPU strum stops;
- USB-C;
- custom one-piece PCB;
- internal OLED diagnostic display;
- console mode selector;
- wireless operation.

Do not add these features until the basic fret, strum, and whammy systems are stable.


---

# 31. Recommended First Revision

For a modern first build:

```text
1 x RP2040
5 x Kailh Choc V1 Red
2 x Kailh BOX Navy
1 x B10K 10 kOhm linear potentiometer
1-2 x SW-520D
7 x tactile switches
wired USB
```

For a World Tour-style strum feel, replace:

```text
2 x Kailh BOX Navy
```

with:

```text
2 x KW11-3Z-compatible lever microswitches
```

and redesign the strum geometry around lever actuation.


---

# 32. Design Priorities

Recommended priority order:

```text
1. Strum consistency
2. Fret travel and alignment
3. Low input latency
4. Whammy reliability
5. Mechanical durability
6. Serviceability
7. Console compatibility
8. Cosmetic finish
```

The strum bar and fret system determine most of the controller's gameplay quality.


---

# 33. Original World Tour Service Note

For servicing an original Guitar Hero: World Tour guitar, a documented disassembly guide uses:

```text
Torx T10
Phillips #0
Torx T6 on some Xbox 360 internal screws
```

The screw arrangement varies between console versions/revisions.

Do not force a driver that does not fit correctly.


---

# 34. Final Recommendations

1. Use **RP2040** for the first wired custom build.
2. Use one GPIO per digital gameplay control.
3. Use **Kailh Choc V1 Red** for low-profile frets.
4. Use **Kailh BOX Navy** for a modern strum.
5. Use **KW11-3Z-compatible lever microswitches** for World Tour-style strum feel.
6. Use a **B10K 10 kOhm linear potentiometer** for whammy.
7. Use **SW-520D** for the simplest tilt implementation.
8. Use an IMU if precise Star Power angle control is desired.
9. Use separate replaceable fret and strum PCBs.
10. Use physical strum stops so switches do not absorb the full impact.
11. Prototype the strum assembly before finalizing body CAD.
12. Calibrate whammy range in firmware.
13. Keep gameplay debounce short.
14. Build the first revision as wired USB.
15. Verify current console support before finalizing console-specific hardware.


---

# 35. Reference Notes

## Santroller

Official documentation:

https://santroller.com/

GitHub:

https://github.com/Santroller/Santroller

Santroller is a platform for programming supported microcontrollers to emulate rhythm-game controllers for computers and multiple consoles.

## Guitar Hero: World Tour Strum Disassembly

iFixit:

https://www.ifixit.com/Guide/Guitar+Hero+World+Tour+Guitar+Strummer+Replacement/145473

The guide documents the World Tour body disassembly and strum assembly and notes differences between PS3 and Xbox 360 internals.

## World Tour Replacement Microswitch

Community repair reports commonly identify the **KW11-3Z** family as a compatible replacement for the three-terminal World Tour lever-microswitch strum mechanism.

Treat this as a compatibility reference rather than proof of a universal OEM part number.


---

# 36. Measurements to Record Before Final PCB/CAD

Before committing to the final design, record:

```text
fret actuation travel
fret release travel
fret cap clearance
strum actuation angle
strum overtravel
strum return force
strum bounce behavior
pivot free play
whammy rest ADC value
whammy full ADC value
whammy mechanical travel
tilt activation angle
USB input behavior
```

Use those measurements to finalize:

```text
CAD dimensions
switch placement
spring force
debounce values
firmware calibration
PCB connector locations
```

Design around measured mechanical behavior rather than nominal component dimensions alone.

