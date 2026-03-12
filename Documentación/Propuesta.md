# Sistema IoT para Higiene del Sueño

## Integrantes

- Angie Paola Jaramillo Ortega
- Angee Ocampo
- Daniel León Danzo

## 1. Problema y Motivación

###  Contexto y Necesidades

En la actualidad, los trastornos del sueño constituyen un problema de salud pública a nivel mundial. De acuerdo con la Encuesta Global del Sueño 2025, realizada por la empresa ResMed, aproximadamente el 38 % de la población mundial reporta dificultades para dormir, siendo Hong Kong el país más afectado, donde el 58 % de sus habitantes manifiesta padecer algún trastorno del sueño. Adicionalmente, la mayoría de los encuestados afirmó no experimentar un descanso reparador, lo que evidencia una afectación generalizada en la calidad del sueño a nivel global.

Diversos estudios indican que este fenómeno no solo está asociado a condiciones médicas, sino también a factores del estilo de vida y del entorno. Aspectos como el estrés, la ansiedad, los problemas familiares y financieros, así como la exposición constante a aparatos tecnológicos, dificultan la conciliación y el mantenimiento del sueño. A pesar de reconocer estas dificultades, una parte significativa de la población no busca apoyo profesional ni adopta estrategias sistemáticas para mejorar su descanso.

En el contexto colombiano, la situación resulta igualmente preocupante. Se estima que el 47 % de la población presenta insomnio, lo que impacta directamente la memoria, el rendimiento académico y laboral, y la salud general. Estudios recientes señalan que hasta el 30 % de los accidentes de tránsito pueden estar relacionados con la somnolencia, mientras que los accidentes laborales pueden incrementarse hasta en un 225 % debido a la falta de descanso adecuado. Asimismo, se ha reportado que estos trastornos pueden afectar hasta en un 55 % la productividad laboral, y se encuentran estrechamente vinculados con condiciones como la ansiedad y la depresión.

Asimismo, los hábitos cotidianos asociados al uso de tecnología influyen negativamente en la calidad del sueño. En Colombia, el 51 % de las personas afirma revisar su teléfono celular antes de dormir, y más del 70 % tiene un televisor en su habitación, lo que incrementa la exposición a luz artificial y estímulos cognitivos durante las horas previas al descanso. Estos factores alteran los ritmos biológicos y dificultan la adaptación a rutinas de sueño saludables.

Frente a esta problemática, asociaciones médicas y profesionales de la salud recomiendan mantener horarios de sueño regulares, establecer rutinas previas al descanso evitando la exposición a la luz de los dispositivos electrónicos y controlar las condiciones ambientales del dormitorio, tales como la iluminación y la temperatura. Sin embargo, en la práctica, estas recomendaciones suelen depender únicamente de la disciplina individual del usuario y no cuentan con herramientas tecnológicas que faciliten su aplicación de forma constante en la vida diaria.

En consecuencia, se identifica la necesidad de contar con soluciones tecnológicas de apoyo que ayuden a las personas a crear y mantener rutinas adecuadas de higiene del sueño, mediante el control del entorno y la reducción de barreras asociadas a la disciplina individual.

###  Propuesta de Solución

Se propone el desarrollo de un sistema IoT orientado al monitoreo y apoyo a la higiene del sueño, enfocado en la recolección, análisis y visualización de variables ambientales relevantes, así como en la automatización de elementos del entorno que influyen en la creación y mantenimiento de rutinas de descanso adecuadas.

El sistema permitirá:

- Medir de forma continua variables ambientales como la iluminación, la temperatura, la humedad y el nivel de ruido, proporcionando información objetiva sobre el entorno de descanso.

- Automatización del sistema de iluminación del entorno de descanso, el cual influye en la creación y mantenimiento de rutinas adecuadas.

- Incorporar un sistema de alarma sonora, mediante un buzzer, que permita complementar rutinas de despertar programadas.

- Registrar información histórica de las variables sensadas, permitiendo al usuario visualizar su comportamiento a lo largo del tiempo por medio de un dashboard.

- Generar notificaciones de recomendaciones para el usuario a partir de los datos adquiridos, con el propósito de fomentar hábitos consistentes de higiene del sueño.

---

## 2. Arquitectura del Sistema
El sistema estará compuesto por tres bloques principales: un nodo IoT de adquisición y control, un servidor local y una aplicación móvil. El nodo IoT estará basado en una ESP32-S3, encargada de captar las variables del entorno y controlar el actuador. Esta se conectará por Wi-Fi a una Raspberry Pi, la cual funcionará como servidor local para la recepción, almacenamiento y visualización de los datos. Finalmente, el usuario podrá acceder a la información mediante una aplicación móvil, desde la cual también recibirá notificaciones de recomendaciones relacionadas con su rutina de sueño.

### 2.1. Capa de Percepción
La capa de percepción estará implementada sobre una ESP32-S3, a la cual se conectarán los sensores y el actuador del sistema. Los elementos contemplados son:

- Sensor de iluminación digital (BH1750), para medir la intensidad lumínica del entorno.

- Sensor digital de temperatura y humedad (DHT11).
  
- Sensor de nivel de ruido, para estimar el nivel de sonido ambiente dentro de la habitación.

- Sensor de movimiento(opcional), para detectar momento de incio y terminación del descanso.

- Buzzer para alarma sonora, para generar alertas sonoras programadas que apoyen las rutinas de despertar

- Actuador de iluminación, consistente en una lámpara de encendido gradual que simulará el amanecer.

La ESP32-S3 realizará la lectura periódica de los sensores, aplicará el filtrado correspondiente sobre las variables medidas, empaquetará los datos y los enviará al servidor local.

### 2.2.  Capa de Red

La capa de red estará basada en Wi-Fi como protocolo de comunicación. A través de esta red, la ESP32-S3 transmitirá los datos capturados hacia la Raspberry Pi. La comunicación entre ambos dispositivos se realizará mediante MQTT por túnel TLS, garantizando un intercambio de información seguro dentro de la arquitectura del sistema.

### 2.3. Capa de Aplicación

La capa de aplicación estará alojada en una Raspberry Pi, que actuará como servidor local del sistema. En esta capa se integrarán los siguientes componentes:

- Servidor MQTT, encargado de recibir los datos provenientes del nodo IoT y gestionar la comunicación con los demás módulos del sistema.

- Base de datos, donde se almacenará la información histórica de las variables ambientales medidas.

- Dashboard de visualización, accesible desde la aplicación móvil, para consultar el comportamiento de los sensores.

- Módulo de generación de recomendaciones y notificaciones, encargado de enviar al usuario alertas o sugerencias relacionadas con la higiene del sueño a partir de las condiciones detectadas.

El acceso del usuario al sistema se realizará desde una aplicación móvil, que permitirá visualizar el dashboard y recibir notificaciones.

### 2.4. Interacción máquina a máquina (M2M)

En la solución propuesta se contempla una interacción máquina a máquina entre la ESP32-S3 y la Raspberry Pi. La primera adquiere y transmite los datos del entorno, mientras que la segunda los procesa, almacena y genera acciones automáticas o recomendaciones para el usuario. Adicionalmente, la Raspberry Pi podrá enviar comandos hacia la ESP32-S3 para controlar el comportamiento de la lámpara de simulación de amanecer, de acuerdo con las rutinas configuradas en el sistema.

---

## 3. Variables a Medir

### 3.1. Variables medidas

- Iluminación ambiental (lux)

- Temperatura ambiente (°C)

- Humedad relativa (%)

- Nivel de ruido ambiental

- Detección de movimiento

### 3.2. Variables controladas

- Intensidad de iluminación de la lámpara

- Activación de alarma sonora mediante buzzer

## 4. Cronograma de Integración del Proyecto

### 4.1 Actividades relacionadas con la Fase 1: Preparación y diseño
#### 4.1.a. Revisión del estado del arte sobre higiene del sueño y sistemas IoT:
Se realizará una revisión bibliográfica sobre la higiene del sueño, los factores ambientales que afectan la calidad del descanso y el impacto de los hábitos nocturnos en la salud. 
#### 4.1.b. Identificación de requisitos del sistema:
Se definirán los requisitos funcionales y no funcionales del sistema, incluyendo las variables a monitorear (temperatura, luminosidad, ruido y movimiento), los requerimientos de conectividad, consumo energético, seguridad de los datos y comodidad del usuario. Esta actividad permitirá establecer las bases para el diseño del sistema.
#### 4.1.c. Diseño de la arquitectura del sistema:
Se elaborará el diseño de la arquitectura general del sistema, integrando los componentes de hardware (sensores, microcontrolador, actuadores, módulos de comunicación) y software (base de datos e interfaz de usuario). Se definirán los protocolos de comunicación y el flujo de datos entre los diferentes módulos.
#### 4.1.d. Diseño preliminar de la interfaz de usuario:
Se identificarán las variables relevantes a visualizar en la interfaz de usuario, así como la interacción con el mismo.


### 4.2.Actividades relacionadas con la Fase 2: Integración de hardware y sensores
#### 4.2.a. Selección y caracterización de sensores:
Se realizará una revisión de la documentación técnica de los sensores seleccionados (temperatura, humedad, luminosidad, movimiento y ruido), con el fin de identificar sus rangos de operación, precisión, limitaciones y requerimientos de alimentación. Posteriormente, se efectuarán pruebas experimentales para comprender su comportamiento y validar su idoneidad para el sistema.
#### 4.2.b. Integración del sistema de hardware:
Se integrarán los sensores con el microcontrolador y los actuadores para garantizar la correcta adquisición de datos y la respuesta del sistema. Esta actividad incluirá la verificación de la lectura adecuada de las variables, la sincronización de los datos obtenidos y la validación del funcionamiento de los actuadores.
#### 4.2.c. Calibración y pruebas de adquisición de datos:
Se realizarán pruebas de calibración para garantizar la precisión de las mediciones. Asimismo, se evaluará la adquisición simultánea de datos provenientes de múltiples sensores, verificando la estabilidad del sistema de monitoreo.

### 4.3. Actividades relacionadas con la Fase 3: Conectividad y almacenamiento
#### 4.3.a. Implementación de la conectividad inalámbrica:
Se desarrollarán los mecanismos de conexión del dispositivo a redes Wi-Fi, permitiendo la transmisión de datos hacia una plataforma en la nube o servidor local. Se evaluará la estabilidad de la conexión y los mecanismos de reconexión automática ante fallos.
#### 4.3.b. Diseño e implementación del protocolo de comunicación:
Se definirá la estructura de los mensajes para la transmisión de datos, mediante protocolos como MQTTS. Esta actividad permitirá garantizar la interoperabilidad entre el dispositivo, el servidor local y la interfaz de usuario.
#### 4.3.c. Implementación de almacenamiento local y sincronización:
Se desarrollarán mecanismos de almacenamiento temporal de datos en memoria local para escenarios sin conectividad. Posteriormente, se implementará la sincronización automática de la información una vez se restablezca la conexión, garantizando la integridad de los datos.

### 4.4. Actividades relacionadas con la Fase 4: Interfaz y visualización
#### 4.4.a. Desarrollo de la interfaz de visualización:
Se implementará un panel de control (dashboard) que permite visualizar en tiempo real las variables registradas por el sistema. Esta interfaz facilitará el monitoreo del descanso del usuario.
#### 4.4.b. Generación de reportes de calidad del sueño:
Se diseñarán algoritmos para el análisis de los datos recolectados, orientados a generar reportes sobre la calidad del entorno de sueño. Estos reportes incluirán tendencias y métricas para la evaluación del descanso.
#### 4.4.c. Implementación de alertas y recomendaciones:
Se desarrollará un sistema de notificaciones que proporcione recomendaciones personalizadas al usuario, tales como establecer una rutina regular de sueño, adelantar la hora de la cena y reducir el uso de dispositivos electrónicos antes de dormir, con el propósito de fomentar hábitos de descanso saludables.

### 4.5. Actividades relacionadas con la Fase 5: Integración total y pruebas
#### 4.5.a. Integración completa del sistema:
Se realizará la integración de los módulos de adquisición de datos, conectividad, almacenamiento e interfaz de usuario, verificando la correcta comunicación entre los componentes y el funcionamiento integral del sistema.
#### 4.5.b. Pruebas de funcionamiento en condiciones reales:
Se llevarán a cabo pruebas nocturnas en un entorno controlado, con el fin de evaluar el desempeño del sistema. Se analizará la estabilidad del monitoreo, la precisión de los datos, la respuesta oportuna de los actuadores, y la utilidad de las recomendaciones generadas.
#### 4.5.c. Corrección de errores y optimización del sistema:
Se identificarán y corregirán fallos detectados durante las pruebas, así como posibles mejoras en el consumo energético, la estabilidad del sistema y la experiencia de usuario.

### 4.6. Actividades relacionadas con la Fase 6: Validación y mejoras
#### 4.6.a. Validación de resultados obtenidos:
Se analizarán los datos recopilados para evaluar la efectividad del sistema en la mejora de las condiciones de higiene del sueño. Esta actividad permitirá determinar el cumplimiento de los objetivos propuestos.
#### 4.6.b. Mejoras en la experiencia de usuario:
Se realizarán ajustes en la interfaz y en las recomendaciones generadas, con base en la retroalimentación obtenida durante las pruebas, con el fin de mejorar la usabilidad y aceptación del sistema.

### 4.7. Actividades relacionadas con la Fase 7: Documentación y entrega final
#### 4.7.a. Documentación técnica del sistema:
Se elaborará la documentación técnica del proyecto, incluyendo la descripción de la arquitectura, manual de usuario, así como los detalles de implementación del sistema.
#### 4.7.b. Análisis de resultados y conclusiones:
Se documentarán los resultados obtenidos, evaluando el impacto del sistema propuesto. 
#### 4.7.c. Preparación de la presentación final:
Se desarrollará la presentación que evidencie el funcionamiento del sistema y los resultados alcanzados.




