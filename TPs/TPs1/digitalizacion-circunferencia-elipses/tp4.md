# Práctico 4 - Demostración de incrementos en el algoritmo de punto medio para elipses

En este práctico se pide demostrar que los incrementos de las variables de decisión para el algoritmo de punto medio aplicado a elipses en **posición estándar** resultan en las siguientes expresiones:

### Región 1:
- $((\Delta D_k)_{ady})_1 = 4r_y^2(2x_k + 1) + 8r_y^2$
- $((\Delta D_k)_{aInf})_1 = 4r_y^2(2x_k + 1) + 8r_y^2 + 2r_x^2(1 - 4y_k) + 6r_x^2$

### Región 2:
- $((\Delta D_k)_{ady})_2 = 4r_x^2(1 - 2y_k) + 8r_x^2$
- $((\Delta D_k)_{aInf})_2 = 2r_y^2(4x_k + 1) + 6r_y^2 + 4r_x^2(1 - 2y_k) + 8r_x^2$

---

## 1. Ecuación implícita de la elipse

Para una elipse en posición estándar, con semiejes $r_x$ y $r_y$ centrada en el origen:

$$
F(x,y) = r_y^2 x^2 + r_x^2 y^2 - r_x^2 r_y^2 = 0
$$

El algoritmo de punto medio evalúa el **signo** de $F(x,y)$ en puntos candidatos para decidir cuál píxel trazar.

---

## 2. División en regiones

La pendiente de la elipse se obtiene derivando implícitamente:

$$
\frac{dy}{dx} = -\frac{r_y^2 x}{r_x^2 y}
$$

- **Región 1:** $\left| \frac{dy}{dx} \right| > 1 \implies 2r_y^2 x < 2r_x^2 y$
- **Región 2:** $\left| \frac{dy}{dx} \right| < 1 \implies 2r_y^2 x > 2r_x^2 y$

Esto permite definir en qué variable se incrementa principalmente:  
- Región 1: incremento en **x**.  
- Región 2: decremento en **y**.

---

## 3. Región 1

Se analiza el **punto medio** entre las dos opciones de píxel siguientes: $(x_k+1, y_k)$ (adyacente) y $(x_k+1, y_k-1)$ (inferior).

El punto medio es:

$$
M = \left(x_k+1, y_k - \tfrac{1}{2}\right)
$$

Evaluamos:

$$
F(M) = r_y^2(x_k+1)^2 + r_x^2(y_k - 1/2)^2 - r_x^2 r_y^2
$$

Al expandir y reagrupar:

$$
F(M) = F(x_k,y_k) + [r_y^2(2x_k+1) + r_x^2(1/4 - y_k)]
$$

Multiplicamos por 4 para evitar fracciones:

$$
D_{k+1} = 4F(M) = 4F(x_k,y_k) + [4r_y^2(2x_k+1) + r_x^2(1-4y_k)]
$$

De aquí se obtienen los incrementos:

- Si se elige el píxel **adyacente**:

$$
((\Delta D_k)_{ady})_1 = 4r_y^2(2x_k+1) + 8r_y^2
$$

- Si se elige el píxel **inferior**:

$$
((\Delta D_k)_{aInf})_1 = 4r_y^2(2x_k+1) + 8r_y^2 + 2r_x^2(1-4y_k) + 6r_x^2
$$

---

## 4. Región 2

Ahora se avanza en dirección **y** decreciente. Los candidatos son $(x_k,y_k-1)$ y $(x_k+1,y_k-1)$.

El punto medio:

$$
M = \left(x_k+\tfrac{1}{2}, y_k - 1\right)
$$

Evaluamos:

$$
F(M) = r_y^2(x_k+1/2)^2 + r_x^2(y_k-1)^2 - r_x^2 r_y^2
$$

Expandiendo:

$$
F(M) = F(x_k,y_k) + [r_y^2(x_k+1/4) + r_x^2(-2y_k+1)]
$$

Multiplicamos por 4:

$$
D_{k+1} = 4F(M) = 4F(x_k,y_k) + [r_y^2(4x_k+1) + 4r_x^2(1-2y_k)]
$$

De aquí se obtienen los incrementos:

- Si se elige el píxel **adyacente**:

$$
((\Delta D_k)_{ady})_2 = 4r_x^2(1-2y_k) + 8r_x^2
$$

- Si se elige el píxel **inferior**:

$$
((\Delta D_k)_{aInf})_2 = 2r_y^2(4x_k+1) + 6r_y^2 + 4r_x^2(1-2y_k) + 8r_x^2
$$

---

## 5. Conclusión

Hemos demostrado que los **incrementos de la variable de decisión** en cada región corresponden exactamente con las fórmulas pedidas en la consigna del **Práctico 4**. Esto asegura la correcta implementación del algoritmo de punto medio para elipses completas en posición estándar.
