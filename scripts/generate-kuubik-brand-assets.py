#!/usr/bin/env python3
"""Generate deterministic Kuubik Draw raster and Windows icon assets.

The design is original to this fork. It does not reuse LibreCAD or Autodesk
branding. Pillow is needed only by maintainers regenerating committed assets;
it is not a Kuubik Draw runtime dependency.
"""

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "librecad" / "res" / "main"
SCALE = 4

BG = "#111820"
PANEL = "#172631"
BLUE = "#168DCE"
CYAN = "#4CC2FF"
WHITE = "#F2F7FA"
MUTED = "#9FB4C2"


def font(size: int, bold: bool = False) -> ImageFont.ImageFont:
    names = [
        "C:/Windows/Fonts/segoeuib.ttf" if bold else "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arialbd.ttf" if bold else "C:/Windows/Fonts/arial.ttf",
    ]
    for name in names:
        if Path(name).exists():
            return ImageFont.truetype(name, size)
    return ImageFont.load_default()


def cube(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], width: int) -> None:
    x0, y0, x1, y1 = box
    cx = (x0 + x1) // 2
    top = (cx, y0)
    left = (x0, y0 + (y1 - y0) // 3)
    right = (x1, left[1])
    bottom = (cx, y1)
    middle = (cx, y0 + (y1 - y0) * 2 // 3)
    draw.polygon([top, right, bottom, left], fill=PANEL)
    draw.line([top, right, bottom, left, top], fill=BLUE, width=width, joint="curve")
    draw.line([left, middle, right], fill=CYAN, width=max(2, width * 2 // 3), joint="curve")
    draw.line([top, middle, bottom], fill=CYAN, width=max(2, width * 2 // 3), joint="curve")


def icon_canvas(size: int) -> Image.Image:
    hi = size * SCALE
    image = Image.new("RGBA", (hi, hi), BG)
    draw = ImageDraw.Draw(image)
    radius = int(hi * 0.19)
    draw.rounded_rectangle((0, 0, hi - 1, hi - 1), radius=radius, fill=BG)
    cube(draw, (int(hi * 0.18), int(hi * 0.14), int(hi * 0.84), int(hi * 0.88)), max(4, int(hi * 0.045)))
    kx = int(hi * 0.31)
    y0 = int(hi * 0.34)
    ym = int(hi * 0.53)
    y1 = int(hi * 0.70)
    draw.line((kx, y0, kx, y1), fill=WHITE, width=max(5, int(hi * 0.052)))
    draw.line((kx, ym, int(hi * 0.52), y0), fill=WHITE, width=max(5, int(hi * 0.052)))
    draw.line((kx, ym, int(hi * 0.54), y1), fill=WHITE, width=max(5, int(hi * 0.052)))
    return image.resize((size, size), Image.Resampling.LANCZOS)


def splash() -> Image.Image:
    size = (544, 338)
    image = Image.new("RGB", size, BG)
    draw = ImageDraw.Draw(image)
    draw.rounded_rectangle((18, 18, 526, 320), radius=24, fill=PANEL, outline=BLUE, width=2)
    cube(draw, (55, 55, 245, 282), 7)
    draw.text((278, 80), "KUUBIK", font=font(39, True), fill=WHITE)
    draw.text((278, 126), "DRAW", font=font(39, True), fill=CYAN)
    draw.line((278, 184, 483, 184), fill=BLUE, width=3)
    draw.text((278, 202), "Native 2D CAD", font=font(20, False), fill=WHITE)
    draw.text((278, 238), "0.2.0-preview.2", font=font(15, True), fill=MUTED)
    draw.text((278, 266), "LibreCAD 2.2.1.5 base · GPLv2", font=font(12, False), fill=MUTED)
    return image


def intro() -> Image.Image:
    image = Image.new("RGB", (73, 335), BG)
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, 72, 334), outline=BLUE, width=2)
    cube(draw, (10, 18, 63, 86), 3)
    draw.line((36, 104, 36, 299), fill=BLUE, width=2)
    for y in (132, 184, 236, 288):
        draw.ellipse((30, y - 6, 42, y + 6), fill=CYAN)
    return image


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    icon = icon_canvas(256)
    icon.resize((128, 128), Image.Resampling.LANCZOS).save(OUT / "kuubikdraw.png")
    icon.save(
        OUT / "kuubikdraw.ico",
        sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)],
    )
    splash().save(OUT / "splash_kuubikdraw.png", optimize=True)
    intro().save(OUT / "intro_kuubikdraw.png", optimize=True)


if __name__ == "__main__":
    main()
