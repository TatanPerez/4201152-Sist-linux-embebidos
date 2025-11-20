// wifi.h
// Función para conectar el ESP32-C6 en modo STA a una red Wi‑Fi.

#ifndef WIFI_H
#define WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

// Conecta a la red Wi-Fi con SSID y contraseña proporcionados.
// Internamente inicializa esp-netif y el stack de Wi-Fi.
void wifi_init_sta(const char *ssid, const char *password);

#ifdef __cplusplus
}
#endif

#endif // WIFI_H
