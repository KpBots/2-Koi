# 2-Koi · Mini-sumo

**2-Koi quedó segundo en la competición de mini-sumo de la OSHWDem 2024.** En esta ocasión se adapta su firmware a la [normativa de 2026](https://rules.oshwdem.org/loita_sumo_es): el juez inicia y detiene el robot por infrarrojos y el movimiento comienza sin la antigua espera de cinco segundos.

El programa se carga en un Arduino Nano con ATmega328P. El módulo IRStart recibe y decodifica el mando IR; el Nano solo lee el estado mantenido de su salida `OUT`. Por tanto, el IRStart debe tener ya cargados los códigos START y STOP adecuados. El programa del Nano no decodifica RC05 ni configura el módulo.

## Conexiones

| Componente | Pin del Nano | Uso |
| --- | --- | --- |
| Sensor de presencia | A0 | Medida analógica del oponente |
| IRStart VCC | A1 | Salida HIGH que alimenta el módulo |
| IRStart GND | A2 | Salida LOW que hace de masa |
| IRStart OUT | A3 | Entrada digital; LOW = parado, HIGH = activo |
| Sensor de borde izquierdo | D7 | Entrada activa en LOW |
| Sensor de borde derecho | D8 | Entrada activa en LOW |
| Selector derecho | D6 | Entrada con pull-up |
| Común de los selectores | D5 | Salida LOW |
| Selector frontal | D4 | Entrada con pull-up |
| Selector izquierdo | D2 | Entrada con pull-up |
| Motor izquierdo | D9 y D10 | Entradas del puente H |
| Motor derecho | D3 y D11 | Entradas del puente H |

Los selectores se activan al unir su entrada con el común D5. El interruptor izquierdo está montado con la orientación física inversa a la esperada: comprueba su posición por la maniobra que realiza, no por la marca ON/OFF de la carcasa. Para que un cable `OUT` suelto no deje A3 flotante, se puede añadir una resistencia externa de 10 kΩ entre A3 y GND.

## Elegir la estrategia de salida

Coloca los selectores **antes de enviar START**. El programa los lee al comenzar cada asalto. La maniobra inicial dura solo el tiempo indicado; después el robot pasa a buscar al oponente.

| Selectores activados | Primera maniobra tras START |
| --- | --- |
| Izquierdo + derecho | Retrocede 300 ms |
| Izquierdo | Gira sobre sí mismo a la izquierda 150 ms |
| Frontal | Avanza 150 ms |
| Derecho | Gira sobre sí mismo a la derecha 120 ms |
| Ninguno | Empieza a buscar inmediatamente |

Si activas varios selectores, la prioridad es: **izquierdo + derecho**, luego **izquierdo**, luego **frontal** y, por último, **derecho**. Por ejemplo, frontal + derecho ejecuta la salida frontal. El selector frontal no modifica la salida hacia atrás si están activados ambos laterales.

Durante la búsqueda, el robot avanza hacia un objeto detectado aproximadamente entre 20 y 200 mm. Si no detecta ninguno, alterna al azar giros a izquierda y derecha. El LED integrado del Nano se enciende mientras detecta al oponente. La maniobra inicial se completa antes de atender los sensores de borde; después, un sensor de borde activo tiene prioridad sobre la búsqueda: el robot retrocede 100 ms y gira para alejarse del borde. STOP sigue teniendo prioridad en todo momento.

## Arranque, parada y carga

1. Abre `2-Koi/2-Koi.ino` en Arduino IDE. Selecciona **Arduino Nano**, ATmega328P y el procesador/bootloader que corresponda a tu placa; después carga el programa por USB.
2. Para la primera prueba, levanta las ruedas del suelo. Enciende el robot con IRStart conectado: debe quedarse parado hasta recibir START. Si el Nano arranca cuando `OUT` ya está en HIGH, esperará a ver primero un estado LOW y luego otro START.
3. Ajusta los selectores y envía START con el mando del juez, un mando compatible o la app configurada para los códigos del IRStart. Observa las ruedas justo al arrancar: la estrategia solo dura entre 120 y 300 ms; después comienza la búsqueda.
4. Envía STOP mientras se mueve: los motores deben detenerse, incluso si está retrocediendo o girando por un borde. Un nuevo START inicia otra vez la estrategia seleccionada.
5. Antes de ponerlo en el dohyo, comprueba por separado los dos sensores de borde y que el LED del Nano responde al sensor de presencia. Haz la prueba de START/STOP con las ruedas levantadas cada vez que cambies el firmware.

El código no utiliza el puerto serie durante el combate. La señal `OUT` del IRStart es un nivel de marcha/parada, no una trama que haya que interpretar en el Nano.
