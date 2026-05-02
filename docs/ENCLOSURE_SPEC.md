# RaceGate Hub — Enclosure Design Specification

## Prompt para modelador 3D / IA de diseño 3D

Copia este prompt en tu herramienta de modelado 3D (FreeCAD, Fusion360, TinkerCAD, o IA como Zoo.dev, Meshy, etc.):

---

### PROMPT

Design a two-piece 3D printable enclosure (front panel + rear box) for an FPV race timing display. The enclosure should look like a compact handheld race monitor.

#### PART 1: FRONT PANEL (faceplate)

**Screen cutout:**
- Rectangular window for visible display area: 49.5mm x 90mm (landscape orientation)
- The TFT module PCB is 55mm x 96mm
- The screen sits flush with the front panel surface
- 2 mounting holes for M3 screws, spaced 33mm x 90mm apart (matching PCB holes, ø3.2mm)
- The PCB is 7.6mm thick — the front panel should have a recessed lip (2mm deep) so the PCB sits flush

**Front panel dimensions:**
- Width: 104mm (PCB 96mm + 4mm margin each side)
- Height: 63mm (PCB 55mm + 4mm margin each side)
- Thickness: 3mm

**Snap/screw attachment:**
- 4x M2.5 screw posts on corners to attach to rear box
- Or snap-fit clips on edges (designer's choice)

#### PART 2: REAR BOX (enclosure body)

**Internal dimensions (minimum):**
- Width: 100mm
- Height: 59mm
- Depth: 40mm (enough for power bank + XIAO + cables + future components)

**Component layout (front to back):**
1. TFT screen PCB (7.6mm) — mounted to front panel
2. XIAO ESP32-S3 (21mm x 17.5mm x 8mm) — attached to screen via dupont cables
3. Power bank (78mm x 38mm x 25mm) — sits in the bottom of the rear box
4. Future space for: LiPo battery, TP4056 charger, MT3608 boost converter, DFPlayer Mini + speaker

**Openings and ports:**
- LEFT SIDE: Slot for TFT pin header (14 pins, ~35mm wide x 8mm tall) — for dupont cable routing
- BOTTOM or SIDE: USB-C port opening (9mm x 3.5mm) for XIAO programming/charging
- BOTTOM or SIDE: Second USB-C opening for power bank charging (if power bank has its own port)
- RIGHT SIDE: Future opening for SMA antenna connector (ø6.5mm hole)
- REAR: Ventilation grille (grid pattern, ~30mm x 20mm) for airflow (RX5808 gets hot)
- REAR: Speaker grille (circular, ø25mm) for future DFPlayer audio output

**Mounting features:**
- 4x M2.5 screw posts on corners matching front panel
- Internal standoffs/rails for power bank (so it doesn't move)
- Internal standoff for XIAO ESP32-S3 (M2 screw or clip)
- Cable routing channels between components

**Design requirements:**
- Wall thickness: 2mm minimum
- Print-friendly: no overhangs > 45°, or design with built-in supports
- Material: PLA or PETG
- Tolerance: 0.3mm for all fitments
- Rounded external corners (2mm radius) for comfortable handling
- Text embossed on rear: "RaceGate Hub" and "Verma FPV"

#### DIMENSIONS SUMMARY

```
Front panel: 104mm x 63mm x 3mm
Rear box:    104mm x 63mm x 40mm (external)
Total assembled: 104mm x 63mm x 43mm

Screen cutout: 49.5mm x 90mm (centered)
Screen PCB:    55mm x 96mm
Screen mounting holes: 2x ø3.2mm, spaced 33mm x 90mm

Power bank: 78mm x 38mm x 25mm
XIAO ESP32-S3: 21mm x 17.5mm x 8mm

USB-C port 1 (XIAO): 9mm x 3.5mm
USB-C port 2 (power bank): 9mm x 3.5mm
SMA antenna hole: ø6.5mm (future)
Vent grille: 30mm x 20mm (rear)
Speaker grille: ø25mm (rear)
```

#### VISUAL REFERENCE

```
FRONT VIEW (assembled):
┌──────────────────────────────────────┐
│  ┌──────────────────────────────┐    │
│  │                              │    │
│  │      SCREEN VISIBLE AREA     │    │
│  │        49.5 x 90 mm         │    │
│  │                              │    │
│  └──────────────────────────────┘    │
└──────────────────────────────────────┘

SIDE VIEW:
┌───┬──────────────────────────────────┐
│   │  front   │ XIAO │  power bank   │
│ 3 │  panel   │cables│  78x38x25     │
│mm │  + PCB   │      │               │
└───┴──────────────────────────────────┘
 ◄──────────── 43mm total ────────────►

REAR VIEW:
┌──────────────────────────────────────┐
│                                      │
│   ┌─────────┐      ╔═══════╗        │
│   │ VENT    │      ║SPEAKER║        │
│   │ GRILLE  │      ║GRILLE ║        │
│   └─────────┘      ╚═══════╝        │
│                                      │
│          RaceGate Hub                │
│           Verma FPV                  │
└──────────────────────────────────────┘
```

---

## Notas para impresión

- Imprimir front panel boca abajo (superficie lisa contra la cama)
- Imprimir rear box boca arriba (abertura hacia arriba)
- Layer height: 0.2mm
- Infill: 20-30%
- Material recomendado: PETG (más resistente al calor que PLA)
- Sin soportes si el diseño respeta los 45° de overhang
