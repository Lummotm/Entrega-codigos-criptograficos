### 1. Cifrado en Flujo (Stream Ciphers) y el Generador (LFSR)

El programa en C implementa un **cifrador en flujo** (*stream cipher*). En lugar de dividir el mensaje en grandes bloques, este sistema cifra el archivo byte a byte (carácter a carácter) de manera continua a medida que los va leyendo.

* **Sucesión de recurrencia lineal (LFSR):** Para cifrar necesitamos una sucesión pseudoaleatoria de números $(c_n)$, llamada **sucesión cifrante**. En lugar de tener una clave tan larga como el archivo, usamos una semilla inicial corta (la clave de hasta 13 caracteres) almacenada en un vector (`registro[13]`) y generamos el resto mediante un **Registro de Desplazamiento con Retroalimentación Lineal (LFSR)**.


* **El Cuerpo Finito $\mathbb{F}_{251}$:** El número $251$ es el mayor número primo menor que $256$ (lo que cabe en un byte). Por el *Teorema 29*, el anillo $\mathbb{Z}/(251) = \mathbb{F}_{251}$ es un **cuerpo finito**.


* **Funcionamiento de `avance()`:** En el código, la función `avance()` implementa una recurrencia lineal de orden $k=13$ sobre el cuerpo $\mathbb{F}_{251}$:



$$s_{n+13} \equiv (3s_{n+9} + 2s_{n+2} + 5s_n) \pmod{251}$$



(En C: `bb = (3*registro[9] + 2*registro[2] + 5*registro[0]) % 251;`).
Después, desplaza todos los elementos del array una posición hacia la izquierda y coloca el nuevo valor `bb` en `registro[12]`, que será el término $c_n$ (`cifrante`) que se usará en ese ciclo. Como $c_n$ se calcula módulo $251$, sus valores posibles están siempre en el conjunto $\{0, 1, 2, \dots, 250\}$.



---

### 2. El Anillo $\mathbb{Z}/(256)$ y la Aritmética de Bytes en C

Un archivo informático se lee byte a byte. En el lenguaje C, el tipo de dato `unsigned char` ocupa exactamente 1 byte (8 bits) y almacena valores enteros entre $0$ y $255$.

* **El Anillo $\mathbb{Z}/(256)$:** Matemáticamente, trabajar con bytes equivale a operar en el anillo cociente de los enteros módulo 256: $\mathbb{Z}/(256) = \{\overline{0}, \overline{1}, \dots, \overline{255}\}$. Como $256 = 2^8$ no es un número primo, $\mathbb{Z}/(256)$ **no es un cuerpo**, sino un anillo conmutativo con divisores de cero.


* **Desbordamiento (Overflow) natural en C:** Una gran ventaja de usar `unsigned char` en C es que la aritmética modular en $\mathbb{Z}/(256)$ es automática. Si sumas o multiplicas dos variables `unsigned char`, el resultado se trunca automáticamente a los 8 bits menos significativos, lo cual es equivalente a hacer `% 256` (módulo 256).
* **Notación del ejercicio:**
* $m_n \in \mathbb{Z}/(256)$: El byte original leído del archivo claro (`leocaracter`).


* $c_n \in \{0, \dots, 250\}$: El byte de la sucesión cifrante (`cifrante = (unsigned char) registro[12]`).


* $e_n \in \mathbb{Z}/(256)$: El byte resultante cifrado (`escribocaracter`).





---

### 3. Las Tres Operaciones de Cifrado y Descifrado (Ejercicio 18)

En el programa modelo original, todos los bytes se cifran y descifran con una única operación (la suma bitwise XOR). En tu ejercicio, **la operación depende del valor que tome $c_n$ en cada paso**, por lo que **el programa de descifrado es distinto del de cifrado**:

#### Caso A: $c_n$ es par ($c_n \in \{0, 2, 4, \dots, 250\}$)



Se opera con el grupo aditivo $(\mathbb{Z}/(256), +)$.

* **Cifrado:** Se suma módulo 256:



$$e_n \equiv m_n + c_n \pmod{256}$$


* **Descifrado:** La operación inversa en el grupo aditivo es sumar el elemento opuesto (restar módulo 256):

$$m_n \equiv e_n - c_n \pmod{256}$$



*(En C, al usar `unsigned char`, basta con hacer `escribocaracter = leocaracter - cifrante;` ya que el underflow en enteros sin signo aplica el módulo 256 de forma automática).*

#### Caso B: $c_n \in \{5, 7, 9, 11, 13, 15, 17, 19, 21\}$

Se opera con la multiplicación en el anillo $(\mathbb{Z}/(256), \cdot)$.

* **Teoría de Unidades e Inversos:** Para que la multiplicación $e_n \equiv m_n \cdot c_n \pmod{256}$ sea reversible (inyectiva) y podamos recuperar $m_n$, el elemento $c_n$ **debe ser una unidad** (inversible) en $\mathbb{Z}/(256)$.


* Según el *Teorema 31*, $\overline{c_n}$ tiene inverso multiplicativo en $\mathbb{Z}/(256)$ si y solo si $\text{m.c.d.}(c_n, 256) = 1$.


* Como $256 = 2^8$, cualquier número impar es coprimo con $256$. Todos los números de la lista $\{5, 7, 9, 11, 13, 15, 17, 19, 21\}$ son impares, por lo que todos son unidades en $\mathbb{Z}/(256)$. *(Nota: En el enunciado aparece por errata tipográfica el 20 en la frase de abajo, pero si cuentas los elementos verás que hay 9 números impares arriba y 9 inversos abajo)*.




* **Cálculo del inverso:** Los inversos se calculan resolviendo la Identidad de Bézout ($1 = s \cdot 256 + t \cdot c_n$) mediante el **Algoritmo de Euclides Extendido** (Sección 2.2 y 6.2), de donde $c_n^{-1} \equiv t \pmod{256}$. El ejercicio ya te da los inversos calculados:


* $5^{-1} = 205 \quad (\text{pues } 5 \times 205 = 1025 = 4 \times 256 + 1 \equiv 1 \pmod{256})$

* $7^{-1} = 183$, $\quad 9^{-1} = 57$, $\quad 11^{-1} = 163$, $\quad 13^{-1} = 197$, $\quad 15^{-1} = 239$, $\quad 17^{-1} = 241$, $\quad 19^{-1} = 27$, $\quad 21^{-1} = 61$.




* **Cifrado:** Multiplicamos por $c_n$ módulo 256:



$$e_n \equiv m_n \cdot c_n \pmod{256}$$


* **Descifrado:** Multiplicamos el byte cifrado por el inverso multiplicativo $c_n^{-1}$ correspondiente en $\mathbb{Z}/(256)$:



$$m_n \equiv e_n \cdot c_n^{-1} \pmod{256}$$



#### Caso C: $c_n$ no está en ninguno de los conjuntos anteriores (es decir, $c_n \in \{1, 3, 23, 25, \dots, 249\}$)



Se opera exactamente igual que en el programa modelo original, mediante la **suma bitwise (XOR)**.

* **Teoría Algebraica ($\mathbb{F}_2^8$):** Cada byte se ve como un vector de 8 bits en el espacio vectorial $(\mathbb{Z}/(2))^8$. La operación $\oplus$ suma bit a bit módulo 2 (sin acarreo).


* **Propiedad de involución:** En $\mathbb{Z}/(2)$, sumar y restar es la misma operación ($1 + 1 = 0$), por lo que cada elemento es su propio inverso: $(m_n \oplus c_n) \oplus c_n = m_n \oplus (c_n \oplus c_n) = m_n \oplus 0 = m_n$.


* **Cifrado y Descifrado:** Se usa el operador `^` de C en ambos programas:



$$e_n = m_n \oplus c_n \quad \Longleftrightarrow \quad m_n = e_n \oplus c_n$$



---

### 4. Resumen de cómo llevarlo al código C

Para realizar tu ejercicio a partir de `cifrador__68.c`, toda la parte de lectura de clave, inicialización del `registro` y la función `avance()` **se mantienen exactamente iguales** tanto en el programa cifrador como en el descifrador (para que ambos generen la misma sucesión $c_n$).

Lo único que debes modificar es el interior del bucle `while(feof(fientrada)==0)` donde se calcula `escribocaracter`:

1. **En el programa cifrador:** Evalúas `cifrante` con condicionales (`if / else if / else` o un `switch`).
* Si `cifrante % 2 == 0`: haces `escribocaracter = leocaracter + cifrante;`.


* Si `cifrante` es impar y está entre `5` y `21`: haces `escribocaracter = leocaracter * cifrante;`.


* En el resto de casos: haces `escribocaracter = leocaracter ^ cifrante;`.




2. **En el programa descifrador:** Evalúas el mismo `cifrante`:
* Si `cifrante % 2 == 0`: haces `escribocaracter = leocaracter - cifrante;`.


* Si `cifrante` está entre `5` y `21` (impar): sustituyes `cifrante` por su inverso de la lista `{205, 183, 57, 163, 197, 239, 241, 27, 61}` y multiplicas `escribocaracter = leocaracter * inverso;`.


* En el resto de casos: haces `escribocaracter = leocaracter ^ cifrante;`.
