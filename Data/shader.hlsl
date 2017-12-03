#include "shader.inc"

/*
  This is simple shader example for Sandworm preprocessor.
*/
float4 main(float4 v : TEXCOORD0) : SV_Target
{    

#if (UBER_SHADER & 1)
    return Foo(float4 (1,1,1,ALPHA), v);
#elif (UBER_SHADER & 2)
    return Foo(float4 (2,1,1,ALPHA), v);
#elif (UBER_SHADER & 4)
    return Foo(float4 (4,1,1,ALPHA), v);
#elif (UBER_SHADER & 8)
    return Foo(float4 (8,1,1,ALPHA), v);
#elif (UBER_SHADER & 16)
    return Foo(float4 (16,1,1,ALPHA), v);
#else
    return Foo(float4 (32,1,1,ALPHA), v);
#endif
}
