# Arquitectura de Red — RaceGate Hub

## Fecha: 2026-05-25

## El Problema Original

FPVGate no envía webhooks cuando opera como AP (Access Point). El código tiene:

```cpp
if (WiFi.status() != WL_CONNECTED) {
    return;  // Nunca envía en modo AP
}
```

### ¿Por qué falla?

`WiFi.status() == WL_CONNECTED` se evalúa **en el ESP32 que ejecuta FPVGate** (el timer).
Esta condición solo es `true` cuando ese ESP32 está conectado **como cliente (STA)** a otra red.

Cuando FPVGate **es** el AP (crea la red), él no está conectado a nadie — él ES la red.
Que otros dispositivos se conecten a él no cambia su propio `WiFi.status()`.

```
FPVGate (Timer)                    Display (Leaderboard)
┌─────────────────────┐            ┌─────────────────────┐
│  Modo: AP           │            │  Modo: STA (cliente) │
│                     │            │                      │
│  WiFi.status() =    │   WiFi    │  WiFi.status() =     │
│    WL_IDLE ❌       │◄──────────│    WL_CONNECTED ✅   │
│                     │            │                      │
│  "¿Estoy conectado  │            │  "¿Estoy conectado   │
│   a otra red?"      │            │   a otra red?"       │
│  → NO. Yo SOY la red│            │  → SÍ, al AP de     │
│                     │            │    FPVGate"          │
└─────────────────────┘            └─────────────────────┘
```

---

## ¿Por qué FPVGate fue diseñado así?

FPVGate es un sistema **escalable multi-nodo**. Está pensado para carreras con múltiples
gates (puertas) donde cada gate tiene su propio timer ESP32.

En ese escenario, **todos los timers se conectan a un router WiFi externo**:

```
Router WiFi externo (hotspot celular, router portátil, o WiFi de casa)
   │
   ├── Timer Nodo 1 (STA) → WL_CONNECTED ✅ → envía webhooks
   ├── Timer Nodo 2 (STA) → WL_CONNECTED ✅ → envía webhooks
   ├── Timer Nodo 3 (STA) → WL_CONNECTED ✅ → envía webhooks
   │
   ├── Celular organizador → interfaz web de control
   └── Display/Leaderboard (STA) → recibe webhooks
```

El chequeo `WL_CONNECTED` asegura que el nodo está en la red compartida antes de
intentar enviar datos. Tiene sentido en multi-nodo con router, pero NO en el escenario
de un solo timer operando como AP standalone.

---

## Escenarios de Despliegue

### Escenario A: Desarrollo en casa (WiFi existente)

```
Router WiFi de casa (ya tenés internet)
   │
   ├── Tu Timer FPVGate (STA) → WL_CONNECTED ✅ → webhooks funcionan
   ├── Tu Display (STA) → WL_CONNECTED ✅ → recibe webhooks
   ├── Tu celular → accede a interfaz web
   └── Tu PC → desarrollo
```

**Configuración**: En la web de FPVGate (192.168.4.1), configurar SSID y password
de tu WiFi de casa. FPVGate se reinicia en modo STA.

**No necesitás internet** — solo que todos estén en la misma red local.
El router solo actúa como "punto de encuentro" para que los dispositivos se vean.

### Escenario B: Campo con hotspot celular

```
Hotspot celular (AP)
   │
   ├── Tu Timer (STA) → WL_CONNECTED ✅
   ├── Timer piloto 2 (STA) → WL_CONNECTED ✅
   ├── Timer piloto 3 (STA) → WL_CONNECTED ✅
   │
   └── Tu Display (STA) → recibe webhooks de todos
```

**Basta con compartir WiFi desde el celular**. No importa si tiene datos móviles o no.
El hotspot crea una red local y los ESP32 se ven entre sí por LAN, sin necesitar internet.

### Escenario C: Tu timer como AP master (problema parcial)

```
Tu Timer FPVGate (AP master)
   │
   ├── Timer piloto 2 (STA) → WL_CONNECTED ✅ → envía webhooks OK
   ├── Timer piloto 3 (STA) → WL_CONNECTED ✅ → envía webhooks OK
   │
   └── Tu Display (STA) → recibe webhooks de otros, pero NO del master
```

En este escenario, los otros timers sí envían webhooks (son STA, tienen WL_CONNECTED).
Pero **tu propio timer** (el AP) no puede enviarse webhooks a sí mismo por la condición.

---

## Decisión: Usar red WiFi externa (Escenario A/B)

### Justificación

1. **No requiere modificar FPVGate** — funciona con el firmware oficial
2. **Escalable** — soporta múltiples timers de otros pilotos
3. **Simple** — en casa usás tu WiFi, en campo usás hotspot del celular
4. **Los webhooks funcionan** — todos los dispositivos son STA con WL_CONNECTED

### Configuración necesaria

| Dispositivo | Modo WiFi | Se conecta a | Resultado |
|-------------|-----------|--------------|-----------|
| FPVGate (timer) | STA | WiFi externa (casa/hotspot) | WL_CONNECTED ✅ |
| Display (leaderboard) | STA | Misma WiFi externa | WL_CONNECTED ✅ |
| Celular | Cliente | Misma WiFi (o es el hotspot) | Accede a web FPVGate |

### Pasos para configurar

**En casa (desarrollo):**
1. Encender FPVGate → crea AP "FPVGate_XXXX"
2. Conectarse al AP desde celular
3. Ir a 192.168.4.1 → configurar WiFi de casa (SSID + password)
4. FPVGate se reinicia y se conecta a tu WiFi como STA
5. Configurar display para conectarse a la misma WiFi de casa
6. Ambos en la misma red → webhooks funcionan

**En campo (carrera):**
1. Activar hotspot en celular (no necesita datos móviles)
2. Configurar FPVGate para conectarse al hotspot
3. Configurar display para conectarse al hotspot
4. Otros pilotos configuran sus timers al mismo hotspot
5. Todos en la misma red → webhooks de todos llegan al display

---

## Alternativas descartadas (por ahora)

| Alternativa | Por qué no |
|-------------|------------|
| Fork de FPVGate (fix WL_CONNECTED) | Requiere mantener fork, mergear updates |
| Polling HTTP al AP de FPVGate | Funciona pero es menos eficiente que webhooks |
| Timer como AP + polling local | Solo resuelve caso single-node, no escala |

---

## Impacto en el código del Display

El `FPVGateProvider` actual ya está diseñado para modo STA (se conecta a una red WiFi).
Solo necesitamos cambiar el SSID/password de la red de FPVGate AP a la red WiFi externa:

```ini
# platformio.ini — antes (conecta al AP de FPVGate):
-D FPVGATE_SSID=\"FPVGate_E110\"
-D FPVGATE_PASSWORD=\"fpvgate1\"

# después (conecta a WiFi externa compartida):
-D FPVGATE_SSID=\"MiWiFiCasa\"
-D FPVGATE_PASSWORD=\"mipassword\"
```

El resto del código (webhook listener en el display) no cambia.

---

## Próximos pasos

1. [ ] Configurar FPVGate en modo STA (conectar a WiFi de casa)
2. [ ] Actualizar SSID/password en platformio.ini del display
3. [ ] Flashear display y verificar que recibe webhooks
4. [ ] Documentar IP asignada al display para configurar webhook en FPVGate
5. [ ] Test end-to-end: FPVGate detecta vuelta → webhook → display actualiza
