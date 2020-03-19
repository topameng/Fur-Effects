/*****************************************************************************/
/*
   main.cpp
   www.xbdev.net
   Simple Effect (fx) Demo with DirectX (Fur)
*/
/*****************************************************************************/
/*
   About?
   Well this tutorial/demo code is to show a basic model using layers of how
   to go about generating fur/hairs.  
   All in a single cpp file (main.cpp) and a single hlsl file (fur.fx) and uses
   a single texture to generate a reasonably nice hair/fur effect. 
   Its basically as simple as you can go - and has the added advantages, of
   allowing you to vary values for the demo while its running.
   Such as pressing various keys to adjust what you see - you can see what the
   fur/hair looks like with more layers, or less layers, longer hair, less
   hair...also bias the hairs with a bit of gravity so there tips bend!

   Demo key inputs:

	Up/Down Keys: Fur Lengths

	Home/End: Number of layers

	Left/Right: Scales texture (finer/chubbier fur effect)

	Also you can rotate the object using the mouse while pressing the
	left mouse button.

	Misc Other Additions:

	Z & X for changing the fur density of the texture - slow as it has to
	destroy and rebuid the texture.

	Right mouse button & drag to zoom in and out of the fur.

	Basic gravity effects - pressing G will bias the fur so you get a
	force acting downwards on the hairs/fur layers so you get a moving
	up/down force in real time.
	....

	If you have any problems or feedback please let me know.

	OPTIMISATION!  BUGS!
	This is only meant as a tutorial/demo to show the works of fur/hair with
	layers - as the main render loop is being used to create and release
	the shader code, which is a performance penalty - but it runs smooth enough
	for our liking, so if your going to modify this demo for speed please
	note this!
*/
/*****************************************************************************/
/*****************************************************************************/


#define SZ_FX_FILE "fur.fx" // Our Effect File!


#define STRICT
#define WIN32_LEAN_AND_MEAN

#define D3D_DEBUG_INFO

#include <windows.h>
#include <assert.h>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")



/*****************************************************************************/
/*
   GLOBALS
   As much as we all hate them, its sometimes a little easier...so we've got
   a few globals for the essential of our code.
/*
/*****************************************************************************/

HWND                    g_hWnd              = NULL;
LPDIRECT3D9             g_pD3D              = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice        = NULL;

IDirect3DTexture9*      g_pFurTexture       = NULL;

static int				g_NumLayers         = 60 ; 
static float			g_FurLength         = 1.3f;
static float			g_UVScale			= 1.0f;
static bool				g_bGravity			= false;
static bool				g_Wire				= false;
static int				g_Density			= 2000;

static float			g_fSpinX			= -25.0f;
static float			g_fSpinY			= 0.0f;
static float			g_fZoom             = 5.0f;

static D3DXVECTOR3      g_vGravity          = D3DXVECTOR3(0,0,0);
static D3DXVECTOR3      g_vDirection        = D3DXVECTOR3(0,0.05f,0);


//*************************************************************************//
/*                                                                         */
/* For this simple demo, we use a single flat surface, which is defined    */
/* here and is just two triangles, with normal and texture coords.         */
/*                                                                         */
/***************************************************************************/

struct CUSTOMVERTEX
{
    FLOAT x, y, z;
	FLOAT nx,ny,nz;
    FLOAT tu, tv;
};

CUSTOMVERTEX cvVertices[] ={
      // x      y      z     nx   ny   nz        tu    tv
   { -1.0f, -1.0f, 0.0f,  0.0f,0.0f,-1.0f,     0.0f, 1.0f },  //Front face
   { -1.0f,  1.0f, 0.0f,  0.0f,0.0f,-1.0f,     0.0f, 0.0f },
   {  1.0f,  1.0f, 0.0f,  0.0f,0.0f,-1.0f,     1.0f, 0.0f },
   {  1.0f,  1.0f, 0.0f,  0.0f,0.0f,-1.0f,     1.0f, 0.0f },
   {  1.0f, -1.0f, 0.0f,  0.0f,0.0f,-1.0f,     1.0f, 1.0f },
   { -1.0f, -1.0f, 0.0f,  0.0f,0.0f,-1.0f,     0.0f, 1.0f }
};

UINT FVF_VERTEX (D3DFVF_XYZ|D3DFVF_TEX1);

// This is where DirectX will store our vertex data above - we'll create a
// DirectX Vertex buffer and copy the data across to it.
IDirect3DVertexBuffer9*	g_pVertexBuffer		= NULL;

// Where using Shaders!  HLSL, of course the fur.fx hlsl code should be simple enough
// for it to be ported to Cg/Vertex/Pixel Asm quiet easily.
// This is our vertex shader definition of for our data that we pass to our shader.

const D3DVERTEXELEMENT9 declaration[] = 
{
	{ 0, 0 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
	{ 0, 3 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
	{ 0, 6 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex1
	D3DDECL_END()
};


// Some definitions of random variables so its easier for us to generate random values
// from some value to some max value etc using our favourite and very popular rand()
// math function.
const float INV_RAND_MAX = 1.0 / (RAND_MAX + 1);
inline float rnd(float max=1.0) { return max * INV_RAND_MAX * rand(); }
inline float rnd(float min, float max) { return min + (max - min) * INV_RAND_MAX * rand(); }


/***************************************************************************/
// PROTOTYPES                                                              //
/***************************************************************************/
void init(void);         // - called at the start of our program
void render(void);       // - our mainloop..called over and over again
void shutDown(void);     // - last function we call before we end!

// This is our amazing Texture creating function!  It doesn't do much I guess
// simply creates a texture and plots lots of random points on its surface
// with fixed alpha values for the points.  Its only a single texture, very
// simple but with a bit more work you could extend this to multiple texture
// layers for decreasing density, changing colour, hair direction, plus much
// much more!
bool CreateFurTexture(UINT Size, UINT idensity, IDirect3DTexture9** pTexture);

/***************************************************************************/
/*                                                                         */
/* Handle all messages for the main window here                            */
/*                                                                         */
/***************************************************************************/
long _stdcall MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
    static POINT ptLastMousePosit;
    static POINT ptCurrentMousePosit;
    static bool bMousing;             // Used for Rotating our fur object
	static bool bZooming;             // Zooming into our fur object
    
    switch( uMsg )
    {
        case WM_KEYDOWN:
        {
            switch( wParam )
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;

                case VK_UP:		g_FurLength += 0.1f;		break;
				case VK_DOWN:	g_FurLength -= 0.1f;		break;
					
				case VK_HOME:	g_NumLayers += 1;			break;
				case VK_END:	g_NumLayers -= 1;			break;

				case VK_LEFT:	g_UVScale	-= 0.1f;		break;
				case VK_RIGHT:  g_UVScale   += 0.1f;		break;

				case 'G':		g_bGravity=!g_bGravity;		break;
				case 'W':		g_Wire=!g_Wire;				break;

				// Z & X Are use to modify the texture density, slow as we have
				// to destroy and create the texture surface, but shows a great
				// more detail of whats happening.
				case 'Z':		g_Density-=150;				
								if(g_Density<60) g_Density=60;
								g_pFurTexture->Release();
								CreateFurTexture(128,g_Density, &g_pFurTexture);
								break;

				case 'X':		g_Density+=150;				
								g_pFurTexture->Release();
								CreateFurTexture(128,g_Density, &g_pFurTexture);
								break;

				case 'N':		g_vGravity.y-=0.4f;				break;
				case 'M':		g_vGravity.y+=0.4f;				break;
            }
        }//End WM_KEYDOWN
        break;

        case WM_LBUTTONDOWN:
        {
            ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
            bMousing = true;
        }
        break;

		case WM_RBUTTONDOWN:
        {
            ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
            bZooming = true;
        }
        break;

        case WM_LBUTTONUP:
        {
            bMousing = false;
        }
        break;

		case WM_RBUTTONUP:
        {
			bZooming = false;
        }
        break;

        case WM_MOUSEMOVE:
        {
            ptCurrentMousePosit.x = LOWORD (lParam);
            ptCurrentMousePosit.y = HIWORD (lParam);

            if( bMousing )
            {
                g_fSpinX -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
                g_fSpinY -= (ptCurrentMousePosit.y - ptLastMousePosit.y);
            }

			if( bZooming )
            {
				// 0.2 is so the zoom isn't so fast!  Makes it more user friendly
                g_fZoom += (ptCurrentMousePosit.y - ptLastMousePosit.y) * 0.2f;
            }
            
            ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
        }
	}//End switch(..)

    if(uMsg == WM_DESTROY)
    {
            // We destroy any directx memory we allocated etc, 
            // Tidy up before leaving.
		    shutDown();

            //UnregisterClass( "DirectX3D", winClass.hInstance );
            PostQuitMessage(0);
			return 0;
    }

    return (long)DefWindowProc(hWnd, uMsg, wParam, lParam);
}


/***************************************************************************/
/*                                                                         */
/* Program entry point.                                                    */
/*                                                                         */
/***************************************************************************/
int _stdcall WinMain(HINSTANCE i, HINSTANCE, char* k, int) 
{
    MSG msg;
    char szname[] = "www.xbdev.net - Fur Tutorial";
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      szname, NULL };
    RegisterClassEx( &wc );
    g_hWnd = CreateWindowEx(WS_EX_APPWINDOW,
                              szname, "Basic Bones Sample", 
                              WS_OVERLAPPEDWINDOW,//for fullscreen make into WS_POPUP
                              50, 50, 500,500,    //for full screen GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                              GetDesktopWindow(), NULL, wc.hInstance, NULL);
    
    // Initilise or directX code here!
	init();

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);     
 
    // Message loop. Note that this has been modified to allow
    // us to execute if no messages are being processed.
    while(1)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (!GetMessage(&msg, NULL, 0, 0))
                break;

            DispatchMessage(&msg);
        }
		else
			// Idle-time processing - do our work here!
			render();
    }
    return 0;
}// End WinMain(..)



/*****************************************************************************/
/*                                                                           */
/* CreateFurTexture(..)                                                      */
/*                                                                           */
/*****************************************************************************/
bool CreateFurTexture(UINT Size, UINT idensity, IDirect3DTexture9** pTexture)
{
	static UINT seed = 23234;
	srand(seed);

	D3DXCOLOR *data = new D3DXCOLOR[Size*Size];
	#define DATA(x, y) data[Size*(y) + (x)]

	// Set all the pixels to the same colour, transparent black!
	{
		for (UINT x = 0; x < Size; x++)
			for (UINT y = 0; y < Size; y++)
				DATA(x, y) = D3DXCOLOR(0,0,0,0);
	}

	
	// Lets plot a load of random pixels on our square texture, red
	// Same across all the layers, so we get spikes
	{
		for(UINT density=0; density<idensity; density++)
		{
			int xrand = (int)rnd(0, (float)Size);
			int yrand = (int)rnd(0, (float)Size);
			
			DATA(xrand, yrand) = D3DXCOLOR(1,0,0,1.0f);
		}
	}
	

	// Create the texture in DirectX IDirect3DTexture.  Basically we create
	// a IDirect3DTexture9 texture, lock its surface, copy across the pixels
	// and release it.
	{
		DWORD *pixels = new DWORD[Size*Size];
		DWORD *p = pixels;
		D3DXCOLOR *pColor = &DATA(0, 0);
		for(UINT i=0; i<Size*Size; i++)
			*p++ = DWORD(*pColor++);

			
		g_pd3dDevice->CreateTexture(Size, Size, 1
                                 , 0, D3DFMT_A8R8G8B8
                                 , D3DPOOL_MANAGED, pTexture, NULL );

		
		D3DLOCKED_RECT d3dlr;
		(*pTexture)->LockRect(0,&d3dlr,NULL,0 );

		// pitch value should be h * w * 4 = 128 x 4 = 

		DWORD * pDst = (DWORD *)d3dlr.pBits;
		int DPitch = d3dlr.Pitch/4;
		DWORD * pSrc = (DWORD *)pixels;
		int SPitch = d3dlr.Pitch/4;	// bad assumption, but lets assume there the same pitch!
		
		for (UINT i=0; i<Size/*height*/; ++i)
			for (UINT j=0; j<Size/*width*/; ++j)
				pDst[i*DPitch + j] = pSrc[i*SPitch + j];				
		
		(*pTexture)->UnlockRect (0);

		delete [] pixels;
	}

	delete [] data;

	return true;
}//End CreateFurTexture(..)


/***************************************************************************/
/*                                                                         */
/* init()                                                                  */
/* Called once at the start of our program to setup directx.               */
/*                                                                         */
/***************************************************************************/
void init( void )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

	D3DPOOL_DEFAULT;

    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	// D3DPRESENTFLAG_LOCKABLE_BACKBUFFER

#if(1) // HAL or REF
	
    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );
	
#else
	
	g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );
#endif

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	
	//g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); //D3DCULL_CCW);
	//g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);


	// Lets enable alpha blending so we can have transparency
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	//g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	//g_pd3dDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	g_pd3dDevice->CreateVertexBuffer(sizeof(cvVertices), D3DUSAGE_WRITEONLY, 
					FVF_VERTEX,D3DPOOL_DEFAULT, &g_pVertexBuffer, NULL /*DX 9*/);
	VOID* pVertices;
    g_pVertexBuffer->Lock(0, sizeof(cvVertices), (void**)&pVertices,  0);
    memcpy(pVertices, cvVertices, sizeof(cvVertices));
	g_pVertexBuffer->Unlock();

	//Create our basic random noise Fur Texture - very simple - noise pattern with
	//alpha values
	CreateFurTexture(128, g_Density, &g_pFurTexture);
	

}//End init(..)




/***************************************************************************/
/*                                                                         */
/* shutDown()                                                              */
/* Release any allocated memory, andy directx api's, resources etc before  */
/* our program finally terminates.                                         */
/*                                                                         */
/***************************************************************************/
void shutDown( void )
{
	g_pFurTexture->Release();

	g_pVertexBuffer->Release();

    g_pd3dDevice->Release();
    g_pd3dDevice = NULL;

    g_pD3D->Release();
    g_pD3D = NULL;
}// End shutDown(..)


/***************************************************************************/
/*                                                                         */
/* render()                                                                */
/* Our main render loop, which gets called over and over agian to do our   */
/* drawing...clears the screen, draws the data, then presents it..         */
/*                                                                         */
/***************************************************************************/
void render()
{
	if( g_bGravity )
		g_vGravity += g_vDirection;
	if( g_vGravity.x > 0.5f || g_vGravity.x < -0.5f ) g_vDirection.x= -g_vDirection.x;
	if( g_vGravity.y > 0.5f || g_vGravity.y < -0.5f ) g_vDirection.y= -g_vDirection.y;

	//Loading fx Effect File-------------------------------------------------
	ID3DXEffect * pEffect;
	LPD3DXBUFFER pBufferErrors;
	DWORD dwShaderFlags = 0;
	DWORD iPass = 0;
	UINT cPasses = 0;

	if(FAILED(
		D3DXCreateEffectFromFile( g_pd3dDevice, SZ_FX_FILE,
		                      NULL, // CONST D3DXMACRO* pDefines
							  NULL, // LPD3DXINCLUDE pInclude
							  dwShaderFlags, NULL, &pEffect, &pBufferErrors)
	         ))
	{
		LPVOID pCompileErrors = pBufferErrors->GetBufferPointer(); 
		MessageBox(NULL, (const char*)pCompileErrors, "Compile Error", 
		MB_OK|MB_ICONEXCLAMATION); 
		PostQuitMessage(WM_DESTROY);
	}

	pEffect->SetTechnique("Fur");

	// Setting Shader Constants----------------------------------------------
	{		
		D3DXMATRIX matTrans;
		D3DXMATRIX matRot;
        D3DXMATRIX matProj,matView,matWorld;

		D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, g_fZoom );
		
		D3DXMatrixRotationYawPitchRoll( &matRot, D3DXToRadian(g_fSpinX), 
												 D3DXToRadian(g_fSpinY), 
												 0.0f );
		
		D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 500.0f );
	    
		g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

		matWorld = matRot * matTrans;

		D3DXMatrixIdentity( &matView ); // This sample is not really making use of a view matrix

		D3DXMATRIX worldViewProj = matWorld * matView * matProj;

		pEffect->SetMatrix("worldViewProj", &worldViewProj);
		pEffect->SetMatrix("matWorld", &matWorld);
	}

	DWORD dwColour  = D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,1.0f); 
	pEffect->GetValue( "BCLR", &dwColour, sizeof(DWORD) );
	

	if( g_Wire == true )
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	else
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );


	LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration = NULL;
	g_pd3dDevice->CreateVertexDeclaration( declaration, &pVertexDeclaration );
	g_pd3dDevice->SetVertexDeclaration( pVertexDeclaration );

	// Begin Rendering-------------------------------------------------------
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, dwColour, 1.0f, 0 );
    g_pd3dDevice->BeginScene();
	
	g_pd3dDevice->SetStreamSource(0, g_pVertexBuffer, 0 /*DX 9*/, sizeof(CUSTOMVERTEX));


	pEffect->SetTexture("FurTexture", g_pFurTexture);

	// You could skip this little bit if you wanted!  So that our fur isn't just
	// see through, I do a single render so you get a solid black center for the fur
	/*
	{
		pEffect->Begin(&cPasses, 0);	
		pEffect->BeginPass(0);
		//Draw our mesh
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
		//End of our Effect Render
		pEffect->EndPass();
		pEffect->End();
	}
	*/


	// Starting Rendering of our effect--------------------------------------
	{
		pEffect->Begin(&cPasses, 0);
		
		// Render Our Shells!
		for(int i=0; i<g_NumLayers; i+=1)
		{
			float layer = float(i+1) / g_NumLayers;
			float length = g_FurLength * layer;

			// Set variables in the shader
			pEffect->SetFloat("FurLength", length);
			pEffect->SetFloat("Layer", layer);
			pEffect->SetFloatArray("vGravity",(float*)&g_vGravity, sizeof(D3DXVECTOR3) );
			
			pEffect->SetFloat("UVScale", g_UVScale);

			// Get the texture layer which will be used for the different shells
			// Sort of like an onion!..different textures for the different layers :)
			pEffect->SetTexture("FurTexture", g_pFurTexture);


			// Render Textured Fur
			pEffect->BeginPass(0);	
			// Draw our mesh object
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
			// End of our Effect Pass
			pEffect->EndPass();
		}//End for loop

		pEffect->End();		
	}

	pEffect->SetTechnique(NULL);
	
	////////////////////////////////////////////////////////////////////////////////
	// Debug - Texture Information - www.xbdev.net (bkenwright@xbdev.net          //
	// Great! Amazing few lines, which if you think about it can be used for all  //
	// sorts of things...like rendering different camera views from different     //
	// positions etc.                                                             //
	////////////////////////////////////////////////////////////////////////////////
	#if 1
	{
		struct TLVERTEX
		{
			float x,y,z,rhw;
			float tu,tv;
		};
		#define	FVF_TLVERTEX (D3DFVF_XYZRHW | D3DFVF_TEX1)

		//g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

		g_pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
		g_pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1,	D3DTA_TEXTURE);
		float scale = 128.0f;
		TLVERTEX Vertex[4] = 
		{
			// x  y  z rhw tu tv
			{    0,    0,0, 1, 0, 0,},
			{scale,    0,0, 1, 1, 0,},
			{scale,scale,0, 1, 1, 1,},
			{    0,scale,0, 1, 0, 1,},
		};
	
		g_pd3dDevice->SetTexture(0, g_pFurTexture);
		g_pd3dDevice->SetVertexShader( NULL );
		g_pd3dDevice->SetFVF( FVF_TLVERTEX );
		g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, Vertex, sizeof( TLVERTEX ) );
	}
	#endif




    g_pd3dDevice->EndScene();
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	pVertexDeclaration->Release();
	pEffect->Release();

}//End Render()



