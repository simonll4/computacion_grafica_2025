from PIL import Image
import os
import sys

def convert_ppm_to_png(input_path, output_path=None):
    # Si no se especifica salida, cambia la extensión automáticamente
    if output_path is None:
        base = os.path.splitext(input_path)[0]
        output_path = base + ".png"

    try:
        img = Image.open(input_path)
        img.save(output_path, "PNG")
        print(f"✅ Convertido: {input_path} → {output_path}")
    except Exception as e:
        print(f"❌ Error convirtiendo {input_path}: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python ppm_to_png.py archivo.ppm")
        sys.exit(1)

    input_file = sys.argv[1]
    convert_ppm_to_png(input_file)
