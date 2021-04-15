Shader "Zippy/Camera/RGBDepthXYZ"
{
    Properties
    {
        _MainTex("Color (RGB)", 2D) = "white"{}
    }

    SubShader
    {
        Tags{ "Queue" = "Transparent" "RenderType" = "Transparent" }

        Pass
        {
            Cull Off ZWrite Off ZTest Always

            CGPROGRAM
            #pragma target 3.0
            #pragma vertex vert
            #pragma fragment frag
            #include "UnityCG.cginc"

            #pragma multi_compile __ IS_GREYSCALE

            sampler2D _MainTex;
            uniform sampler2D _CameraDepthTexture; //the depth texture
            uniform float4x4 projInv;

            struct VertexOutput
            {
                float4 pos : SV_POSITION;       //Clip space
                float4 uv : TEXCOORD0;          //UV data
            };

            struct FragmentOutput
            {
                float4 image : SV_Target0;
                float4 xyz : SV_Target1;
            };

            VertexOutput vert(appdata_base v)
            {
                VertexOutput o;
                o.pos = UnityObjectToClipPos(v.vertex);
                o.uv = v.texcoord;
                return o;
            }

            FragmentOutput frag(VertexOutput i) : SV_Target
            {
                float4 color = tex2Dproj(_MainTex, i.uv);
                float rawDepth = SAMPLE_DEPTH_TEXTURE_PROJ(_CameraDepthTexture, i.uv);
                float depth = LinearEyeDepth(rawDepth);

                #ifdef IS_GREYSCALE
                color.r = Luminance(color.rgb);
                color.g = depth;
                #else
                color.a = depth;
                #endif

                float4 pos;
                pos.x = (i.uv.x * 2.0) - 1.0;
                pos.y = (i.uv.y * 2.0) - 1.0;
                pos.z = -1;
                pos.w = 1;

                float4 ray = mul(projInv, pos);
                ray.z = -1.0;

                float3 position = ray.xyz * depth;

                //make right handed, z forward, x right, y down
                float4 xyz = float4(position.x, -position.y, -position.z, 1);


                FragmentOutput o;
                o.image = color;
                o.xyz = xyz;
                return o;
            }
            ENDCG
        }
    }
}
