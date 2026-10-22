// PBRDepth-style sparse material bindings, with optional storage resources.
Texture2D<float4> texture4 : register(t4, space2);
SamplerState sampler4 : register(s4, space2);
#if SAMPLE_ARRAY
Texture2D<float4> texture6[2] : register(t6, space2);
SamplerState sampler6[2] : register(s6, space2);
#else
Texture2D<float4> texture6 : register(t6, space2);
SamplerState sampler6 : register(s6, space2);
#endif

cbuffer Frame : register(b0, space3) { float4 tint; };
cbuffer Material : register(b3, space3) { float4 exposure; };

#if STORAGE_KIND
#if STORAGE_KIND == 2
#define TEXTURE_REGISTER t9
#define BUFFER_REGISTER t12
#elif STORAGE_KIND == 3
// Invalid SDL ordering: a storage texture inside the sampler slot span.
#define TEXTURE_REGISTER t5
#define BUFFER_REGISTER t12
#elif STORAGE_KIND == 4
#define TEXTURE_REGISTER t7
// Invalid SDL ordering: a storage buffer below the storage texture boundary.
#define BUFFER_REGISTER t5
#elif SAMPLE_ARRAY
#define TEXTURE_REGISTER t8
#define BUFFER_REGISTER t9
#else
#define TEXTURE_REGISTER t7
#define BUFFER_REGISTER t8
#endif
// Classification must not depend on the order of reflected image declarations.
Texture2D<float4> storageTexture : register(TEXTURE_REGISTER, space2);
StructuredBuffer<float4> storageBuffer : register(BUFFER_REGISTER, space2);
#endif

float4 main(float4 position : SV_Position) : SV_Target0
{
    float4 color = texture4.SampleLevel(sampler4, position.xy, 0);
#if SAMPLE_ARRAY
    color += texture6[0].SampleLevel(sampler6[0], position.xy, 0);
#if !LAST_ARRAY_UNUSED
    color += texture6[1].SampleLevel(sampler6[1], position.xy, 0);
#endif
#else
    color += texture6.SampleLevel(sampler6, position.xy, 0);
#endif
#if STORAGE_KIND
    color += storageTexture.Load(int3(0, 0, 0)) + storageBuffer[0];
#endif
    return color * tint * exposure;
}
