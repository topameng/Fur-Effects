/*****************************************************************************/
/*                                                                           */
/* File: fur.fx                                                              */
/* bkenwright@xbdev.net                                                      */
/* www.xbdev.net                                                             */
/*                                                                           */
/*****************************************************************************/
/*
   Theres lots of various methods of generating fur/hair effects, from modelling
   simple individual sets of hairs, to using tricks like volume rendering.
   
   We use the technique of layers/shells which produces stunning effects and
   of course allows us to create hairs in real time (e.g. games, demos etc)
   
   The main pixel/vertex shader techinque is Shells{..}, which produces
   the fur effect - the additional techniques in this demo shader, Shadow{..}
   and NoFur{..} are used to add additional features, such as shadow layers
   and render a single fur layer.
   
   The Fur/Hair Direction is taken from the object surface normal, but of course
   with a few lines of modifications you can take the normal from a 
   normal texture map to create more stunning and realistic hair!
   
*/
/*****************************************************************************/

float FurScale = 0;
float FurLength = 0;
float UVScale = 3.0f;
float Layer = 0; // 0 to 1 for the level

float3 vGravity = float3(0,-2.0,0); 


DWORD BCLR = 0x00ffffff;

float Level = 0;

float4 vecLightDir = float4(0.8,0.8,-1,0); 

texture FurTexture
< 
    string TextureType = "2D";
>;

texture ColourTexture
< 
    string TextureType = "2D";
>;

//---------------------------------------------------------------------------//
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

//---------------------------------------------------------------------------//
sampler TextureSamplerB = sampler_state 
{
    texture = <ColourTexture>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

//---------------------------------------------------------------------------//

// transformations
float4x4 worldViewProj : WORLDVIEWPROJ;
float4x4 matWorld : WORLD;

//------------------------------------
struct vertexInput {
    float3 position				: POSITION;
    float3 normal				: NORMAL;
    float4 texCoordDiffuse		: TEXCOORD0;
};

struct vertexOutput {
    float4 HPOS		: POSITION;    
    float4 T0	    : TEXCOORD0; // fur alpha
    float4 T1	    : TEXCOORD1; // fur offset / interfur shadowing
    float3 normal   : TEXCOORD2;
};

//---------------------------------------------------------------------------//
vertexOutput VS_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT = (vertexOutput)0;

	float3 P = IN.position.xyz + (IN.normal * FurLength);
	float3 normal = normalize(mul(IN.normal, (float3x3)matWorld));
	
	// Additional Gravit/Force Code
	float3 gravity = mul(vGravity, (float3x3)matWorld);
	float k =  pow(Layer, 3);  // We use the pow function, so that only the tips 
	                           // of the hairs bend  As layer goes from 0 to 1, 
	                           // so by using pow(..) function is still goes from 
	                           // 0 to 1, but it increases faster! exponentially
	P = P + gravity*k;
	// End Gravity Force Addit Code


	OUT.T0 = IN.texCoordDiffuse * UVScale;
	// UVScale??  Well we scale the fur texture alpha coords so this effects the 
	// fur thickness/thinness, we don't change our T1 value as this is our 
	// actual texture!
	
	OUT.HPOS = mul(float4(P, 1.0f), worldViewProj);
    OUT.normal = normal;
    return OUT;
}



//---------------------------------------------------------------------------//
vertexOutput VS_Shadow_TransformAndTexture(vertexInput IN) 
{
    vertexOutput OUT = (vertexOutput)0;
	
	float3 P = IN.position.xyz + (IN.normal * FurLength);
	float4 normal = mul(IN.normal, matWorld);
	
	//---Additional Gravit/Force Code----
	float3 gravity = mul(vGravity, (float3x3)matWorld);
	float k =  pow(Layer, 3);  // We use the pow function, so that only the tips of the hairs bend
	                           // As layer goes from 0 to 1, so by using pow(..) function is still 
	                           // goes form 0 to 1, but it increases faster! exponentially
	P = P + gravity * k;
	//---End Gravity Force Addit Code----
	
	// We want the fur in the center of the object, as where doing offsets!  So if we just
	// use the normal, when its facing in the z direction, the center of the object mesh,
	// then it will be the same when we scale it larger.  So we modify the normal so we
	// have larger values in the center and less on the edge for use in our offset inter
	// fur shading!  If thats makes any sense?...not the best explanation I've ever done :(
	float4 znormal = 1 - dot(normal, float4(0,0,1,0));
	// Works pretty well though, even if you just do this below, and just use the normal!
	//float4 znormal = normal;


	OUT.T0 = IN.texCoordDiffuse * UVScale;
	OUT.T1 = IN.texCoordDiffuse * UVScale + znormal * 0.0011;
	// UVScale??  We only multiply the UVScale by the T0 & T1 as this is our Fur Alpha value!, hence
	// scaling this value scales the fur across your object...so reducing it makes the fur thicker,
	// increasing it makes it thinner.  We don't do it to our T2 output as this is our texture
	// coords for our texture...and we don't want to effect this

	OUT.HPOS = mul(float4(P, 1.0f), worldViewProj);
    OUT.normal = normal;
    return OUT;
}


//---------------------------------------------------------------------------//
float4 PS_Textured( vertexOutput IN): COLOR
{
	float4 FurTexture = tex2D( TextureSampler,  IN.T0 ); // Fur Texture - alpha is VERY IMPORTANT!
  
    //return FurTexture; // Return just the fur colour (red in this demo)
    //return (ColourTexture * (1,1,1,FurTexture.a));//Return texture colour looking like fur!
                                                    // Colour texture would be any other basic object
                                                    // texture we pass along!
	
	float4 FinalColour = FurTexture;
	
	//--------------------------
	
	//Basic Directional Lighting
	float4 ambient = {0.3, 0.3, 0.3, 0.0};
	ambient = ambient * FinalColour;
	float4 diffuse = FinalColour;
	FinalColour = ambient + diffuse * dot(vecLightDir, IN.normal);
	//End Basic Lighting Code
	
	//--------------------------
	
	FinalColour.a = FurTexture.a;
	//return FurTexture;      // fur colour only!
	return FinalColour;       // Use texture colour
	//return float4(0,0,0,0); // Use for totally invisible!  Can't see
}

//---------------------------------------------------------------------------//
float4 PS_NoFur_Textured( vertexOutput IN): COLOR
{
	//float4 FurTexture = tex2D( TextureSampler,  IN.T0 ); // Fur Texture - alpha is VERY IMPORTANT!
	float4 FurTexture = float4(0,0.5,0.5,1);
	float4 FinalColour = FurTexture;
	
	//--------------------------
	
	//Basic Directional Lighting
	float4 ambient = {0.3, 0.3, 0.3, 0.0};
	ambient = ambient * FinalColour;
	float4 diffuse = FinalColour;
	FinalColour = ambient + diffuse * dot(vecLightDir, IN.normal);
	//End Basic Lighting Code
	
	//--------------------------
	
	FinalColour.a = 1.0f;     // We set our alpha value to 1.0f, so we have no fur effect
	return FinalColour;       // Use texture colour
}//End PS_NoFur_Textured(..)

//---------------------------------------------------------------------------//

float4 PS_Shadow_Textured( vertexOutput IN ): COLOR
{
	float4 furcolr        =  tex2D( TextureSampler, IN.T0 );
	float4 furcolr_offset =  tex2D( TextureSampler, IN.T1 );
    
	//??We use a simple offset trick to give individual hair shadows.  Works by using
	//the normal  - furcolor_offset in the direction of the normal of the triangle
	//face.
	//Of course we scale this by a value so our offset is only small, but just
	//enough to give some individual hair lighting
    //--------------------------
	float4 color = furcolr_offset - furcolr;
	
	float4 fcolor = color;
	fcolor.a = color.a;
	//--------------------------
	
	//??We have our offset colour - but of course our fur colour could be a single
	// colour, red or just green!  So we want this as a grey, as we are concerned
	// with the fur shadows!
	//-------------------------
	// From RGB to YUV   
    // Y = 0.299R + 0.587G + 0.114B   
    // U = 0.492 (B-Y)   
    // V = 0.877 (R-Y)   
    
    // From YUV to RGB   
    // R = Y + 1.140V   
    // G = Y - 0.395U - 0.581V   
    // B = Y + 2.032U
    
    // Y is the luma, and contains most of the information of the image
    float4 Y    = float4(0.299, 0.587, 0.114, 0.0f);
    fcolor  = dot(Y, fcolor); // grey output
	//-------------------------
	
	
	//??Add simple fur lighting - basically if the fur is in the dark, we draw
	// them darker, the same as we do in the normal loop
	//--------------------------
	// SKIPPING DIRECTIONAL LIGHTING???...WHY???
	// Well I found that when we add in directional lighting to our shadow values,
	// it sort of cancels out the effect - makes our inter fur effect less noticble!
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//Basic Directional Lighting
	/*
	float4 ambient = {0.3, 0.3, 0.3, 0.0};
	ambient = ambient * fcolor;
	float4 diffuse = fcolor;
	fcolor = ambient + diffuse * dot(vecLightDir, IN.normal);
	//End Basic Lighting Code
	*/
	//--------------------------
	
	//return float4(color.a, color.a, color.a, color.a);
	return fcolor;
	
	
	//return float4(1.0f, 1.0f, 1.0f, 0.3f); //rrggbbaa
}

//---------------------------------------------------------------------------//
technique Fur	        
{   
    pass Shells
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture();
		PixelShader  = compile ps_2_0 PS_Textured();
		AlphaBlendEnable = true;   
//---------------------------------------------------------------------------//
		// Final = Dest * DestBlend + Src * SrcBlend, where dest is the pixel 
		// on the screen and src is the texel color.
		// blends in D3D are: SRCCOLOR, INVSRCCOLOR, SRCALPHA, INVSRCALPHA, 
		// DESTCOLOR, INVDESTCOLOR, DESTALPHA, INVDESTALPHA, ONE, and ZERO.

		// Final = Dest * DestBlend + Src * SrcBlend
		
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
		
		CullMode = None;
		//CullMode = CCW;
    }
    
    pass Shadows
    {		
		VertexShader = compile vs_1_1 VS_Shadow_TransformAndTexture();
		PixelShader  = compile ps_2_0 PS_Shadow_Textured();
		AlphaBlendEnable = true;
		SrcBlend = srcalpha; // **
		//SrcBlend = one;
		DestBlend = invsrcalpha;
		
		//CullMode = CCW;
		CullMode = None;
    }
    
    pass NoFur
    {		
		VertexShader = compile vs_1_1 VS_TransformAndTexture();
		PixelShader  = compile ps_2_0 PS_NoFur_Textured();
		//AlphaBlendEnable = true;
		AlphaBlendEnable = false;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		CullMode = None;
		//CullMode = CCW;
    }
    
}// End technique Fur

