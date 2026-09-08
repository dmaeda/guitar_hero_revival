# DIY Guitar Hero Drum Controller
## Build Guide, BOM, Design Notes, and Observations

This document describes a practical way to build a **Guitar Hero-style drum controller from scratch**, using the **Guitar Hero: World Tour** kit as the main mechanical and functional reference.

The recommended custom-controller architecture is:

- **3 main drum pads**
- **2 cymbals**
- **1 kick pedal**
- **Velocity-sensitive piezo sensors**
- **Raspberry Pi Pico / RP2040**
- **External multi-channel ADC such as the MCP3008**
- **USB output using suitable rhythm-game controller firmware such as Santroller**

The design is intended primarily for a fully custom controller. It is **not an exact electrical clone of the original World Tour PCB**.

---

# 1. Target Layout

A World Tour-style layout consists of:

```text
        YELLOW                       ORANGE
        Cymbal                        Cymbal
           O                             O


       RED              BLUE              GREEN
       Snare            Tom               Floor Tom
         O                O                   O


                         KICK
                          |
                        Pedal
```

Recommended inputs:

| Input | Type | Velocity Sensitive |
|---|---|---|
| Red pad | Piezo | Yes |
| Blue pad | Piezo | Yes |
| Green pad | Piezo | Yes |
| Yellow cymbal | Piezo | Yes |
| Orange cymbal | Piezo | Yes |
| Kick pedal | Piezo or digital switch | Optional |
| D-pad Up | Digital | No |
| D-pad Down | Digital | No |
| D-pad Left | Digital | No |
| D-pad Right | Digital | No |
| Start | Digital | No |
| Select / Back | Digital | No |
| Home / Guide | Digital | No |

---

# 2. Important Design Observations

## 2.1 The original World Tour pads use vibration sensors

The original Guitar Hero: World Tour drum pads and cymbals use **piezoelectric sensors** to detect hits.

Piezo sensors are appropriate because they generate a voltage pulse when mechanically flexed or vibrated.

This makes it possible to detect not only that a hit occurred, but also an approximation of **hit strength / velocity**.

---

## 2.2 The original World Tour kick pedal also uses a piezo sensor

This is an important difference from some Rock Band pedals.

The original World Tour kick pedal uses a **piezoelectric impact sensor** rather than a simple magnetic or mechanical switch.

Therefore:

- If the goal is to connect a custom pedal to an **original World Tour drum brain**, use a piezo-based pedal.
- If the entire controller is being built from scratch with an RP2040, a **reed switch or microswitch can be used instead** if velocity information is not required for the kick.

For a custom controller, a digital kick pedal is mechanically simpler and usually very reliable.

---

## 2.3 Do not connect a piezo directly to a microcontroller ADC

A piezoelectric disk can generate voltage spikes significantly higher than the ADC supply voltage.

A raw piezo should therefore **not** be connected directly to a 3.3 V RP2040 ADC input.

Use:

- series resistance;
- discharge resistance;
- voltage clamps;
- optional filtering.

A protected input circuit is described later in this document.

---

## 2.4 The RP2040 does not provide enough exposed ADC channels for five or six piezos

A typical Raspberry Pi Pico exposes only three general-purpose analog inputs.

A World Tour-style kit needs at least:

```text
3 drum pads
2 cymbals
----------------
5 analog channels
```

and possibly a sixth analog channel for a piezo kick pedal.

An external ADC such as the **MCP3008** is therefore convenient.

The MCP3008 provides:

- 8 analog input channels;
- 10-bit conversion;
- SPI interface;
- enough channels for the complete drum kit plus future expansion.

A 12-bit ADC such as the MCP3208 can also be used if greater amplitude resolution is desired.

---

# 3. Recommended Bill of Materials

## 3.1 Main Electronics

| Item | Qty. | Suggested Part | Notes |
|---|---:|---|---|
| Microcontroller | 1 | Raspberry Pi Pico / RP2040 board | Main USB controller |
| External ADC | 1 | MCP3008 | 8-channel SPI ADC |
| Piezo disks | 6 | 27 mm piezo disks | 5 required + 1 spare or kick |
| Series resistors | 6 | 10 kOhm, 1/4 W | Limits clamp current |
| Discharge resistors | 6 | 1 MOhm, 1/4 W | Pulls piezo input back to 0 V |
| Clamp diodes | 6 | BAT54S | Dual Schottky clamp per channel |
| Capacitors | 6 | 47 nF | Optional filtering |
| ADC decoupling capacitor | 1 | 100 nF ceramic | Close to MCP3008 |
| Bulk capacitor | 1 | 10 uF | Near ADC / 3.3 V rail |
| Panel jacks | 6 | 3.5 mm | For detachable pads/cymbals/pedal |
| Plugs/cables | 6 | 3.5 mm | Pad cables |
| Tactile switches | 7 | 6 x 6 mm or Omron B3F | D-pad + Start + Select + Home |
| USB cable | 1 | Appropriate for Pico board | Data-capable cable |
| Wire | As needed | 24-28 AWG stranded | Flexible wiring preferred |
| Perfboard or PCB | 1 | Custom | Main electronics |

### Optional kick-pedal parts

| Item | Qty. | Suggested Part | Notes |
|---|---:|---|---|
| Reed switch | 1 | Normally-open reed | Silent digital pedal option |
| Neodymium magnet | 1 | Small magnet | Used with reed switch |
| Microswitch | 1 | Omron SS-5GL or similar | Alternative digital pedal |
| Additional piezo | 1 | 27 mm | For original-WT-style pedal |

---

# 4. Mechanical BOM

## 4.1 Drum pads

Recommended for three drum pads:

| Item | Qty. |
|---|---:|
| Rigid pad base, approximately 200-220 mm diameter | 3 |
| Rubber / silicone striking surface | 3 |
| EVA, neoprene, or foam isolation layer | 3 |
| Piezo disk | 3 |
| Printed or fabricated pad housing | 3 |
| Pad mounting clamps | 3 |

A diameter around **210 mm** is a practical target for a compact Guitar Hero-style controller.

It does not have to exactly reproduce the original shell dimensions.

---

## 4.2 Cymbals

| Item | Qty. |
|---|---:|
| Rigid cymbal body | 2 |
| Rubber / neoprene striking surface | 2 |
| Piezo disk | 2 |
| Cymbal mounting arm | 2 |
| Clamp | 2 |
| Rubber isolation washers | Several |

The cymbal body can be made from:

- ABS;
- PETG;
- polycarbonate;
- nylon;
- a rigid printed shell.

Avoid mounting the cymbal completely rigidly to the rack. Mechanical isolation helps reduce crosstalk.

---

## 4.3 Rack

Suggested materials:

```text
25 mm aluminum tubing
```

or, for a lower-cost prototype:

```text
25 mm PVC tubing
```

Typical requirement:

- approximately 3-4 m of tubing;
- T-connectors;
- elbows;
- clamps;
- rubber feet;
- M4/M6 hardware.

For a permanent build, aluminum tubing is preferable.

For a prototype, PVC is inexpensive and easy to modify.

---

## 4.4 Kick pedal

Suggested mechanical components:

| Item | Qty. |
|---|---:|
| Pedal plate | 1 |
| Base plate | 1 |
| 6-8 mm shaft | 1 |
| 608 bearings | 2 |
| Return spring | 1 |
| Stop bumper | 1 |
| Reed switch + magnet, microswitch, or piezo | 1 |

Two 608 bearings make a good inexpensive pivot for a DIY pedal.

---

# 5. Piezo Sensor Construction

A basic drum pad stack can be built as follows:

```text
               Drum stick
                   |
                   v
        +----------------------+
        | Rubber / silicone    |
        +----------------------+
        | Rigid striking plate |
        +----------------------+
        | Foam / EVA transfer  |
        |        block         |
        |          |           |
        |       [PIEZO]        |
        |                      |
        +----------------------+
        | Pad housing          |
        +----------------------+
```

## Important

Do not allow the drumstick to strike the ceramic piezo disk directly.

The piezo should receive vibration through the pad structure.

This:

- protects the ceramic element;
- improves consistency;
- reduces extreme voltage spikes;
- makes sensitivity easier to tune.

---

# 6. Piezo Mounting Methods

Several mounting methods are possible.

## Method A - Directly attached to the pad

The piezo is bonded to the underside of the rigid striking plate.

Advantages:

- simple;
- high sensitivity.

Disadvantages:

- can produce very large signals;
- can increase crosstalk;
- adhesive consistency affects response.

---

## Method B - Foam transfer block

A small piece of foam transfers vibration from the striking plate to the piezo.

Advantages:

- easier sensitivity adjustment;
- protects the sensor;
- can reduce mechanical shock.

This is a good approach for a DIY design.

---

## Method C - Foam cone

Electronic drum triggers often use a foam cone between the striking surface and the piezo.

This can provide very consistent sensitivity but requires more mechanical tuning.

---

# 7. Piezo Input Protection Circuit

A practical single-channel input can be implemented approximately as follows:

```text
                     10 kOhm
 PIEZO + -------------/\/\/\-------+--------> ADC input
                                   |
                                   |
                                  1 MOhm
                                   |
                                   |
                                  GND

                                    +----|<|---- 3.3 V
                                    |
 ADC INPUT -------------------------+
                                    |
                                    +----|>|---- GND

 PIEZO - -------------------------------------- GND
```

Use a dual Schottky part such as **BAT54S** or equivalent to clamp the ADC node.

Recommended starting values:

```text
Series resistor:      10 kOhm
Discharge resistor:    1 MOhm
Optional capacitor:   47 nF
Clamp diodes:         Schottky
```

The series resistor limits the current that flows through the clamp diodes during large piezo spikes.

The 1 MOhm resistor allows the sensor node to discharge back toward zero.

---

# 8. About the Optional Capacitor

A capacitor can reduce high-frequency noise, but excessive capacitance can make the pulse too wide.

A reasonable experimental starting value is:

```text
47 nF
```

Useful range:

```text
10 nF to 100 nF
```

Treat this as a tuning component rather than a mandatory fixed value.

If the firmware samples the piezos correctly, the circuit may work well with little or no extra capacitance.

---

# 9. MCP3008 Connections

The MCP3008 can collect all drum sensor signals.

Suggested channel map:

```text
MCP3008

CH0 -> Red
CH1 -> Blue
CH2 -> Green
CH3 -> Yellow cymbal
CH4 -> Orange cymbal
CH5 -> Kick piezo (optional)
CH6 -> Expansion
CH7 -> Expansion
```

Suggested SPI connection to an RP2040:

```text
RP2040                  MCP3008
--------------------------------
3.3 V   --------------> VDD
3.3 V   --------------> VREF
GND     --------------> AGND
GND     --------------> DGND

GPIO SPI CLK ----------> CLK
GPIO SPI MOSI ---------> DIN
GPIO SPI MISO <--------- DOUT
GPIO CS  --------------> CS/SHDN
```

Use the same 3.3 V supply for the RP2040 and ADC unless there is a specific reason to do otherwise.

Place a **100 nF ceramic capacitor** close to the MCP3008 supply pins.

---

# 10. Example RP2040 Pin Assignment

The exact GPIO numbers are flexible.

Example:

```text
GP2  -> SPI SCK
GP3  -> SPI MOSI
GP4  -> SPI MISO
GP5  -> MCP3008 CS

GP6  -> D-pad Up
GP7  -> D-pad Down
GP8  -> D-pad Left
GP9  -> D-pad Right

GP10 -> Start
GP11 -> Select / Back
GP12 -> Home / Guide

GP13 -> Digital kick input (if reed/microswitch is used)
```

All digital button inputs can use internal pull-ups.

Typical wiring:

```text
GPIO ---- switch ---- GND
```

Firmware behavior:

```text
Released = HIGH
Pressed  = LOW
```

---

# 11. Kick Pedal Options

There are three practical approaches.

## Option A - Piezo pedal

Use this when reproducing the behavior of the original World Tour pedal.

```text
Pedal impact
    |
    v
Mechanical plate
    |
    v
Piezo disk
    |
    v
Protected analog input
```

Advantages:

- closest to original World Tour behavior;
- can measure impact strength;
- appropriate if interfacing with original WT electronics.

Disadvantages:

- mechanically more sensitive;
- piezo needs protection;
- requires analog processing.

---

## Option B - Reed switch + magnet

Recommended for a completely custom RP2040 controller if kick velocity is not needed.

```text
Pedal arm
   |
 [MAGNET]
     \
      \ movement
       \
      [REED SWITCH]
```

Advantages:

- silent;
- almost no mechanical wear;
- simple electronics;
- easy firmware handling.

---

## Option C - Microswitch

A lever microswitch such as an **Omron SS-5GL** can also be used.

Advantages:

- simple;
- inexpensive;
- positive tactile activation;
- easy adjustment.

Disadvantages:

- produces mechanical noise;
- eventually wears;
- geometry must prevent excessive overtravel.

---

# 12. D-pad and Menu Buttons

Recommended controls:

```text
D-pad Up
D-pad Down
D-pad Left
D-pad Right
Start
Select / Back
Home / Guide
```

Total:

```text
7 digital buttons
```

Generic 6 x 6 mm tactile switches are sufficient.

Higher-quality switches such as Omron B3F can be used if desired.

---

# 13. Firmware - Basic Hit Detection

A piezo signal looks approximately like this:

```text
ADC

4095 |
     |
3000 |             *
     |            ***
2000 |           *   *
     |          *     *
1000 |         *       *
     |________*_________*____________
          detection threshold
```

Do not simply generate a hit on every ADC sample above the threshold.

A better algorithm is:

1. Wait for a channel to exceed its threshold.
2. Open a short peak-detection window.
3. Measure the maximum ADC value during that window.
4. Convert the peak into a velocity value.
5. Generate the drum event.
6. Apply a short retrigger lockout.
7. Resume normal monitoring.

---

# 14. Example Hit Algorithm

Conceptual pseudocode:

```text
if adc_value > threshold and pad_is_ready:

    peak = adc_value

    for the next few milliseconds:
        sample ADC
        if adc_value > peak:
            peak = adc_value

    velocity = map_peak_to_velocity(peak)

    send_drum_hit(pad, velocity)

    start_retrigger_timer(pad)
```

A starting peak window might be:

```text
2-5 ms
```

A starting retrigger lockout might be:

```text
15-40 ms
```

These values must be tuned experimentally.

---

# 15. Velocity Mapping

A linear mapping is easy but usually does not feel ideal.

Example:

```text
ADC peak        MIDI-style velocity
-----------------------------------
  300                   10
  800                   35
 1500                   65
 2500                   95
 3800                  127
```

A configurable curve is better.

Recommended firmware parameters per pad:

```text
threshold
minimum velocity
maximum velocity
gain
velocity curve
peak window
retrigger time
crosstalk suppression
```

Each physical pad should be calibrated individually.

---

# 16. Crosstalk

Crosstalk occurs when hitting one pad causes another sensor to produce a smaller pulse.

Example:

```text
A strong RED hit produces:

RED      = 2700
BLUE     = 220
GREEN    = 140
YELLOW   = 90
ORANGE   = 60
```

Without filtering, the firmware may interpret this as multiple hits.

---

# 17. Crosstalk Reduction - Mechanical

Mechanical construction is the first line of defense.

Use:

- rubber washers;
- foam isolation;
- separate pad mounts;
- flexible cymbal mounts;
- non-rigid sensor mounting;
- independent cable routing.

Avoid making all pads part of one extremely rigid resonant plate.

---

# 18. Crosstalk Reduction - Firmware

A useful firmware strategy is:

1. Detect candidate hits occurring within a short time window.
2. Compare their peak amplitudes.
3. Treat the strongest channel as the probable real hit.
4. Reject much smaller simultaneous pulses on neighboring channels.

Example:

```text
RED   = 2600
BLUE  = 180

BLUE / RED = 0.069
```

If the crosstalk rejection ratio is configured to 15%, the blue pulse can safely be ignored.

Do not make the rejection too aggressive because legitimate simultaneous drum hits must still be possible.

---

# 19. Suggested Initial Calibration Values

These are only starting points.

```text
Peak detection window:    3 ms
Retrigger lockout:        25 ms
Crosstalk time window:     5 ms
Minimum velocity:          8
Maximum velocity:        127
```

Threshold must be measured experimentally for each sensor.

A useful calibration procedure is:

1. Measure idle ADC noise.
2. Tap the pad extremely softly.
3. Record the smallest legitimate hit.
4. Place the threshold above noise but below that soft-hit value.
5. Test hard hits for clipping.
6. Adjust mechanical coupling or gain as necessary.

---

# 20. Avoiding Double Triggering

Double triggering is normally caused by mechanical ringing.

A piezo may produce:

```text
MAIN HIT
   |
   |\
   | \
   |  \       secondary vibration
   |   \        /\
   |    \______/  \____
```

Potential solutions:

- increase retrigger lockout slightly;
- change foam stiffness;
- change piezo mounting;
- reduce structural resonance;
- add controlled damping;
- improve peak-detection logic.

Do not solve every double-trigger problem by simply increasing the threshold, because that can make soft hits impossible.

---

# 21. Pad Surface Materials

Good choices:

- silicone rubber;
- neoprene;
- mouse-pad rubber;
- EVA;
- purpose-made drum rubber.

A practical stack is:

```text
2-3 mm rubber
rigid plate
foam transfer element
piezo
housing
```

The ideal material should provide:

- reasonable rebound;
- low acoustic noise;
- durability;
- enough vibration transfer to trigger the piezo.

---

# 22. 3D Printing Recommendations

For structural clamps and stressed parts, prefer:

```text
PETG
ABS
ASA
Nylon
```

PLA can be used for prototypes and non-stressed housings.

Avoid PLA for clamps that remain under high continuous tension, especially in warm environments.

Threaded brass inserts are highly recommended for frequently disassembled parts.

Suggested hardware:

```text
M3 heat-set inserts
M3 screws for electronics/housings
M4-M6 bolts for mechanical mounts
nylon lock nuts
rubber washers
```

---

# 23. Cable and Connector Notes

Detachable pad cables greatly improve maintenance.

A convenient architecture is:

```text
RED PAD -------- 3.5 mm --------+
BLUE PAD ------- 3.5 mm --------+
GREEN PAD ------ 3.5 mm --------+
YELLOW --------- 3.5 mm --------+--> Controller enclosure
ORANGE --------- 3.5 mm --------+
KICK ----------- 3.5 mm --------+
```

Important:

The original World Tour hardware used 3.5 mm connections in parts of the kit, but plug/contact arrangements can vary by application.

For a fully custom controller, choose your own consistent connector pinout and document it.

For example:

```text
TIP    = sensor positive
SLEEVE = ground
```

Do not assume an original Guitar Hero accessory uses the same custom pinout without checking it first.

---

# 24. Grounding and Wiring

Recommended practices:

- use a common signal ground;
- keep piezo wiring away from noisy USB/power wiring;
- use twisted pairs for longer piezo runs;
- avoid large ground loops;
- put the ADC close to the sensor connectors when practical;
- place decoupling capacitors close to IC power pins.

For each piezo cable:

```text
Signal + Ground
```

can be lightly twisted together to reduce coupled electrical noise.

Note that much of the practical "crosstalk" in drum kits is mechanical vibration rather than electrical interference.

---

# 25. Power

A USB-connected RP2040 design can normally power the controller electronics directly.

Typical rails:

```text
USB 5 V
   |
   +--> RP2040 board
          |
          +--> regulated 3.3 V
                    |
                    +--> MCP3008
                    +--> input clamp reference
```

Keep all ADC-related signals within the ADC's permitted input range.

---

# 26. Firmware Platform

A custom implementation can be written directly using:

- Pico SDK;
- Arduino-Pico;
- TinyUSB;
- custom USB HID descriptors.

However, for rhythm-game controllers, **Santroller** is worth evaluating before writing the complete USB stack yourself.

Santroller is designed for microcontroller-based rhythm-game controllers and supports drum-controller use cases.

It can also simplify console-specific controller emulation and authentication workflows.

---

# 27. PC Compatibility

PC is the easiest target.

Possible output types include:

- USB HID game controller;
- rhythm-game-specific HID implementation;
- MIDI, if the software accepts MIDI drums.

For Clone Hero/YARG-style use, a controller firmware designed for rhythm games is generally preferable to inventing a custom host protocol.

---

# 28. Xbox 360 Compatibility

Xbox 360 compatibility requires special attention.

A generic RP2040 USB HID controller should **not** be assumed to work directly on a retail Xbox 360.

With modern Santroller setups:

- an RGH-modified Xbox 360 can use an appropriate software/plugin route;
- a retail Xbox 360 can require authentication through a compatible wired Xbox 360 controller.

Therefore, decide the target platform **before finalizing the USB architecture**.

If Xbox 360 support is important, reserve the required USB/GPIO resources and connector arrangement from the beginning.

---

# 29. Recommended Assembly Order

Do not build the complete rack before validating one pad.

Recommended sequence:

## Phase 1 - Electronics prototype

Build:

```text
RP2040
MCP3008
one protected piezo input
one piezo
```

Verify:

- ADC readings;
- soft hits;
- hard hits;
- no ADC overvoltage;
- peak detection;
- retrigger filtering.

---

## Phase 2 - One complete drum pad

Build one full mechanical pad.

Tune:

- piezo location;
- foam stiffness;
- rubber thickness;
- threshold;
- velocity curve.

Only after one pad behaves correctly should the other pads be duplicated.

---

## Phase 3 - Five analog channels

Add:

```text
Red
Blue
Green
Yellow
Orange
```

Test simultaneous hits and crosstalk.

---

## Phase 4 - Control panel

Add:

```text
D-pad
Start
Select
Home
```

Verify all digital inputs.

---

## Phase 5 - Kick pedal

Choose:

```text
piezo
reed switch
or microswitch
```

and validate fast repeated kicks.

---

## Phase 6 - Final rack

Only after the electronics and trigger behavior are stable should the final rack geometry be completed.

This avoids repeatedly rebuilding the frame while debugging trigger sensitivity.

---

# 30. Bring-Up Checklist

Before connecting everything:

- [ ] Check 3.3 V rail with a multimeter.
- [ ] Verify ADC ground and RP2040 ground are common.
- [ ] Verify no ADC input exceeds the intended range.
- [ ] Test one piezo channel first.
- [ ] Confirm idle ADC value is stable.
- [ ] Test soft hits.
- [ ] Test hard hits.
- [ ] Check for ADC clipping.
- [ ] Test retrigger behavior.
- [ ] Test simultaneous two-pad hits.
- [ ] Test crosstalk.
- [ ] Test cymbal isolation.
- [ ] Test rapid kick operation.
- [ ] Test USB controller recognition.
- [ ] Calibrate every pad individually.

---

# 31. Common Problems

## Missed soft hits

Possible causes:

- threshold too high;
- piezo poorly coupled to striking surface;
- foam too soft;
- piezo located too far from impact zone;
- signal filtering too aggressive.

---

## Hard hits all produce the same velocity

Possible causes:

- ADC clipping;
- sensor mechanically overloaded;
- gain too high;
- piezo coupled too strongly.

Solutions:

- change mechanical coupling;
- increase series/input attenuation if required;
- adjust the velocity curve.

---

## Double triggers

Possible causes:

- pad ringing;
- retrigger time too short;
- overly rigid structure;
- insufficient damping.

---

## Other pads trigger when one pad is hit

This is crosstalk.

First improve mechanical isolation.

Then tune firmware rejection.

---

## Cymbals are too insensitive

Try:

- moving the piezo;
- changing foam coupling;
- reducing rubber thickness;
- lowering the threshold.

---

## Cymbals trigger when the rack moves

Try:

- softer mounting washers;
- more independent cymbal mounts;
- different sensor location;
- higher cymbal threshold;
- crosstalk filtering.

---

# 32. Recommended Final Architecture

For a first complete DIY build, the recommended configuration is:

```text
                    USB
                     |
              +-------------+
              |    RP2040   |
              |             |
              | Digital I/O |---- D-pad
              |             |---- Start
              |             |---- Select
              |             |---- Home
              |             |---- Digital kick (optional)
              |             |
              |     SPI     |
              +------|------+
                     |
               +-----v------+
               |  MCP3008   |
               |            |
          CH0 -| Red        |
          CH1 -| Blue       |
          CH2 -| Green      |
          CH3 -| Yellow     |
          CH4 -| Orange     |
          CH5 -| Kick piezo |
          CH6 -| Expansion  |
          CH7 -| Expansion  |
               +------------+
```

This architecture is inexpensive, repairable, expandable, and easy to prototype.

---

# 33. Recommended BOM Summary

```text
ELECTRONICS
-----------

1 x Raspberry Pi Pico / RP2040
1 x MCP3008

6 x 27 mm piezo disks
6 x 10 kOhm resistors
6 x 1 MOhm resistors
6 x BAT54S dual Schottky diodes
6 x 47 nF capacitors (optional)

1 x 100 nF ADC decoupling capacitor
1 x 10 uF bulk capacitor

6 x 3.5 mm panel jacks
6 x matching plugs/cables

7 x tactile switches
1 x USB data cable

Optional digital kick:
1 x reed switch
1 x neodymium magnet

or

1 x Omron SS-5GL-style microswitch
```

```text
MECHANICAL
----------

3 x approximately 210 mm drum pads
2 x cymbal bodies
1 x pedal assembly

Rubber / silicone striking material
EVA / neoprene / foam
Approximately 3-4 m of 25 mm rack tubing

2 x cymbal arms
3 x pad mounts
2 x cymbal clamps

2 x 608 bearings
1 x 6-8 mm pedal shaft
1 x pedal return spring

M3/M4/M6 hardware
Nylon lock nuts
Rubber washers
Threaded inserts
```

---

# 34. Useful Future Upgrades

After the basic kit works reliably, possible upgrades include:

- dual-zone cymbals;
- additional cymbal;
- second kick pedal;
- 12-bit ADC such as MCP3208;
- adjustable analog gain;
- per-channel status LEDs;
- OLED calibration display;
- USB-C controller enclosure;
- detachable main harness;
- MIDI output;
- automatic calibration mode;
- stored sensitivity profiles;
- wireless control panel;
- metal rack.

Do not add these until basic trigger performance is stable.

---

# 35. Final Recommendations

For the first prototype:

1. Use **27 mm piezos**.
2. Use an **RP2040/Pico**.
3. Use an **MCP3008** for the analog channels.
4. Protect every piezo input.
5. Build **one pad first**.
6. Implement peak detection rather than simple threshold detection.
7. Add retrigger suppression.
8. Add crosstalk handling.
9. Mechanically isolate cymbals and pads.
10. Calibrate every trigger individually.
11. Use a **reed switch** for the simplest custom kick pedal.
12. Use a **piezo kick** if compatibility with original World Tour-style electronics is required.
13. Decide whether **Xbox 360 support** is required before finalizing the controller/USB design.

The most difficult part of this project is usually **mechanical trigger consistency and crosstalk**, not the USB microcontroller electronics.

---

# 36. Reference Notes

The following resources are useful background references for the design decisions in this guide:

- **Santroller project**  
  https://github.com/Santroller/Santroller

- **Santroller Configurator**  
  https://github.com/mat1jaczyyy/SantrollerConfigurator

- **Guitar Hero: World Tour drum sensor repair discussion**  
  https://gamefaqs.gamespot.com/boards/944203-guitar-hero-world-tour/46286601

- **World Tour kick pedal piezo discussion / modification**  
  https://www.criticalhit.net/gaming/guitar-hero-world-tour-rocking-pedal-mod/

- **World Tour drum crosstalk / internal piezo observations**  
  https://gamefaqs.gamespot.com/boards/944201-guitar-hero-world-tour/46198688

- **Historical World Tour drum-kit velocity sensitivity description**  
  Search for period reviews and demonstrations of the Guitar Hero: World Tour drum kit; the original pads were designed to report velocity-sensitive hits.

---

## Project Status

This document is intended as a **design specification and starting point**.

Before producing a custom PCB, prototype at least one complete piezo input and one complete mechanical pad on a breadboard or perfboard and record:

```text
idle ADC value
soft-hit peak
normal-hit peak
hard-hit peak
ringing duration
crosstalk amplitude
```

Those measurements should determine the final component values and firmware thresholds.
