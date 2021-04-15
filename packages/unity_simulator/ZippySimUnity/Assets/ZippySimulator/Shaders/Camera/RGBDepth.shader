Shader "Zippy/Camera/RGBDepth"
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

            struct VertexOutput
            {
                float4 pos : SV_POSITION;        //Clip space
                float4 uv : TEXCOORD0;            //UV data
            };

            VertexOutput vert(appdata_base v)
            {
                VertexOutput o;
                o.pos = UnityObjectToClipPos(v.vertex);
                o.uv = v.texcoord;
                return o;
            }

            float4 frag(VertexOutput i) : SV_Target
            {
                float4 color = tex2Dproj(_MainTex, i.uv);
                float rawDepth = SAMPLE_DEPTH_TEXTURE_PROJ(_CameraDepthTexture, i.uv);
                float depth = LinearEyeDepth(rawDepth);// Linear01Depth(rawDepth);

                #ifdef IS_GREYSCALE
                color.r = Luminance(color.rgb);
                color.g = depth;
                #else
                color.a = depth;
                #endif

                return color;
            }
            ENDCG
        }
    }
}
