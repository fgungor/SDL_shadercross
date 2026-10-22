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

typedef struct SparseSamplerCase {
    const char *name;
    char *storage_kind;
    char *sample_array;
    Uint32 samplers;
    Uint32 storage_textures;
    Uint32 storage_buffers;
    bool raw_spirv;
} SparseSamplerCase;

static const SparseSamplerCase sparse_sampler_cases[] = {
    { "t4/s4 and t6/s6", "0", "0", 7, 0, 0, false },
    { "storage immediately after sampler span", "1", "0", 7, 1, 1, false },
    { "gaps in storage ranges", "2", "0", 7, 3, 3, false },
    { "sampled resource arrays", "0", "1", 8, 0, 0, false },
    { "arrays followed by storage resources", "1", "1", 8, 1, 1, false },
#ifdef SDL_SHADERCROSS_TEST_RAW_ARRAY_SPIRV
    { "unflattened arrays followed by storage", "1", "1", 8, 1, 1, true },
#endif
};

static void *shadercross_sparse_sampler_spirv(const SparseSamplerCase *test_case, size_t *size)
{
#ifdef SDL_SHADERCROSS_TEST_RAW_ARRAY_SPIRV
    if (test_case->raw_spirv) {
        return SDL_LoadFile(SDL_SHADERCROSS_TEST_RAW_ARRAY_SPIRV, size);
    }
#endif
    char *source = SDL_LoadFile(SDL_SHADERCROSS_TEST_SHADER_DIR "/sparse-samplers.frag.hlsl", NULL);
    SDL_ShaderCross_HLSL_Define defines[] = {
        { "STORAGE_KIND", test_case->storage_kind },
        { "SAMPLE_ARRAY", test_case->sample_array },
        { NULL, NULL }
    };
    SDL_ShaderCross_HLSL_Info info;
    void *spirv;
    if (!source) {
        return NULL;
    }
    SDL_zero(info);
    info.source = source;
    info.entrypoint = "main";
    info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    info.defines = defines;
    spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(&info, size);
    SDL_free(source);
    return spirv;
}

static int SDLCALL shadercross_sparse_sampler_reflection(void *args)
{
    (void)args;
    if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_SPIRV)) {
        return TEST_SKIPPED;
    }
    for (size_t i = 0; i < SDL_arraysize(sparse_sampler_cases); ++i) {
        const SparseSamplerCase *test_case = &sparse_sampler_cases[i];
        size_t size = 0;
        void *spirv = shadercross_sparse_sampler_spirv(test_case, &size);
        SDLTest_AssertCheck(spirv != NULL, "%s: compile SPIRV (%s)", test_case->name, SDL_GetError());
        if (!spirv) {
            continue;
        }
        SDL_ShaderCross_GraphicsShaderMetadata *metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(spirv, size, 0);
        SDLTest_AssertCheck(metadata != NULL, "%s: reflect (%s)", test_case->name, SDL_GetError());
        if (metadata) {
            SDLTest_AssertCheck(metadata->resource_info.num_samplers == test_case->samplers,
                                "%s: sampler span is %u (got %u)", test_case->name, test_case->samplers, metadata->resource_info.num_samplers);
            SDLTest_AssertCheck(metadata->resource_info.num_storage_textures == test_case->storage_textures,
                                "%s: storage texture span is %u", test_case->name, test_case->storage_textures);
            SDLTest_AssertCheck(metadata->resource_info.num_storage_buffers == test_case->storage_buffers,
                                "%s: storage buffer span is %u", test_case->name, test_case->storage_buffers);
            SDLTest_AssertCheck(metadata->resource_info.num_uniform_buffers == 4, "b0/b3 reserve four uniform slots");
            SDL_free(metadata);
        }
        if (i == 0) {
            Uint32 *swapped = SDL_malloc(size);
            if (swapped) {
                SDL_memcpy(swapped, spirv, size);
                for (size_t word = 0; word < size / sizeof(Uint32); ++word) {
                    swapped[word] = SDL_Swap32(swapped[word]);
                }
                metadata = SDL_ShaderCross_ReflectGraphicsSPIRV((const Uint8 *)swapped, size, 0);
                SDLTest_AssertCheck(metadata && metadata->resource_info.num_samplers == 7,
                                    "Byte-swapped SPIRV retains the sampler slot span (%s)", SDL_GetError());
                SDL_free(metadata);
                SDL_free(swapped);
            }
        }
        SDL_ShaderCross_SPIRV_Info info;
        SDL_zero(info);
        info.bytecode = spirv;
        info.bytecode_size = size;
        info.entrypoint = "main";
        info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        char *hlsl = SDL_ShaderCross_TranspileHLSLFromSPIRV(&info);
        SDLTest_AssertCheck(hlsl != NULL, "%s: transpile HLSL (%s)", test_case->name, SDL_GetError());
        if (hlsl) {
            SDLTest_AssertCheck(SDL_strstr(hlsl, "register(t4, space2)") && SDL_strstr(hlsl, "register(s4, space2)") &&
                                SDL_strstr(hlsl, "register(t6, space2)") && SDL_strstr(hlsl, "register(s6, space2)"),
                                "Generated HLSL preserves t4/s4 and t6/s6");
            if (test_case->storage_textures) {
                char texture_register[64], buffer_register[64];
                SDL_snprintf(texture_register, sizeof(texture_register), "register(t%u, space2)",
                             test_case->samplers + test_case->storage_textures - 1);
                SDL_snprintf(buffer_register, sizeof(buffer_register), "register(t%u, space2)",
                             test_case->samplers + test_case->storage_textures + test_case->storage_buffers - 1);
                SDLTest_AssertCheck(SDL_strstr(hlsl, texture_register) && SDL_strstr(hlsl, buffer_register),
                                    "Generated storage registers agree with reflected sampler/storage boundaries");
            }
            SDL_free(hlsl);
        }
        info.props = SDL_CreateProperties();
        SDL_SetStringProperty(info.props, SDL_SHADERCROSS_PROP_SPIRV_MSL_VERSION_STRING, "2.0.0");
        char *msl = SDL_ShaderCross_TranspileMSLFromSPIRV(&info);
        SDLTest_AssertCheck(msl != NULL, "%s: transpile Metal (%s)", test_case->name, SDL_GetError());
        if (msl) {
            if (test_case->storage_buffers) {
                char buffer_index[32];
                SDL_snprintf(buffer_index, sizeof(buffer_index), "[[buffer(%u)]]", 4 + test_case->storage_buffers - 1);
                SDLTest_AssertCheck(SDL_strstr(msl, buffer_index) != NULL,
                                    "Metal storage buffer index uses uniform and texture slot spans");
            }
            SDL_free(msl);
        }
        SDL_DestroyProperties(info.props);
        SDL_free(spirv);
    }
    {
        static const char *sources[] = {
            "Texture2D<float4> image4 : register(t4, space2);"
            "SamplerState sampler4 : register(s4, space2); SamplerState sampler6 : register(s6, space2);"
            "float4 main() : SV_Target { return image4.SampleLevel(sampler4, float2(0,0), 0) + image4.SampleLevel(sampler6, float2(0,0), 0); }",
            "Texture2D<float4> image4 : register(t4, space2); Texture2D<float4> image6 : register(t6, space2);"
            "SamplerState sampler4 : register(s4, space2);"
            "float4 main() : SV_Target { return image4.SampleLevel(sampler4, float2(0,0), 0) + image6.SampleLevel(sampler4, float2(0,0), 0); }"
        };
        for (size_t i = 0; i < SDL_arraysize(sources); ++i) {
            SDL_ShaderCross_HLSL_Info info;
            size_t size = 0;
            SDL_zero(info);
            info.source = sources[i];
            info.entrypoint = "main";
            info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
            void *spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(&info, &size);
            SDLTest_AssertCheck(spirv != NULL, "Compile shared texture/sampler fixture (%s)", SDL_GetError());
            if (spirv) {
                SDL_ShaderCross_GraphicsShaderMetadata *metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(spirv, size, 0);
                SDLTest_AssertCheck(metadata != NULL, "Reflect shared texture/sampler fixture (%s)", SDL_GetError());
                if (metadata) {
                    SDLTest_AssertCheck(metadata->resource_info.num_samplers == 7 && metadata->resource_info.num_storage_textures == 0,
                                        "Sampler span covers both image and sampler bindings without inventing storage textures");
                    SDL_free(metadata);
                }
                SDL_free(spirv);
            }
        }
    }
    for (int kind = 3; kind <= 4; ++kind) {
        const SparseSamplerCase invalid = { "overlapping storage range", kind == 3 ? "3" : "4", "0", 0, 0, 0, false };
        size_t size = 0;
        void *spirv = shadercross_sparse_sampler_spirv(&invalid, &size);
        SDLTest_AssertCheck(spirv != NULL, "Compile invalid-layout fixture (%s)", SDL_GetError());
        if (spirv) {
            SDL_ShaderCross_GraphicsShaderMetadata *metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(spirv, size, 0);
            SDLTest_AssertCheck(metadata == NULL && SDL_strstr(SDL_GetError(), "overlaps") != NULL,
                                "Reject storage resource t5 below its class boundary (%s)", SDL_GetError());
            SDL_free(metadata);
            SDL_free(spirv);
        }
    }
    SDL_ClearError();
    return TEST_COMPLETED;
}

static int shadercross_sparse_sampler_d3d12_internal(void *args)
{
    static const char vertex_source[] =
        "float4 main(uint id : SV_VertexID) : SV_Position { return float4(float(id), 0, 0, 1); }";
    SDL_GPUDevice *device;
    SDL_ShaderCross_HLSL_Info hlsl_info;
    SDL_ShaderCross_SPIRV_Info vertex_info;
    SDL_ShaderCross_GraphicsShaderResourceInfo vertex_resources;
    SDL_GPUShader *vertex_shader;
    void *vertex_spirv;
    size_t vertex_size;
    (void)args;
    if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_DXIL)) {
        return TEST_SKIPPED;
    }
    device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL, true, "direct3d12");
    if (!device) {
        SDLTest_Log("D3D12 unavailable: %s", SDL_GetError());
        if (SDL_GetEnvironmentVariable(SDL_GetEnvironment(), "SDLSHADERCROSS_REQUIRE_D3D12")) {
            SDLTest_AssertCheck(false, "Required D3D12 device could not be created");
            return TEST_ABORTED;
        }
        return TEST_SKIPPED;
    }
    SDL_ClearError();
    SDL_zero(hlsl_info);
    hlsl_info.source = vertex_source;
    hlsl_info.entrypoint = "main";
    hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
    vertex_spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &vertex_size);
    SDLTest_AssertCheck(vertex_spirv != NULL, "Compile pipeline vertex shader (%s)", SDL_GetError());
    if (!vertex_spirv) {
        SDL_DestroyGPUDevice(device);
        return TEST_ABORTED;
    }
    SDL_zero(vertex_info);
    vertex_info.bytecode = vertex_spirv;
    vertex_info.bytecode_size = vertex_size;
    vertex_info.entrypoint = "main";
    vertex_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
    SDL_zero(vertex_resources);
    vertex_shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &vertex_info, &vertex_resources, 0);
    SDL_free(vertex_spirv);
    SDLTest_AssertCheck(vertex_shader != NULL, "Create D3D12 vertex shader (%s)", SDL_GetError());
    if (!vertex_shader) {
        SDL_DestroyGPUDevice(device);
        return TEST_ABORTED;
    }
    for (size_t i = 0; i < SDL_arraysize(sparse_sampler_cases); ++i) {
        size_t size = 0;
        void *spirv = shadercross_sparse_sampler_spirv(&sparse_sampler_cases[i], &size);
        SDLTest_AssertCheck(spirv != NULL, "Compile D3D12 fragment fixture (%s)", SDL_GetError());
        if (!spirv) {
            continue;
        }
        SDL_ShaderCross_GraphicsShaderMetadata *metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(spirv, size, 0);
        SDLTest_AssertCheck(metadata != NULL, "Reflect D3D12 fragment fixture (%s)", SDL_GetError());
        if (metadata) {
            SDL_ShaderCross_SPIRV_Info info;
            SDL_zero(info);
            info.bytecode = spirv;
            info.bytecode_size = size;
            info.entrypoint = "main";
            info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
            SDL_GPUShader *fragment = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &info, &metadata->resource_info, 0);
            SDLTest_AssertCheck(fragment != NULL, "Create D3D12 sparse fragment shader (%s)", SDL_GetError());
            if (fragment) {
                SDL_GPUColorTargetDescription target;
                SDL_GPUGraphicsPipelineCreateInfo pipeline_info;
                SDL_zero(target);
                target.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
                SDL_zero(pipeline_info);
                pipeline_info.vertex_shader = vertex_shader;
                pipeline_info.fragment_shader = fragment;
                pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
                pipeline_info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
                pipeline_info.target_info.num_color_targets = 1;
                pipeline_info.target_info.color_target_descriptions = &target;
                SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
                SDLTest_AssertCheck(pipeline != NULL, "%s: D3D12 pipeline creates successfully (%s)", sparse_sampler_cases[i].name, SDL_GetError());
                if (pipeline) {
                    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
                }
                SDL_ReleaseGPUShader(device, fragment);
            }
            SDL_free(metadata);
        }
        SDL_free(spirv);
    }
    SDL_ReleaseGPUShader(device, vertex_shader);
    SDL_DestroyGPUDevice(device);
    return TEST_COMPLETED;
}

static int SDLCALL shadercross_sparse_sampler_d3d12(void *args)
{
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        SDLTest_Log("Video initialization failed: %s", SDL_GetError());
        if (SDL_GetEnvironmentVariable(SDL_GetEnvironment(), "SDLSHADERCROSS_REQUIRE_D3D12")) {
            SDLTest_AssertCheck(false, "Video initialization required for D3D12 test");
            return TEST_ABORTED;
        }
        return TEST_SKIPPED;
    }
    int result = shadercross_sparse_sampler_d3d12_internal(args);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    return result;
}

static const SDLTest_TestCaseReference shadercrossSparseSamplerReflection = {
    shadercross_sparse_sampler_reflection, "shadercross_SparseSamplerReflection", "Reflect sparse sampler and storage slot spans", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossSparseSamplerD3D12 = {
    shadercross_sparse_sampler_d3d12, "shadercross_SparseSamplerD3D12", "Create D3D12 pipelines with sparse sampler and storage registers", TEST_ENABLED
};

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
    &shadercrossSparseSamplerReflection,
    &shadercrossSparseSamplerD3D12,
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
