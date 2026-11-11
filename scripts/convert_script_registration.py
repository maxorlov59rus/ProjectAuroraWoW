"""
One-shot conversion: static script instances -> Add<Module>Scripts().

AzerothCore creates module scripts inside AddModulesScripts(), which the core
calls from sScriptMgr->Initialize() after its own statics are constructed.
Registering through file-scope objects instead runs in _initterm, before
ScriptRegistry's own statics exist, and worldserver dies with an access
violation in ScriptRegistry<T>::_checkMemory.

So every

    namespace
    {
        FooCommandScript sFooCommandScript;
    }

declaration is dropped and the scripts are created in the loader function the
generated ModulesLoader.cpp already declares and calls:

    void Add<Module>Scripts()
    {
        new FooCommandScript();
    }

Run:  python scripts/convert_script_registration.py
Then: python scripts/migrate_to_ac.py
"""
import re
import sys
from pathlib import Path

MODULES = Path(__file__).resolve().parent.parent / "modules"

# The empty stub migrate_to_ac.py appends when a source has no loader yet.
STUB_RE = re.compile(
    r"\n// =+\n"
    r"// AzerothCore module loader entry point\.\n"
    r"(?://[^\n]*\n)*"
    r"// =+\n+\n"
    r"void Add\w+Scripts\(\)\n\{\n\}\n\Z"
)

NAMESPACE_RE = re.compile(r"^\s*namespace(?:\s+(\w+))?\s*(\{)?")
INSTANCE_RE = re.compile(r"^(\s*)([A-Za-z]\w*Script)\s+(s\w+);\s*$")


def scan(lines):
    """Yields (index, line, namespace_prefix) for every line of the file.

    The prefix is tracked by brace depth rather than by counting closing
    braces: '};' of a class body must not be mistaken for the end of the
    enclosing namespace. A namespace is therefore active for every line whose
    depth is at least the depth it opened at.

    Anonymous namespaces are left out of the prefix: their members are visible
    in the enclosing namespace through the implicit using-directive, so
    'Aurora::FooScript' names a class declared in 'namespace Aurora { namespace
    { ... } }'.
    """
    opened, depth = [], 0
    for idx, line in enumerate(lines):
        match = NAMESPACE_RE.match(line)
        brace_on_next = False
        if match and match.group(2) is None:  # 'namespace Aurora' / brace on the next line
            for nxt in lines[idx + 1 :]:
                if nxt.strip():
                    brace_on_next = nxt.strip().startswith("{")
                    break

        before = depth
        depth += line.count("{") - line.count("}")
        if match and (match.group(2) or brace_on_next):
            opened.append((before + 1, match.group(1)))
        opened[:] = [entry for entry in opened if entry[0] <= depth]

        yield idx, line, "".join("%s::" % name for _, name in opened if name)


def convert(path: Path) -> None:
    module = path.parents[1].name if path.parent.name == "src" else path.parent.name
    loader = "Add%sScripts" % module.replace("-", "_")

    text = path.read_text(encoding="utf-8")
    if re.search(r"void\s+%s\s*\(\)\s*\{\s*\n\s*new\s" % loader, text):
        print(f"  skip (already converted): {path.name}")
        return

    stub = STUB_RE.search(text)
    if stub:
        text = text[: stub.start()] + "\n"

    lines = text.splitlines()
    drop, scripts = set(), []
    for idx, line, prefix in scan(lines):
        match = INSTANCE_RE.match(line)
        if not match:
            continue
        drop.add(idx)
        scripts.append(prefix + match.group(2))

    if not scripts:
        print(f"  {path.name}: no static script instances")
        return

    # Blank out the instance lines, then drop anonymous namespaces left empty.
    for idx in drop:
        lines[idx] = ""
    for start, _line, _ in list(scan(lines)):
        if lines[start].strip() not in ("namespace", "namespace {"):
            continue
        depth, opened, end = 0, False, None
        for i in range(start, len(lines)):
            depth += lines[i].count("{") - lines[i].count("}")
            opened = opened or depth > 0
            if opened and depth == 0:
                end = i
                break
        if end is None or any(l.strip() for l in lines[start + 1 : end]):
            continue
        del lines[start:end]
        break  # indices shifted; one empty block per pass is enough

    body = "\n".join("    new %s();" % name for name in scripts)
    kept = "\n".join(l for l in lines if l is not None)
    text = "\n".join(kept.splitlines()).rstrip() + """

// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void %s()
{
%s
}
""" % (loader, body)

    path.write_text(text, encoding="utf-8")
    print(f"  {path.name}: {loader}() <- {', '.join(scripts)}")


def main():
    paths = sorted(set(MODULES.glob("*/*/*.cpp")) | set(MODULES.glob("*/*.cpp")))
    for path in paths:
        convert(path)


if __name__ == "__main__":
    sys.exit(main())
