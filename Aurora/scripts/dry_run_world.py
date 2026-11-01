"""Dry-run of worldserver on this machine.

Purpose is to verify the Aurora module wiring, not to reach the realm loop: the
server stops at the DBC check (the 3.3.5a client files are not extracted here),
which happens *after* LoadModulesConfigs() and after the script registries are
built. Everything the Aurora modules depend on is therefore exercised.

worldserver reads configs/worldserver.conf relative to its own directory, so no
command line options are needed; stdin is closed to keep it out of the console
loop.
"""
import os
import subprocess
import sys

BIN = r"E:\azerothcore-wotlk-master\build\bin\RelWithDebInfo"

proc = subprocess.Popen(
    [os.path.join(BIN, "worldserver.exe")],
    cwd=BIN,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    stdin=subprocess.DEVNULL,
    text=True,
    encoding="utf-8",
    errors="replace",
)

try:
    out, _ = proc.communicate(timeout=120)
except subprocess.TimeoutExpired:
    proc.kill()
    out, _ = proc.communicate()

print(out)
print("=== exit code:", proc.returncode, file=sys.stderr)
