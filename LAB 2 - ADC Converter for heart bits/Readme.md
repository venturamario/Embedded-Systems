## 1. Introducción

En esta práctica se pretende realizar un algoritmo con el lenguaje C que, junto un osciloscopio del laboratorio y un sensor de frecuencia cardíaca, permite detectar la frecuencia cardíaca del usuario que toca el sensor (típicamente con el dedo y en un entorno estable sin turbulencias o temblores).

Debe usarse el ESP32-C6 para el procesamiento digital e implementar un ADC, mostrando la frecuencia cardíaca por la terminal del ordenador cada 5-10 segundos. 

En la imagen mostrada a continuación puede verse un ejemplo del entorno y el circuito implementado en el laboratorio.



## 2. Solución planteada

El algoritmo propuesto se ha adjuntado en la entrega, pero también puede encontrarse en el repositorio GitHub del Lab. 

La lógica del proyecto recae en transformar la señal analógica (el cual podemos ver en el osciloscopio) en un señal digital donde podemos detectar si hay o no pulso. Para detectar la entrada analógica trabajamos con un detector de pulso a partir de un dedo de la mano. Para poder comprobar que la entrada digital y la entrada analógica coinciden hemos decidido trabajar con el ESP32-C6 y el osciloscopio para ver a simple vista si el gráfico y el output del código coinciden. Para poder trabajar de esta forma paralela simplemente conectamos las pinzas rojas en la entrada del detector de input y las negras en el ground.

Para detectar los picos hemos tomado un THRESHOLD a 2380 ya que es un valor que coincide con la entrada de nuestro detector. De esta forma si hay una subida superior a este valor podemos asegurar que hay un pico y por lo tanto conseguimos transformar la entrada analógica en digital.

Este algoritmo comienza declarando algunos valores globales para todo el programa ya que serán útiles en diferentes instancias de este.

Tras esto, ya comienza la función main void app_main(void).
Al inicio de la función se configura el filtro ADC y el canal, y posteriormente se entra en el bucle infinito while(1)que realizará las mediciones, calculará frecuencias, etc. Mediante la función adc_oneshot_read(adc1_handle,ADC_CHANNEL_0,&value), se realiza una medición al inicio de cada iteración y se almacena su valor en la variable value.

Para obtener un valor más preciso de nuestro pulso vamos a hacer la media entre los últimos 5 picos detectados. Es decir, restamos el tiempo inicial (T0) al tiempo final (el tiempo del pico 5) y dividimos este valor entre 5. Se podría dividir entre 10 para tener un valor ligeramente más preciso.

De esta manera, obtenemos el período medio de un pulso cardíaco. A partir de este período, la frecuencia cardíaca se calcula aplicando la fórmula frecuencia (latidos por minuto) = 60 / período medio (en segundos). Así, con la media de los últimos 5 picos reducimos el ruido y las fluctuaciones de la señal, logrando una estimación más estable y fiable de la frecuencia cardíaca que posteriormente mostramos en la terminal cada 5-10 segundos.


## 3. Referencias

Analog to Digital Converter (ADC)
https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/adc/index.html
