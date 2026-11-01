Import("env")

import struct
from pathlib import Path


project_directory = Path(env.subst("$PROJECT_DIR"))
build_directory = Path(env.subst("$BUILD_DIR"))
logo_png_path = project_directory / "logo" / "SondAris_320x240.png"
generated_header_directory = build_directory / "generated"
generated_header_path = generated_header_directory / "SondArisPngData.h"

if not logo_png_path.is_file():
    raise RuntimeError("Impossibile trovare logo/SondAris_320x240.png.")

logo_png_bytes = logo_png_path.read_bytes()
png_signature = b"\x89PNG\r\n\x1a\n"
if not logo_png_bytes.startswith(png_signature) or len(logo_png_bytes) < 24:
    raise RuntimeError("Il file SondAris_320x240.png non e un PNG valido.")

logo_width_pixels, logo_height_pixels = struct.unpack(">II", logo_png_bytes[16:24])
if (logo_width_pixels, logo_height_pixels) != (320, 240):
    raise RuntimeError(
        "Il logo PNG deve essere 320x240 pixel; trovato "
        f"{logo_width_pixels}x{logo_height_pixels}."
    )

png_byte_rows = []
for byte_offset in range(0, len(logo_png_bytes), 16):
    png_byte_row = logo_png_bytes[byte_offset:byte_offset + 16]
    png_byte_rows.append(
        "    " + ", ".join(f"0x{png_byte:02X}" for png_byte in png_byte_row) + ","
    )

generated_header_text = (
    "/* File generato da tools/embed_logo_png.py; non modificare a mano. */\n"
    "#pragma once\n"
    "#include <Arduino.h>\n\n"
    "static const uint8_t sondarisLogoPngData[] PROGMEM = {\n"
    + "\n".join(png_byte_rows)
    + "\n};\n"
)

generated_header_directory.mkdir(parents=True, exist_ok=True)
if not generated_header_path.is_file() or generated_header_path.read_text(
    encoding="ascii"
) != generated_header_text:
    with generated_header_path.open("w", encoding="ascii", newline="\n") as generated_header_file:
        generated_header_file.write(generated_header_text)

env.Append(CPPPATH=[str(generated_header_directory)])
