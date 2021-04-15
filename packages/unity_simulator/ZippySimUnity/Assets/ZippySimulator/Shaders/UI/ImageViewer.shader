Shader "Zippy/UI/ImageViewer"
{
	Properties
	{
		_MainTex("Texture", 2D) = "white" {}
	}

	SubShader
	{
		Tags{ "RenderType" = "Opaque" }

		Pass
		{
			CGPROGRAM
			#pragma vertex vert_img
			#pragma fragment frag
			#include "UnityCG.cginc"

			#pragma multi_compile __ IS_GREYSCALE

			sampler2D _MainTex;

			float4 frag(v2f_img i) : SV_Target
			{
				// sample the texture
				half4 color = tex2D(_MainTex, i.uv);
#ifdef IS_GREYSCALE
                color.rgb = color.rrr;
                color.a = 1;
#else
                color.a = 1;
#endif
				return color;
			}
			ENDCG
		}
	}
}
