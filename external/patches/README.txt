spirv-tools-preserve-interface.patch fixes SDL_shadercross issue #169 in the
SPIRV-Tools legalization pass used by DXC. It preserves unused user-defined
stage inputs and outputs when requested, while still removing unused built-ins
and resource variables. Resource-array flattening remains enabled.

Vendored DXC builds apply this patch automatically during CMake configuration.
The patch is idempotent and leaves the SPIRV-Tools submodule modified locally.
Do not discard these changes while a vendored build is in use.

Non-vendored builds must use a DXC library built with the same fix in its
SPIRV-Tools dependency; replacing SDL_shadercross alone cannot fix that library.
Apply the patch from the root of DXC's SPIRV-Tools source directory:

    git apply /path/to/spirv-tools-preserve-interface.patch

Then rebuild DXC and configure SDL_shadercross to link that dxcompiler library.
