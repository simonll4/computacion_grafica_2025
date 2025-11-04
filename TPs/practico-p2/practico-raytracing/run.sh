#!/bin/bash

# Script para compilar y ejecutar el Ray Tracer
# Computación Gráfica - Práctico 2

echo "╔════════════════════════════════════════╗"
echo "║     RAY TRACER - COMPILAR Y EJECUTAR   ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Compilar si es necesario
if [ ! -f "build/RayTracer" ] || find src -type f \( -name "*.cpp" -o -name "*.h" \) -newer build/RayTracer | grep -q .; then
    echo "🔨 Compilando ray tracer..."
    make clean > /dev/null 2>&1
    make
    if [ $? -ne 0 ]; then
        echo "❌ Error en la compilación"
        exit 1
    fi
    echo "✓ Compilación exitosa"
    echo ""
fi

# Ejecutar
if [ "$1" == "all" ]; then
    echo "Renderizando todas las escenas..."
    echo "0" | ./build/RayTracer
elif [ "$1" != "" ]; then
    ./build/RayTracer "$1"
else
    ./build/RayTracer
fi

# Mostrar las imágenes generadas
echo ""
echo "📸 Imágenes generadas en results/:"
ls -lh results/*.bmp 2>/dev/null | awk '{print "   - " $9 " (" $5 ")"}'
echo ""
echo "✨ Proceso completado!"
