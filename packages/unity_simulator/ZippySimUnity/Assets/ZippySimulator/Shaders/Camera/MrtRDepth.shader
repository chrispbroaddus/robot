Shader "Zippy/Camera/MrtRDepth"
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

            struct v2f
            {
                float4 pos : SV_POSITION;        //Clip space
                float4 uv : TEXCOORD0;    //UV data
            };

            // MRT shader
            struct FragmentOutput
            {
                //FIXME This should be float as images should be a RED only
                float4 image : SV_Target0;
                float depth : SV_Target1;
            };

            v2f vert(appdata_base v)
            {
                v2f o;
                o.pos = UnityObjectToClipPos (v.vertex);
                o.uv = v.texcoord;
                return o;
            }

            FragmentOutput frag(v2f i)
            {                
                float2 color = tex2Dproj(_MainTex, i.uv).rg;

                FragmentOutput o;
                //FIXME we should only need to copy the red channel
                //o.image = color.r;
                o.image = color.rrrr;
                o.depth = color.g;
                return o;
            }
            ENDCG
        }
    }
}