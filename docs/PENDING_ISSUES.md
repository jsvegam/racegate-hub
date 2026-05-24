# Problemas Pendientes — RaceGate Hub

## BLOQUEANTE: Webhooks de FPVGate no funcionan en modo AP

**Fecha**: 2026-05-24
**Estado**: Sin resolver

### Problema

FPVGate no envía webhooks HTTP cuando opera como Access Point (AP). El código de webhooks en FPVGate (`lib/WEBHOOK/webhook.cpp`) tiene esta condición:

```cpp
if (WiFi.status() != WL_CONNECTED) {
    DEBUG("Webhook skipped - WiFi not connected\n");
    return;
}
```

`WiFi.status() == WL_CONNECTED` solo es `true` cuando el ESP32 está conectado como **cliente (STA)** a otra red WiFi. Cuando FPVGate es el AP (que es el modo por defecto), esta condición es `false` y los webhooks nunca se envían.

### Lo que funciona

- ✅ Display se conecta al WiFi de FPVGate (IP: 192.168.4.2)
- ✅ Display levanta servidor HTTP (responde 404 a GET)
- ✅ FPVGate detecta vueltas por RSSI correctamente
- ✅ FPVGate muestra vueltas en su interfaz web
- ❌ FPVGate NO envía POST a 192.168.4.2/Lap (webhooks bloqueados)

### Soluciones posibles

1. **Polling HTTP (recomendado)**: El display consulta periódicamente un endpoint de FPVGate para obtener datos de carrera. La interfaz web de FPVGate ya muestra datos en tiempo real, así que debe existir un endpoint o WebSocket que podamos consumir.

2. **Modificar FPVGate**: Cambiar la condición del webhook para que también funcione en modo AP (reemplazar `WiFi.status() == WL_CONNECTED` por una verificación de que hay clientes conectados al AP). Requiere fork de FPVGate.

3. **Modo STA en FPVGate**: Configurar FPVGate para que se conecte a una red WiFi existente (modo Station) en vez de crear su propio AP. Así `WiFi.status()` sería `WL_CONNECTED`. Pero requiere un router/AP externo.

### Próximos pasos

- Investigar qué endpoints HTTP usa la interfaz web de FPVGate para obtener datos de carrera en tiempo real
- Implementar polling en el display como alternativa a webhooks
- Considerar WebSocket si FPVGate lo usa para su interfaz web
