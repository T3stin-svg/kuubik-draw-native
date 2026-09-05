#!/usr/bin/env python3
"""Corrupt a real Qt probe in memory: each misleading PDF must be rejected."""
import importlib.util
from io import BytesIO
from pathlib import Path
import sys

from pypdf import PdfReader, PdfWriter, Transformation
from pypdf.generic import DecodedStreamObject, NameObject, NumberObject, RectangleObject

spec = importlib.util.spec_from_file_location("preview_verifier", Path(__file__).with_name("verify-preview-outputs.py"))
verifier = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verifier)
source = Path(sys.argv[1])
verifier.verify_viewport_pdf_probe(PdfReader(source))
for case in ("emptyClip", "doubledUnits", "rotatedPage", "croppedPage", "doubledGeometry", "mirroredGeometry"):
    writer = PdfWriter()
    writer.add_page(PdfReader(source).pages[0])
    page = writer.pages[0]
    if case == "emptyClip":
        stream = DecodedStreamObject()
        stream.set_data(b"0 0 0 0 re W n\n" + page.get_contents().get_data())
        page[NameObject("/Contents")] = stream
    elif case == "doubledUnits":
        page[NameObject("/UserUnit")] = NumberObject(2)
    elif case == "rotatedPage":
        page.rotate(90)
    elif case == "croppedPage":
        page.cropbox = RectangleObject((0, 0, 10, 10))
    else:
        page.add_transformation(Transformation().scale(2 if case == "doubledGeometry" else -1, 2 if case == "doubledGeometry" else 1))
    buffer = BytesIO()
    writer.write(buffer)
    buffer.seek(0)
    try:
        verifier.verify_viewport_pdf_probe(PdfReader(buffer))
    except RuntimeError:
        print("PASS rejected", case)
    else:
        raise RuntimeError("Verifier accepted " + case)
