Shader "Zippy/PointCloud"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
    }
    SubShader
    {
        // No culling or depth
        Cull Off ZWrite Off ZTest Always

        Pass
        {
            CGPROGRAM
            #pragma target 3.0
            #pragma vertex vert
            #pragma fragment frag
            
            #include "UnityCG.cginc"

            uniform float4x4 projInv;

            struct v2f
            {
                float4 pos : SV_POSITION;       //Clip space
                float4 uv : TEXCOORD0;          //UV data
            };

            v2f vert (appdata_base v)
            {
                v2f o;
                o.pos = UnityObjectToClipPos(v.vertex);
                o.uv = v.texcoord;
                return o;
            }
            
            sampler2D _MainTex;
            uniform sampler2D _CameraDepthTexture; //the depth texture

            float4 frag (v2f i) : SV_Target
            {
                float3 position;
                float rawDepth = SAMPLE_DEPTH_TEXTURE_PROJ(_CameraDepthTexture, i.uv);
                float depthVal = LinearEyeDepth(rawDepth);

                float4 pos;
                pos.x = (i.uv.x * 2.0) - 1.0;
                pos.y = (i.uv.y * 2.0) - 1.0;
                pos.z = -1;
                pos.w = 1;

                float4 ray = mul(projInv, pos);
                ray.z = -1.0;

                position = ray.xyz * depthVal;

                //make right handed, z forward, x right, y down
                return float4(position.x, -position.y, -position.z, 1);
            }
            ENDCG
        }
    }
}
