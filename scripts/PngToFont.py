from PIL import Image
import numpy as np
import struct
import sys
import os

GLYPH_WIDTH : int = 0
GLYPH_HEIGHT : int = 0
NUM_CHARS : int = 0  # ASCII full set
GLYPHS_PER_ROW : int = 16
TOTAL_GLYPHS : int = 0

def extract_bitmap_data(img):
    # Threshold to convert grayscale to monochrome (black & white)
    width, height = img.size
    
    data = []
    if height > GLYPH_HEIGHT:
        threshold = 128
        img_bw = img.point(lambda p: 1 if p < threshold else 0, mode='1')

        # Extract each glyph
        for glyph_index in range(TOTAL_GLYPHS):
            row = glyph_index // GLYPHS_PER_ROW
            col = glyph_index % GLYPHS_PER_ROW

            for y in range(GLYPH_HEIGHT):
                byte = 0
                for x in range(GLYPH_WIDTH):
                    pixel_x = col * GLYPH_WIDTH + x
                    pixel_y = row * GLYPH_HEIGHT + y
                    pixel = img_bw.getpixel((pixel_x, pixel_y))
                    byte = (byte << 1) | (1 if pixel == 0 else 0)  # 0 is black ("on")
                data.append(byte)
                
    elif height == GLYPH_HEIGHT:
        for ch in range(NUM_CHARS):
            x_offset = ch * GLYPH_WIDTH
            for y in range(GLYPH_HEIGHT):
                byte = 0
                for x in range(GLYPH_WIDTH):
                    pixel = img.getpixel((x_offset + x, y))
                    if img.mode == 'L':  # grayscale
                        pixel = 0 if pixel < 128 else 255
                    if pixel == 0:  # black = set pixel
                        byte |= (1 << (7 - x))
                data.append(byte)
                
    return data

def format_nasm_font(data, out_file):
    with open(out_file, 'w') as f:
        f.write(f"; !!! THIS FILE IS GENERATED USING THE PngToFont TOOL !!!{os.linesep}")
        f.write(f"; This file format 1 Glyph is a {GLYPH_HEIGHT} byte section{os.linesep}")
        f.write(f"section .font{os.linesep}")
        f.write(f"%macro GLYPH {GLYPH_HEIGHT}{os.linesep}")
        chunk = ', '.join(f"%{b}" for b in range(1, GLYPH_HEIGHT + 1))
        f.write(f"\tdb {chunk}{os.linesep}")
        f.write(f"%endmacro{os.linesep}")
        f.write(f"{os.linesep}")
        f.write(f"global default8x16Font{os.linesep}")
        f.write(f"default8x16Font:{os.linesep}")
        f.write(f"\t; metadata{os.linesep}")
        f.write(f"\tdb 0 ; number of extra glyphs for metadata{os.linesep}")
        f.write(f"\tdb \"F0.1\" ; version{os.linesep}")
        f.write(f"\tdb 0 ; reserved{os.linesep}")
        f.write(f"\tdb {GLYPH_WIDTH}, {GLYPH_HEIGHT} ; glyph width and height{os.linesep}")
        f.write(f"\tdw {NUM_CHARS}; the number of glyphs{os.linesep}")
        f.write(f"\ttimes 6 db 0x00{os.linesep}")
        f.write(f"{os.linesep}")
        f.write(f"fontData:{os.linesep}")
        for i in range(0, len(data), 16):
            glyph_bytes = data[i:i + GLYPH_HEIGHT]
            # Pad in case data is incomplete
            glyph_bytes += [0x00] * (GLYPH_HEIGHT - len(glyph_bytes))
            hex_bytes = ', '.join(f"0x{b:02x}" for b in glyph_bytes)
            f.write(f"    ; char {i//16:03d}{os.linesep}")
            f.write(f"    GLYPH {hex_bytes}{os.linesep}")

def main():
    if len(sys.argv) != 4:
        print("Usage: png2font.py <width>x<height>x<numchars> input.png output.asm")
        sys.exit(1)

    fontFormat = sys.argv[1].lower().split('x')
    if len(fontFormat) != 3:
        print("Font format must be in the form <width>x<height>x<numchars>")
        sys.exit(1)

    global GLYPH_WIDTH, GLYPH_HEIGHT, NUM_CHARS, TOTAL_GLYPHS, GLYPHS_PER_ROW
    GLYPH_WIDTH = int(fontFormat[0])
    GLYPH_HEIGHT = int(fontFormat[1])
    NUM_CHARS = int(fontFormat[2])
    TOTAL_GLYPHS = NUM_CHARS
    GLYPHS_PER_ROW = NUM_CHARS // GLYPH_HEIGHT
    # Placeholder test using a generated blank image since user didn't upload PNG
    # Replace with actual PNG for real use
    img = Image.open(sys.argv[2]).convert('L')
    data = extract_bitmap_data(img)
    format_nasm_font(data, sys.argv[3])

if __name__ == '__main__':
    main()