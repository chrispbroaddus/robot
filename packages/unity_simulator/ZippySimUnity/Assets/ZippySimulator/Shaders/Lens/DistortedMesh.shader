Shader "Zippy/Lens/DistortedMesh"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
        _ValidMask("Alpha", 2D) = "white"{}
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            
            #include "UnityCG.cginc"

            sampler2D _MainTex;
            sampler2D _ValidMask;
            
            struct v2f
            {
                float4 pos : SV_POSITION;        //Clip space
                float4 uv : TEXCOORD0;            //UV data
                float4 uv2 : TEXCOORD1;
            };

            v2f vert(appdata_full v)
            {
                v2f o;
                o.pos = UnityObjectToClipPos(v.vertex);
                o.uv = v.texcoord;
                o.uv2 = v.texcoord1;
                return o;
            }

            float4 frag (v2f i) : SV_Target
            {
                float mask = tex2Dproj(_ValidMask, i.uv2).a;

                if (mask < .2) {
                    return float4(0, 0, 0, 0);
                }
                else if (mask > .8)
                {
//                    return float4(1, 0, 0, 1);
                    return tex2D(_MainTex, i.uv);
                }
                else
                {
                    return float4(.5, .5, .5, 0);
                }

                // sample the texture
//                float4 col = tex2D(_MainTex, i.uv);
//                col *= mask;
//                return col;
            }

            ENDCG
        }
    }
}
