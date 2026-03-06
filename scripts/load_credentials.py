"""
PlatformIO extra_script: Load WiFi credentials from credentials.h if present.

If credentials.h exists in the project root, its -D flags are appended
to build_flags. Without it the ESP starts in AP mode.
"""
import os

Import("env")

credentials_file = os.path.join(env.get("PROJECT_DIR"), "credentials.h")
if os.path.isfile(credentials_file):
    print(f"  [credentials] Loading {credentials_file}")
    env.Append(CPPDEFINES=[("HAS_CREDENTIALS", 1)])
    env.Append(BUILD_FLAGS=[f'-include{credentials_file}'])
else:
    print("  [credentials] No credentials.h found - ESP will start in AP mode")
