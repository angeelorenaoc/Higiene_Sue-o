# Higiene_Sueño
Este repositorio tiene toda la información referente a la implementación de un sistema que ayuda a mejorar la higiene del sueño.

## Descripción
El proyecto se trata de una plataforma distribuida de IoT para el monitoreo de la higiene del sueño, recolectando datos ambientales desde sensores instalados en la habitacion, evalua reglas de automatizacion configurables y expone una interfaz web para el monitoreo de las mediciones, las configuraciones y de las acciones realizadas.

## Arquitectura

```mermaid
flowchart TD
    ESP32[ESP32] <-->|Telemetry / Commands| MQTT[MQTT Broker]

    subgraph Backend["Raspberry Pi Backend"]
        MQTTLayer[MQTT Layer]
        Repository[Repository]
        SQLite[(SQLite)]
        API[HTTP API]
        Rules[Rules Engine]

        MQTTLayer --> Repository
        Repository <--> SQLite
        MQTTLayer --> Rules
        Rules --> MQTTLayer
        API <--> Repository
    end

    MQTT <--> MQTTLayer
    Dashboard[React Dashboard] <-->|REST API| API
```
