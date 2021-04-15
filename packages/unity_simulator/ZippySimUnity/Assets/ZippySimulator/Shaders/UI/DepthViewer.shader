Shader "Zippy/UI/DepthViewer"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
		_MaxDistance ("Max Distance (m)", Range(0, 1)) = 1
        _RenderingDepth ("Source Camera Rendering Depth (m)", float) = 1
		[MaterialToggle] _Invert ("Invert Depth Values", Float) = 0
    }

    SubShader
    {
        Tags { "RenderType"="Opaque" }
		
        Pass
        {
            CGPROGRAM
            #pragma vertex vert_img
            #pragma fragment frag
            #include "UnityCG.cginc"

            sampler2D _MainTex;
            float _MaxDistance;
            float _RenderingDepth;
			float _Invert;

            float4 frag (v2f_img i) : SV_Target
            {
                // sample the texture to get depth in m
                float d = tex2D(_MainTex, i.uv).r;

                //normalize to 0-1
                d = d / _RenderingDepth;

                //rescale so _MaxDistance is the maximum value
				d = min(d, _MaxDistance) / _MaxDistance;

                //invert if required
				d = abs(_Invert - d);

				return float4(d, d, d, 1);
            }
            ENDCG
        }
    }
}
