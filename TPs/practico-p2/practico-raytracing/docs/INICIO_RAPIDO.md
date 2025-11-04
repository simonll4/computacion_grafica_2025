# 🚀 Inicio Rápido - Ray Tracer

## Compilación

```bash
make
```

## Ejecución

### 🎯 Formas de usar el Ray Tracer

#### 1. Menú Interactivo (Recomendado)
```bash
make run
```
Te mostrará un menú para elegir qué escena renderizar.

#### 2. Escenas Individuales
```bash
make scene1    # Escena principal (esfera + cubo + luces)
make scene2    # Test de sombras
make scene3    # Test de materiales (shininess)
make scene4    # Test de atenuación
make scene5    # Test de múltiples luces
```

#### 3. Renderizar Todas las Escenas
```bash
make all-scenes
```

#### 4. Ejecutable Directo
```bash
./build/RayTracer          # Menú interactivo
./build/RayTracer 1        # Escena específica
```

#### 5. Script Rápido
```bash
./run.sh          # Menú
./run.sh all      # Todas las escenas
```

## Resultados

Las imágenes se guardan en la carpeta `results/`:
- `results/escena_principal.bmp/ppm`
- `results/test_sombras.bmp/ppm`
- `results/test_materiales.bmp/ppm`
- `results/test_atenuacion.bmp/ppm`
- `results/test_multiples_luces.bmp/ppm`

## Ver las Imágenes

```bash
# Linux
eog results/*.bmp
xdg-open results/escena_principal.bmp

# Conversión a PNG
convert results/escena_principal.bmp results/escena_principal.png
```

## Limpiar

```bash
make clean              # Limpia binarios
make clean-results      # Limpia imágenes en results/
```

## Ayuda Rápida

```bash
make               # Compila
make run           # Ejecuta con menú
make test          # Test rápido (escena 1)
make all-scenes    # Todas las escenas
make clean         # Limpia
```

## Troubleshooting

### Error: No rule to make target 'build/glad.o'
**Solución**: Ya está corregido. Usa `make` directamente.

### Error: undefined reference to 'omp_get_num_threads'
**Solución**: Instalar OpenMP:
```bash
sudo apt-get install libomp-dev
```

### Error: glm/glm.hpp: No such file
**Solución**: Instalar GLM:
```bash
sudo apt-get install libglm-dev
```

---

## 🎨 Ejemplo Completo

```bash
# 1. Compilar
make

# 2. Ejecutar escena principal
make scene1

# 3. Ver resultado
xdg-open escena_principal.bmp

# ✨ ¡Listo!
```

---

Para más detalles, consulta:
- `README.md` - Documentación completa
- `EJEMPLOS_USO.md` - Personalización
- `IMPLEMENTACION.md` - Detalles técnicos
