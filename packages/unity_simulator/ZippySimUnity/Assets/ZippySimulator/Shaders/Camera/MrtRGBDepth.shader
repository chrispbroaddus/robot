Shader "Zippy/Camera/MrtRGBDepth"
{
    Properties
    {
        _MainTex("Color (RGB)", 2D) = "white"{}
    }

    SubShader
    {
        Tags { "Queue" = "Transparent" "RenderType" = "Transparent" }

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

            struct VertexOutput
            {
                float4 pos : SV_POSITION;        //Clip space
                float4 uv : TEXCOORD0;    //UV data
            };

            // MRT shader
            struct FragmentOutput
            {
                float4 image : SV_Target0;
                float depth : SV_Target1;
            };

            VertexOutput vert(appdata_base v)
            {
                VertexOutput o;
                o.pos = UnityObjectToClipPos (v.vertex);
                o.uv = v.texcoord;
                return o;
            }

            FragmentOutput frag(VertexOutput i)
            {
                float4 color = tex2Dproj(_MainTex, i.uv);
                float depth = color.a;
                color.a = 1.0;

                FragmentOutput o;
                o.image = color;
                o.depth = depth;
                return o;
            }
            ENDCG
        }
    }
}