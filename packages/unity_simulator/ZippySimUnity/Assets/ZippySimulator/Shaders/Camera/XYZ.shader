Shader "Zippy/Camera/XYZ"
{
    Properties
    {
        _MainTex("Color (RGB)", 2D) = "white"{}
    }

    SubShader
    {
        Tags { "Queue"="Transparent" "RenderType"="Transparent" }

        Pass
        {
            Cull Off ZWrite Off ZTest Always

            CGPROGRAM
            #pragma target 3.0
            #pragma vertex vert
            #pragma fragment frag
            #include "UnityCG.cginc"

            sampler2D _MainTex;

            struct v2f
            {
                float4 pos : SV_POSITION; //Clip space
                float4 uv : TEXCOORD0;    //UV data
            };

            v2f vert(appdata_base v)
            {
                v2f o;
                o.pos = UnityObjectToClipPos (v.vertex);
                o.uv = v.texcoord;
                return o;
            }

            float4 frag(v2f i) : SV_Target
            {                
                return tex2Dproj(_MainTex, i.uv);
            }
            ENDCG
        }
    }
}
