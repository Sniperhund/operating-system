python

import gdb
import os

PROGRAMS_DIR = os.path.abspath("../programs/binaries")

class SymbolLoadBreakpoint(gdb.Breakpoint):
    def __init__(self):
        # Break on the notification function.
        # 'internal' keeps it out of the 'info breakpoints' noise.
        super().__init__("loadDebugSymbols", internal=False)
        self.silent = True

    def stop(self):
        try:
            # Read arguments: (const char* path, unsigned int text_base)
            frame     = gdb.newest_frame()
            path_ptr  = frame.read_var("path")
            text_base = int(frame.read_var("textBase"))

            path = path_ptr.string()          # dereference char*
            # Strip leading slash and build local path
            # e.g. "/bin/test.elf" -> programs/binaries/test.elf
            basename = os.path.basename(path)
            name, _  = os.path.splitext(basename)
            local    = os.path.join(PROGRAMS_DIR, f"{name}.elf")

            if os.path.exists(local):
                print(f"\n[gdb] Loading symbols: {local}  text=0x{text_base:x}")
                gdb.execute(f"add-symbol-file {local} 0x{text_base:x}", to_string=True)
                print(f"[gdb] Symbols loaded. You can now set breakpoints in {basename}.")
            else:
                print(f"\n[gdb] SYMLOAD: {path} @ 0x{text_base:x}  (no local ELF found at {local})")

        except Exception as e:
            print(f"[gdb] SymbolLoad handler error: {e}")

        # Return False = don't stop execution, just load symbols transparently
        return False

end

# Connect to QEMU GDB stub
target remote localhost:1234

# Load kernel symbols
symbol-file iso_root/boot/kernel.elf

# Arm the breakpoint
python SymbolLoadBreakpoint()

# Optional: break at kernel_main so you can step from the start
# break kernel_main

echo [gdb] Ready. Symbol auto-loading is active.\n