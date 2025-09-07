# TP3 — Computación Gráfica y Ambientes Virtuales (CGyAV)

## Descripción del práctico
Este práctico corresponde a la **Guía de Prácticos Nº 2** de la materia, específicamente al **Práctico 3**【57†CGyAV-guia-practicos-02.pdf】:

> Combinar los programas 2.1 y 2.3 para lograr una animación en la que un eje vertical que pasa por el vértice superior del triángulo se mantenga alineado con la dirección radial mientras el triángulo se desplaza describiendo la trayectoria circular.

La idea es que el triángulo no solo gire sobre su vértice superior (como en el práctico 2.1), ni solo traslade dicho vértice en una circunferencia (como en el práctico 2.3), sino que ambos efectos se combinen: el triángulo debe **desplazarse a lo largo de la circunferencia** y, al mismo tiempo, **orientarse de forma que su eje vertical se alinee con el radio de la órbita**.

---

## Conceptos teóricos involucrados

El práctico aplica los contenidos vistos en clase sobre **transformaciones geométricas 2D**【53†CGyAV - Clase 04 - 18Ago25.pdf】:  
- **Traslación:** desplazar todos los puntos de un objeto una misma distancia en $x$ e $y$.  
- **Rotación:** girar el objeto un ángulo θ alrededor de un punto fijo.  
- **Composición de transformaciones:** aplicar varias transformaciones en secuencia, combinadas en una matriz compuesta.  
- **Rotación respecto a un punto arbitrario:** se realiza mediante el “sandwich” de traslaciones y rotaciones:  
  $M = T(p) \cdot R(θ) \cdot T(-p)$.

Además, se utilizan **coordenadas homogéneas (3x3 en 2D, 4x4 en OpenGL)** que permiten unificar traslaciones y transformaciones lineales en una sola multiplicación matricial【53†CGyAV - Clase 04 - 18Ago25.pdf】.

---

## Transformaciones utilizadas en el código

En el programa implementado, la matriz modelo final es:

$$
M = T(d(t) - p_{Top}) \cdot T(p_{Top}) \cdot R_z(\varphi) \cdot T(-p_{Top})
$$

### Explicación paso a paso:
1. **$T(-p_{Top})$**: traslado el triángulo de forma que el vértice superior (pivote) quede en el origen.  
   → Permite girar el triángulo alrededor de ese punto.

2. **$R_z(\varphi)$**: rotación en torno al eje Z (en el plano XY).  
   - $\varphi$ se calcula como:  
     $$\varphi = \text{atan2}(y_d - c_y, x_d - c_x) + \frac{\pi}{2}$$
   - Esto asegura que el eje vertical del triángulo quede alineado con el radio de la circunferencia.

3. **$T(p_{Top})$**: devuelvo el triángulo a su posición original (antes de la traslación inicial).  
   → Ahora el triángulo está rotado alrededor de su vértice superior.

4. **$T(d(t) - p_{Top})$**: muevo todo el triángulo para que el vértice superior esté en el punto $d(t)$ de la órbita.

---

## Cálculo de la trayectoria circular

El vértice superior del triángulo se mueve en una circunferencia de radio $R$ y centro $(c_x, c_y)$【54†Clase 02 - 06 Ago.pdf】:

$$d(t) = (c_x + R \cdot \cos(\omega t),\; c_y + R \cdot \sin(\omega t))$$

Donde:  
- $R$: radio de la órbita.  
- $\omega$: velocidad angular (rad/s).  
- $t$: tiempo transcurrido (escalado según velocidad o pausa).

Así, en cada frame:
- Se obtiene la nueva posición $d(t)$.  
- Se calcula el ángulo $\theta = atan2(d_y - c_y, d_x - c_x)$.  
- Se ajusta $\varphi = \theta + \pi/2$ para alinear el eje vertical con el radio.  
- Se arma la matriz $M$ y se dibuja el triángulo.

---

## Referencias en el código

- **Cálculo de $d(t)$** (línea ~173 en tu código):
  ```cpp
  const glm::vec2 d = center + gOrbit.R * glm::vec2(std::cos(gOrbit.w * t), std::sin(gOrbit.w * t));
  ```
- **Ángulo radial + alineación hacia afuera**:
  ```cpp
  const float theta = std::atan2(d.y - center.y, d.x - center.x);
  const float phi = theta + float(M_PI_2);
  ```
- **Construcción de la matriz modelo**:
  ```cpp
  const glm::mat4 T_toTop = glm::translate(I, glm::vec3(pTop, 0.0f));
  const glm::mat4 T_fromTop = glm::translate(I, glm::vec3(-pTop, 0.0f));
  const glm::mat4 R_about = glm::rotate(I, phi, glm::vec3(0, 0, 1));
  const glm::mat4 Mrot = T_toTop * R_about * T_fromTop;
  const glm::mat4 T_move = glm::translate(I, glm::vec3(d - pTop, 0.0f));
  const glm::mat4 M = T_move * Mrot;
  ```
- **Aplicación en el pipeline**:
  ```cpp
  shader.setMat4("uModel", &M[0][0]);
  triMesh.bind();
  triMesh.draw();
  ```

---

## Relación con los contenidos de clase
- El práctico conecta directamente con **rotación respecto a un punto arbitrario**【53†CGyAV - Clase 04 - 18Ago25.pdf】.  
- También aplica el concepto de **composición de transformaciones** y su importancia en el orden (no conmutativo).  
- Se basa en la **digitalización de circunferencias**【54†CGyAV - Clase 02 - 06 Ago.pdf】, pero aquí no dibujamos píxel a píxel sino que usamos las ecuaciones paramétricas para el movimiento.

---

## Controles implementados
- **ESPACIO**: pausar/continuar animación.  
- **R**: reiniciar tiempo.  
- **Q/E**: disminuir/aumentar velocidad angular.  
- **ESC**: cerrar ventana.

---

## Conclusión
Este práctico combina **traslación + rotación alrededor de un punto** para lograr una animación más compleja. El código implementa correctamente la secuencia de transformaciones y demuestra cómo los conceptos matemáticos de clase se traducen en instrucciones de OpenGL/GLM.  

Se refuerzan dos ideas fundamentales de las transformaciones en gráficos por computadora:
1. El orden de las operaciones es crítico.  
2. El uso de matrices homogéneas permite unificar traslaciones y rotaciones en un único pipeline eficiente.

