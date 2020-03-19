//------------------------------------

// fur.fx

//------------------------------------


float FurLength = 0;

DWORD BCLR = 0x999999ff;

texture FurTexture
< 
    string TextureType = "2D";
>;

//------------------------------------
sampler TextureSampler = sampler_state 
{
    texture = <FurTexture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


// transformations
float4x4 worldViewProj : WORLDVIEWPROJ;


//------------------------------------
struct vertexInput {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexOutput {
    float4 HPOS		: POSITION;    
    float4 T0	    : TEXCOORD0;
    float3 normal   : TEXCOORD1;
};


//------------------------------------
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT = (vertexOutput)0;;

	float3 P = IN.position.xyz + (IN.normal * FurLength);
	OUT.HPOS = mul(float4(P, 1.0f), worldViewProj);
	
	OUT.T0 = IN.texCoordDiffuse;
    OUT.normal = IN.normal;
    return OUT;
}




//-----------------------------------
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 diffuseTexture = tex2D( TextureSampler, IN.T0 );
	
	return diffuseTexture;
	//return float4(1.0f, 0.0f, 1.0f, 1.0f); //rrggbbaa
}

//-----------------------------------
technique Fur	        
{
    pass Shell
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture();
		PixelShader  = compile ps_2_0 PS_Textured();
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		//DestBlend = one;
		DestBlend = invsrcalpha;
		//DestBlend = srccolor;
		//DestBlend = invsrccolor;
		//DestBlend = srcalpha;
		//DestBlend = destalpha;
		//DestBlend = invdestalpha;
		//DestBlend = destcolor;
		//DestBlend = invdestcolor;  
		
		CullMode = CCW;  
    }
}

