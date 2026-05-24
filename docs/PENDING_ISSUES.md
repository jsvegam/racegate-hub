# Pending Issues — RaceGate Hub

## RESOLVED: FPVGate webhooks don't work in AP mode

**Date**: 2026-05-24
**Status**: ✅ Resolved (2026-05-25)

### Problem

FPVGate doesn't send HTTP webhooks when operating as Access Point (AP). The webhook code checks:

```cpp
if (WiFi.status() != WL_CONNECTED) {
    return;  // Never sends in AP mode
}
```

`WiFi.status() == WL_CONNECTED` is only `true` when the ESP32 is connected as a **client (STA)** to another WiFi network. When FPVGate IS the AP, this condition is always `false`.

### Resolution

Use an external WiFi network (home router or phone hotspot). All devices connect as STA clients to the same network. FPVGate has `WL_CONNECTED == true` and sends webhooks normally.

This is the scenario FPVGate was designed for (multi-node with central router).

See [docs/NETWORK_ARCHITECTURE.md](NETWORK_ARCHITECTURE.md) for full explanation.

### Verified Working (2026-05-25)

- ✅ Display connects via WiFi Manager (captive portal)
- ✅ FPVGate connects to same network in STA mode
- ✅ Webhooks arrive at display (POST /Lap, /RaceStart, /RaceStop)
- ✅ Lap times display correctly in single-pilot mode
- ✅ Smart mode detection (single vs multi pilot)

---

## OPEN: Pilot names not transmitted in webhooks

**Date**: 2026-05-25
**Status**: Open (cosmetic)

### Problem

FPVGate sends empty POST bodies for webhooks. No pilot name, channel, or metadata is included. The display currently generates names from the source IP (e.g., "PILOT_129").

### Possible Solutions

1. Parse FPVGate's web API to get pilot configuration
2. Allow manual pilot name configuration on the display
3. Contribute a PR to FPVGate adding pilot info to webhook payload

### Impact

Low — display works correctly, just shows generic names instead of configured pilot names.
