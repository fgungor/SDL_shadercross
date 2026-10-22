Sparse graphics resource regression tests preserve register numbers and check
the slot spans shared by JSON reflection and SDL shader creation:

- `t4/s4` and `t6/s6`: seven sampler slots.
- Storage texture `t7` and buffer `t8`: spans `7 / 1 / 1`.
- Storage texture `t9` and buffer `t12`: spans `7 / 3 / 3`.
- Arrays at `t6/s6` of length two: eight sampler slots.
- `b0` and `b3`: four uniform slots.

The tests also cover shared textures/samplers, overlapping storage ranges, and
Metal storage-buffer indices. Vendored builds compile an additional SPIR-V
fixture directly with DXC without flattening resource arrays. Its last array
element is unused, so reflection must use the declared array length.

`shadercross_SparseSamplerD3D12` creates real graphics pipelines for each valid
layout. It skips when D3D12 is unavailable. To require the GPU test on Windows:

```powershell
$env:SDLSHADERCROSS_REQUIRE_D3D12 = '1'
ctest --test-dir <build-directory> --output-on-failure
```

The CLI JSON tests run when the CLI and DXC support are enabled. All fixtures
are generated in the build directory.
