"""Adds the constructor/destructor declarations that the module headers omit.

Several Aurora managers define `X::X()` / `X::~X()` in their .cpp while the
class body never declares them, which MSVC rejects as C2600 (you cannot
define an implicitly-generated special member function out of line). The
declarations are inserted into the existing `public:` section so the
definitions become valid.
"""
import pathlib
import re

MODS = pathlib.Path(__file__).resolve().parent.parent / "modules"

CLASS_RE = re.compile(r"class\s+(\w+Manager)\b[^{;]*\{")


def fix(header: pathlib.Path) -> str | None:
    text = header.read_text(encoding="utf-8", errors="surrogateescape")

    match = CLASS_RE.search(text)
    if not match:
        return None
    cls = match.group(1)

    body_start = match.end()
    body_end = text.find("\n};", body_start)
    if body_end == -1:
        return None
    body = text[body_start:body_end]

    has_ctor = re.search(rf"^\s*{cls}\s*\([^;]*\)\s*(const\s*)?[;{{]", body, re.M)
    has_dtor = re.search(rf"^\s*~{cls}\s*\(\s*\)\s*[;{{]", body, re.M)
    if has_ctor and has_dtor:
        return None

    additions = []
    if not has_ctor:
        additions.append(f"    {cls}();")
    if not has_dtor:
        additions.append(f"    ~{cls}();")

    # Put the declarations right after the first access specifier so they sit
    # in the same section as the rest of the public interface.
    anchor = re.search(r"^\s*public:\s*$", body, re.M)
    if anchor:
        insert_at = body_start + anchor.end()
        body_new = body[:anchor.end()] + "\n" + "\n".join(additions) + body[anchor.end():]
    else:
        insert_at = body_start
        body_new = "\n" + "\n".join(additions) + body

    header.write_text(text[:body_start] + body_new + text[body_end:],
                      encoding="utf-8", errors="surrogateescape")
    return cls


def main():
    for header in sorted(MODS.glob("*/src/*Module.h")):
        cls = fix(header)
        print(f"  {header.parts[-3]:22s} {cls or 'ok'}")


if __name__ == "__main__":
    main()
