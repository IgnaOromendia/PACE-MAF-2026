#!/bin/sh

# Corta la ejecución si falla un comando o si se usa una variable no definida.
set -eu

# Verifica que se haya pasado exactamente un argumento:
# la carpeta dentro de input que contiene los archivos .nw.
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <input-folder>" >&2
    exit 1
fi

# Construye la ruta a la carpeta de entrada, por ejemplo: input/dataset.
input_dir="input/$1"

# Frena si la carpeta indicada no existe.
if [ ! -d "$input_dir" ]; then
    echo "Folder does not exist: $input_dir" >&2
    exit 1
fi

# Sirve para detectar el caso en que no haya archivos .nw en la carpeta.
found_instances=0

# Recorre todos los archivos .nw de la carpeta elegida.
for instance_path in "$input_dir"/*.nw; do
    # Si el patrón no encontró archivos, evita procesar el literal "*.nw".
    if [ ! -e "$instance_path" ]; then
        continue
    fi

    # Marca que al menos se encontró una instancia válida.
    found_instances=1

    # Extrae el nombre base del archivo sin la extensión .nw.
    name=$(basename "$instance_path" .nw)

    # Construye la ruta esperada para la solución correspondiente.
    solution_path="output/out_${name}.nw"

    # Si falta la solución, informa el problema y sigue con el próximo archivo.
    if [ ! -f "$solution_path" ]; then
        echo "Missing solution for '$name': $solution_path" >&2
        continue
    fi

    # Ejecuta el checker con la instancia y su solución asociada.
    echo "Checking $name"
    ./checker "$instance_path" "$solution_path"
done

# Si no se encontró ningún .nw en la carpeta, devuelve error.
if [ "$found_instances" -eq 0 ]; then
    echo "No .nw files found in $input_dir" >&2
    exit 1
fi
