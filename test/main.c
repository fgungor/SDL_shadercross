#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <SDL3/SDL_test.h>

#include "shaders/hinc/simple.vert.hlsl.h"

static int SDLCALL shadercross_testInitQuit(void *args)
{
    SDL_GPUShaderFormat hlsl_formats;
    SDL_GPUShaderFormat spirv_formats;
    (void) args;

    hlsl_formats = SDL_ShaderCross_GetHLSLShaderFormats();
    spirv_formats = SDL_ShaderCross_GetSPIRVShaderFormats();
    SDLTest_AssertPass("                                         input");
    SDLTest_AssertPass("                                      HLSL  SPIRV");
    SDLTest_AssertPass("       SDL_GPU_SHADERFORMAT_PRIVATE      %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_PRIVATE), !!(spirv_formats & SDL_GPU_SHADERFORMAT_PRIVATE));
    SDLTest_AssertPass("       SDL_GPU_SHADERFORMAT_DXBC         %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_DXBC), !!(spirv_formats & SDL_GPU_SHADERFORMAT_DXBC));
    SDLTest_AssertPass("output SDL_GPU_SHADERFORMAT_DXIL         %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_DXIL), !!(spirv_formats & SDL_GPU_SHADERFORMAT_DXIL));
    SDLTest_AssertPass("       SDL_GPU_SHADERFORMAT_MSL          %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_MSL), !!(spirv_formats & SDL_GPU_SHADERFORMAT_MSL));
    SDLTest_AssertPass("       SDL_GPU_SHADERFORMAT_METALLIB     %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_METALLIB), !!(spirv_formats & SDL_GPU_SHADERFORMAT_METALLIB));
    return TEST_COMPLETED;
}

static int SDLCALL shadercross_CompileHLSL_to_XXX(void *args)
{
    size_t i;
    struct {
        const char *formatname;
        const char *funcname;
        void * (SDLCALL * compile_XXXFromHLSL)(const SDL_ShaderCross_HLSL_Info *, size_t *);
        SDL_GPUShaderFormat format;
    } cases[] = {
        { "DXBC", "SDL_ShaderCross_CompileDXBCFromHLSL", SDL_ShaderCross_CompileDXBCFromHLSL, SDL_GPU_SHADERFORMAT_DXBC },
        { "DXIL", "SDL_ShaderCross_CompileDXILFromHLSL", SDL_ShaderCross_CompileDXILFromHLSL, SDL_GPU_SHADERFORMAT_DXIL },
        { "SPIRV", "SDL_ShaderCross_CompileSPIRVFromHLSL", SDL_ShaderCross_CompileSPIRVFromHLSL, SDL_GPU_SHADERFORMAT_SPIRV },
    };

    (void)args;
    for (i = 0; i < SDL_arraysize(cases); i++) {
        SDL_ShaderCross_HLSL_Info hlsl_info;
        SDL_ShaderCross_HLSL_Define hlsl_defines[2];
        void *shader;
        size_t shader_size;

        SDLTest_AssertPass("Compiling HLSL -> %s", cases[i].formatname);

        if (!(SDL_ShaderCross_GetHLSLShaderFormats() & cases[i].format)) {
            SDLTest_Log("SDL_shadercross does not support HLSL -> %s", cases[i].formatname);
            continue;
        }

        SDLTest_AssertPass("Compile a valid HLSL vertex shader to %s", cases[i].formatname);
        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.entrypoint = "main";
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        shader_size = 0;
        shader = cases[i].compile_XXXFromHLSL(&hlsl_info, &shader_size);
        SDLTest_AssertCheck(shader != NULL, "%s should return a valid compiled DXBC shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size != 0, "Size of shader returned by %s should be size > 0", cases[i].funcname);
        SDL_free(shader);
        SDL_ClearError();

        SDLTest_AssertPass("Compile a valid HLSL vertex shader to %s with Debug enabled", cases[i].formatname);
        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.entrypoint = "main";
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.props = SDL_CreateProperties();
        SDL_SetBooleanProperty(hlsl_info.props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_ENABLE_BOOLEAN, true);
        SDL_SetStringProperty(hlsl_info.props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_NAME_STRING, "Simple shader");
        shader_size = 0;
        shader = cases[i].compile_XXXFromHLSL(&hlsl_info, &shader_size);
        SDLTest_AssertCheck(shader != NULL, "%s should return a valid compiled DXBC shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size != 0, "Size of shader returned by %s should be size > 0", cases[i].funcname);
        SDL_free(shader);
        SDL_DestroyProperties(hlsl_info.props);
        SDL_ClearError();

        SDLTest_AssertPass("Break a HLSL vertex shader by defining a macro");
        SDL_zero(hlsl_info);
        SDL_zero(hlsl_defines);
        hlsl_defines[0].name = "BREAK_SHADER";
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.entrypoint = "main";
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.defines = hlsl_defines;
        shader_size = 0;
        shader = cases[i].compile_XXXFromHLSL(&hlsl_info, &shader_size);
        SDLTest_AssertCheck(shader == NULL, "%s should fail when getting an invalid shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size == 0, "Size of shader returned by %s should be == 0", cases[i].funcname);
        SDL_free(shader);
        SDL_ClearError();
    }
    return TEST_COMPLETED;
}

static int SDLCALL shadercross_CompileSPIRV_to_XXX(void *args)
{
    size_t i;
    void *spirv_shader;
    size_t spirv_shader_size;
    struct {
        const char *formatname;
        const char *funcname;
        void * (SDLCALL * compile_XXXFromSPIRV)(const SDL_ShaderCross_SPIRV_Info *, size_t *);
        SDL_GPUShaderFormat format;
    } cases[] = {
        { "DXBC", "SDL_ShaderCross_CompileDXBCFromSPIRV", SDL_ShaderCross_CompileDXBCFromSPIRV, SDL_GPU_SHADERFORMAT_DXBC },
        { "DXIL", "SDL_ShaderCross_CompileDXBCFromSPIRV", SDL_ShaderCross_CompileDXILFromSPIRV, SDL_GPU_SHADERFORMAT_DXIL },
    };

    (void)args;

    {
        SDL_ShaderCross_HLSL_Info hlsl_info;

        SDLTest_AssertPass("Prepare SPIRV vertex shader (HLSL -> SPIRV)");

        if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_SPIRV)) {
            SDLTest_AssertPass("SDL_ShaderCross does not support HLSL -> SPIRV");
            return TEST_SKIPPED;
        }

        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.entrypoint = "main";
        spirv_shader = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_shader_size);
        SDLTest_AssertCheck(spirv_shader != NULL, "SDL_ShaderCross_CompileSPIRVFromHLSL must return a non-NULL shader (%s)", SDL_GetError());
        if (spirv_shader == NULL) {
            return TEST_ABORTED;
        }
    }

    for (i = 0; i < SDL_arraysize(cases); i++) {
        SDL_ShaderCross_SPIRV_Info spirv_info;
        void *shader;
        size_t shader_size;

        SDLTest_AssertPass("Compiling SPIRV -> %s", cases[i].formatname);

        if (!(SDL_ShaderCross_GetSPIRVShaderFormats() & cases[i].format)) {
            SDLTest_Log("SDL_shadercross does not support SPIRV -> %s", cases[i].formatname);
            continue;
        }

        SDLTest_AssertPass("Compile a valid HLSL vertex shader to %s", cases[i].formatname);
        SDL_zero(spirv_info);
        spirv_info.bytecode = spirv_shader;
        spirv_info.bytecode_size = spirv_shader_size;
        spirv_info.entrypoint = "main";
        spirv_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        shader_size = 0;
        shader = cases[i].compile_XXXFromSPIRV(&spirv_info, &shader_size);
        SDLTest_AssertCheck(shader != NULL, "%s should return a valid compiled DXBC shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size != 0, "Size of shader returned by %s should be size > 0", cases[i].funcname);
        SDL_free(shader);
        SDL_ClearError();

        SDLTest_AssertPass("Compile a valid HLSL vertex shader to %s with Debug enabled", cases[i].formatname);
        SDL_zero(spirv_info);
        spirv_info.bytecode = spirv_shader;
        spirv_info.bytecode_size = spirv_shader_size;
        spirv_info.entrypoint = "main";
        spirv_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        spirv_info.props = SDL_CreateProperties();
        SDL_SetBooleanProperty(spirv_info.props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_ENABLE_BOOLEAN, true);
        SDL_SetStringProperty(spirv_info.props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_NAME_STRING, "Simple shader");
        shader_size = 0;
        shader = cases[i].compile_XXXFromSPIRV(&spirv_info, &shader_size);
        SDLTest_AssertCheck(shader != NULL, "%s should return a valid compiled DXBC shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size != 0, "Size of shader returned by %s should be size > 0", cases[i].funcname);
        SDL_free(shader);
        SDL_DestroyProperties(spirv_info.props);
        SDL_ClearError();
    }
    SDL_free(spirv_shader);
    return TEST_COMPLETED;
}

static int SDLCALL shadercross_TranspileSPIRV_to_MSL(void *args)
{
    void *spirv_shader;
    size_t spirv_shader_size;
    void *msl_shader;
    SDL_ShaderCross_SPIRV_Info spirv_info;

    (void)args;
    if (!(SDL_ShaderCross_GetSPIRVShaderFormats() & SDL_GPU_SHADERFORMAT_MSL)) {
        SDLTest_AssertPass("SDL_ShaderCross does not support SPIRV -> MSL");
        return TEST_SKIPPED;
    }

    {
        SDL_ShaderCross_HLSL_Info hlsl_info;

        SDLTest_AssertPass("Prepare SPIRV vertex shader (HLSL -> SPIRV)");
        if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_SPIRV)) {
            SDLTest_AssertPass("SDL_ShaderCross does not support HLSL -> SPIRV");
            return TEST_SKIPPED;
        }

        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.entrypoint = "main";
        spirv_shader = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_shader_size);
        SDLTest_AssertCheck(spirv_shader != NULL, "SDL_ShaderCross_CompileSPIRVFromHLSL must return a non-NULL shader (%s)", SDL_GetError());
        if (spirv_shader == NULL) {
            return TEST_ABORTED;
        }
    }

    SDLTest_AssertPass("Transpile SPIRV -> MSL");
    SDL_zero(spirv_info);
    spirv_info.bytecode = spirv_shader;
    spirv_info.bytecode_size = spirv_shader_size;
    spirv_info.entrypoint = "main";
    msl_shader = SDL_ShaderCross_TranspileMSLFromSPIRV(&spirv_info);
    SDLTest_AssertCheck(msl_shader != NULL, "SDL_ShaderCross_TranspileMSLFromSPIRV returns non-NULL shader (%s)", SDL_GetError());
    SDL_free(msl_shader);

    SDL_free(spirv_shader);
    return TEST_COMPLETED;
}

static const char *iovartype_to_string(SDL_ShaderCross_IOVarType type)
{
    switch (type) {
    case SDL_SHADERCROSS_IOVAR_TYPE_UNKNOWN:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UNKNOWN";
    case SDL_SHADERCROSS_IOVAR_TYPE_INT8:
        return "SDL_SHADERCROSS_IOVAR_TYPE_INT8";
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT8:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UINT8";
    case SDL_SHADERCROSS_IOVAR_TYPE_INT16:
        return "SDL_SHADERCROSS_IOVAR_TYPE_INT16";
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT16:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UINT16";
    case SDL_SHADERCROSS_IOVAR_TYPE_INT32:
        return "SDL_SHADERCROSS_IOVAR_TYPE_INT32";
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT32:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UINT32";
    case SDL_SHADERCROSS_IOVAR_TYPE_INT64:
        return "SDL_SHADERCROSS_IOVAR_TYPE_INT64";
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT64:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UINT64";
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT16:
        return "SDL_SHADERCROSS_IOVAR_TYPE_FLOAT16";
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT32:
        return "SDL_SHADERCROSS_IOVAR_TYPE_FLOAT32";
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT64:
        return "SDL_SHADERCROSS_IOVAR_TYPE_FLOAT64";
    }
    SDLTest_AssertCheck(false, "Unknown SDL_ShaderCross_IOVarType");
    return NULL;
}

static void log_SDL_ShaderCross_IOVarMetadata(Uint32 index, const SDL_ShaderCross_IOVarMetadata *iovar_metadata)
{
    SDLTest_AssertPass("%" SDL_PRIu32 ": { name: \"%s\", location: %" SDL_PRIu32 ", vector_type: %s, vector_size: %" SDL_PRIu32 " }",
        index, iovar_metadata->name, iovar_metadata->location, iovartype_to_string(iovar_metadata->vector_type), iovar_metadata->vector_size);
}

static int SDLCALL shadercross_ReflectSPIRV(void *args)
{
    Uint32 i;
    void *spirv_shader;
    size_t spirv_shader_size;
    SDL_ShaderCross_GraphicsShaderMetadata *shader_gfx_metadata;

    (void)args;
    if (!(SDL_ShaderCross_GetSPIRVShaderFormats() & SDL_GPU_SHADERFORMAT_MSL)) {
        SDLTest_AssertPass("SDL_ShaderCross does not support SPIRV -> MSL");
        return TEST_SKIPPED;
    }

    {
        SDL_ShaderCross_HLSL_Info hlsl_info;

        SDLTest_AssertPass("Prepare SPIRV vertex shader (HLSL -> SPIRV)");
        if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_SPIRV)) {
            SDLTest_AssertPass("SDL_ShaderCross does not support HLSL -> SPIRV");
            return TEST_SKIPPED;
        }

        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.entrypoint = "main";
        spirv_shader = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_shader_size);
        SDLTest_AssertCheck(spirv_shader != NULL, "SDL_ShaderCross_CompileSPIRVFromHLSL must return a non-NULL shader (%s)", SDL_GetError());
        if (spirv_shader == NULL) {
            return TEST_ABORTED;
        }
    }

    SDLTest_AssertPass("Reflect SPIRV");

    shader_gfx_metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(spirv_shader, spirv_shader_size, 0);
    SDLTest_AssertCheck(shader_gfx_metadata != NULL, "SDL_ShaderCross_ReflectGraphicsSPIRV returns non-NULL shader (%s)", SDL_GetError());
    SDLTest_AssertCheck(shader_gfx_metadata->resource_info.num_samplers == 0, "num_samplers is %d, should be 0", shader_gfx_metadata->resource_info.num_samplers);
    SDLTest_AssertCheck(shader_gfx_metadata->resource_info.num_storage_textures == 0, "num_storage_textures is %d, should be 0", shader_gfx_metadata->resource_info.num_storage_textures);
    SDLTest_AssertCheck(shader_gfx_metadata->resource_info.num_storage_buffers == 0, "num_storage_buffers is %d, should be 0", shader_gfx_metadata->resource_info.num_storage_buffers);
    SDLTest_AssertCheck(shader_gfx_metadata->resource_info.num_uniform_buffers == 1, "num_uniform_buffers is %d, should be 1", shader_gfx_metadata->resource_info.num_uniform_buffers);
    SDLTest_AssertCheck(shader_gfx_metadata->num_inputs == 1, "num_inputs is %d, should be 1", shader_gfx_metadata->num_inputs);
    SDLTest_AssertCheck(shader_gfx_metadata->num_outputs == 1, "num_outputs is %d, should be 1", shader_gfx_metadata->num_outputs);

    SDLTest_AssertPass("inputs:");
    for (i = 0; i < shader_gfx_metadata->num_inputs; i++) {
        log_SDL_ShaderCross_IOVarMetadata(i, &shader_gfx_metadata->inputs[i]);
    }
    SDLTest_AssertPass("outputs:");
    for (i = 0; i < shader_gfx_metadata->num_outputs; i++) {
        log_SDL_ShaderCross_IOVarMetadata(i, &shader_gfx_metadata->outputs[i]);
    }

    SDL_free(shader_gfx_metadata);

    SDL_free(spirv_shader);
    return TEST_COMPLETED;
}

static Uint32 shadercross_read_u32(const Uint8 *data)
{
    Uint32 value;
    SDL_memcpy(&value, data, sizeof(value));
    return SDL_Swap32LE(value);
}

static bool shadercross_check_dxil_inputs(const void *bytecode, size_t size)
{
    const Uint8 *data = bytecode;
    Uint32 part_count, i;

    if (size < 32 || SDL_memcmp(data, "DXBC", 4) != 0) {
        return false;
    }
    part_count = shadercross_read_u32(data + 28);
    if (part_count > (size - 32) / 4) {
        return false;
    }
    for (i = 0; i < part_count; ++i) {
        Uint32 offset = shadercross_read_u32(data + 32 + 4 * i);
        const Uint8 *signature;
        Uint32 signature_size, params, j;
        if (offset > size - 8) {
            return false;
        }
        if (SDL_memcmp(data + offset, "ISG1", 4) != 0) {
            continue;
        }
        signature_size = shadercross_read_u32(data + offset + 4);
        if (signature_size < 8 || signature_size > size - offset - 8) {
            return false;
        }
        signature = data + offset + 8;
        params = shadercross_read_u32(signature + 4);
        if (shadercross_read_u32(signature) != 2 || params > signature_size ||
            signature_size - params < 2 * 32) {
            return false;
        }
        for (j = 0; j < 2; ++j) {
            const Uint8 *param = signature + params + 32 * j;
            Uint32 name = shadercross_read_u32(param + 4);
            if (name > signature_size || signature_size - name < 9 ||
                SDL_memcmp(signature + name, "TEXCOORD", 9) != 0 ||
                shadercross_read_u32(param + 8) != j ||
                shadercross_read_u32(param + 20) != j) {
                return false;
            }
        }
        return true;
    }
    return false;
}

static int SDLCALL shadercross_preserve_interface(void *args)
{
    static const char source[] =
        "struct Input { float4 unused : TEXCOORD0; float4 color : TEXCOORD1;\n"
        "#if !VERTEX\nfloat4 position : SV_Position;\n#endif\n};\n"
        "#if VERTEX\n#define SPACE space0\n#define TARGET SV_Position\n"
        "#else\n#define SPACE space2\n#define TARGET SV_Target0\n#endif\n"
        "#if RESOURCE_KIND == 1\n"
        "Texture2D<float4> textures[2] : register(t0, SPACE);\n"
        "SamplerState samplers[2] : register(s0, SPACE);\n"
        "#elif RESOURCE_KIND == 2\n"
        "StructuredBuffer<float4> buffer : register(t0, SPACE);\n#endif\n"
        "float4 main(Input input) : TARGET {\n"
        "#if RESOURCE_KIND == 1\n"
        "return input.color * (textures[0].SampleLevel(samplers[0], float2(0,0), 0) +\n"
        "textures[1].SampleLevel(samplers[1], float2(0,0), 0));\n"
        "#elif RESOURCE_KIND == 2\nreturn input.color * buffer[0];\n"
        "#else\nreturn input.color;\n#endif\n}\n";
    char *resource_kinds[] = { "0", "1", "2" };
    int vertex, kind;
    (void)args;

    if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_SPIRV) ||
        !(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_DXIL)) {
        return TEST_SKIPPED;
    }
    for (vertex = 0; vertex < 2; ++vertex) {
        for (kind = 0; kind < 3; ++kind) {
            SDL_ShaderCross_HLSL_Define defines[] = {
                { "VERTEX", vertex ? "1" : "0" },
                { "RESOURCE_KIND", resource_kinds[kind] },
                { NULL, NULL }
            };
            SDL_ShaderCross_HLSL_Info hlsl_info;
            SDL_ShaderCross_SPIRV_Info spirv_info;
            SDL_ShaderCross_GraphicsShaderMetadata *metadata;
            void *spirv, *dxil;
            size_t spirv_size = 0, dxil_size = 0;
            Uint32 i, locations = 0;

            SDLTest_Log("Preserve interface: vertex=%d, resource kind=%d", vertex, kind);
            SDL_zero(hlsl_info);
            hlsl_info.source = source;
            hlsl_info.entrypoint = "main";
            hlsl_info.shader_stage = vertex ? SDL_SHADERCROSS_SHADERSTAGE_VERTEX : SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
            hlsl_info.defines = defines;
            spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_size);
            SDLTest_AssertCheck(spirv != NULL, "Compile interface test to SPIRV (%s)", SDL_GetError());
            if (!spirv) {
                continue;
            }
            metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(spirv, spirv_size, 0);
            SDLTest_AssertCheck(metadata != NULL, "Reflect interface test (%s)", SDL_GetError());
            if (metadata) {
                for (i = 0; i < metadata->num_inputs; ++i) {
                    if (metadata->inputs[i].location < 2) {
                        locations |= 1u << metadata->inputs[i].location;
                    }
                }
                SDLTest_AssertCheck(metadata->num_inputs == 1 && locations == 2,
                                    "Reflection reports the active input at its declared location 1");
                SDLTest_AssertCheck(metadata->resource_info.num_samplers == (kind == 1 ? 2u : 0u),
                                    "Resource arrays still flatten into two bindings");
                SDLTest_AssertCheck(metadata->resource_info.num_storage_buffers == (kind == 2 ? 1u : 0u),
                                    "Structured buffers remain supported");
                SDL_free(metadata);
            }

            dxil = SDL_ShaderCross_CompileDXILFromHLSL(&hlsl_info, &dxil_size);
            SDLTest_AssertCheck(dxil != NULL, "Compile HLSL to DXIL (%s)", SDL_GetError());
            if (dxil) {
                SDLTest_AssertCheck(shadercross_check_dxil_inputs(dxil, dxil_size),
                                    "HLSL -> DXIL retains TEXCOORD0/1 in registers 0/1");
                SDL_free(dxil);
            }
            SDL_zero(spirv_info);
            spirv_info.bytecode = spirv;
            spirv_info.bytecode_size = spirv_size;
            spirv_info.entrypoint = "main";
            spirv_info.shader_stage = hlsl_info.shader_stage;
            dxil = SDL_ShaderCross_CompileDXILFromSPIRV(&spirv_info, &dxil_size);
            SDLTest_AssertCheck(dxil != NULL, "Compile SPIRV to DXIL (%s)", SDL_GetError());
            if (dxil) {
                SDLTest_AssertCheck(shadercross_check_dxil_inputs(dxil, dxil_size),
                                    "SPIRV -> DXIL retains TEXCOORD0/1 in registers 0/1");
                SDL_free(dxil);
            }
            SDL_free(spirv);
        }
    }
    return TEST_COMPLETED;
}

static const SDLTest_TestCaseReference shadercrossPreserveInterface = {
    shadercross_preserve_interface, "shadercross_PreserveInterface", "Preserve unused stage inputs through legalization and DXIL compilation", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossInitQuit = {
    shadercross_testInitQuit, "shadercrossInitQuit", "Test SDL_ShaderCross_Init and SDL_ShaderCross_Quit", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossCompileHLSL = {
    shadercross_CompileHLSL_to_XXX, "shadercross_CompileHLSL", "Compile HLSL -> {DXBC, DXIL, SPIRV}", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossCompileSPIRV = {
    shadercross_CompileSPIRV_to_XXX, "shadercross_CompileSPIRV", "Compile SPIRV -> {DXBC, DXIL}", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossReflectSPIRV = {
    shadercross_TranspileSPIRV_to_MSL, "shadercross_TranspileSPIRVtoMSL", "Compile SPIRV -> {DXBC, DXIL}", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossTranspileSPIRVToMSL = {
    shadercross_ReflectSPIRV, "shadercross_ReflectSPIRV", "Reflect SPIRV", TEST_ENABLED
};

static const SDLTest_TestCaseReference *shadercrossTests[] = {
    &shadercrossInitQuit,
    &shadercrossCompileHLSL,
    &shadercrossCompileSPIRV,
    &shadercrossTranspileSPIRVToMSL,
    &shadercrossReflectSPIRV,
    &shadercrossPreserveInterface,
    NULL
};

static void SDLCALL setup_shadercross(void **args)
{
    bool result;
    (void) args;
    SDL_ClearError();
    result = SDL_ShaderCross_Init();
    SDLTest_AssertCheck(result, "SDL_ShaderCross_Init() succeeded (%s)", SDL_GetError());
    SDL_ClearError();
}

static void SDLCALL teardown_shadercross(void *args)
{
    (void) args;
    SDL_ShaderCross_Quit();
}

static SDLTest_TestSuiteReference shadercrossTestSuite = {
    "SDL_shadercross",
    setup_shadercross,
    shadercrossTests,
    teardown_shadercross
};

static SDLTest_TestSuiteReference *testSuites[] = {
    &shadercrossTestSuite,
    NULL
};

int main(int argc, char *argv[])
{
    int i;
    int result;
    SDLTest_CommonState *state;
    SDLTest_TestSuiteRunner *runner;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, 0);
    if (!state) {
        return 1;
    }

    runner = SDLTest_CreateTestSuiteRunner(state, testSuites);

    /* Parse commandline */
    for (i = 1; i < argc;) {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if (!consumed) {
            /* Parse custom arguments here */
        }
        if (consumed <= 0) {
            const char *options[] = {
                NULL
            };
            SDLTest_CommonLogUsage(state, argv[0], options);
            return 1;
        }

        i += consumed;
    }

    /* Verify custom options here */

    result = SDLTest_ExecuteTestSuiteRunner(runner);

    SDL_Quit();
    SDLTest_DestroyTestSuiteRunner(runner);
    SDLTest_CommonDestroyState(state);
    return result;
}
