/*****************************************************************************/
/*
   main.cpp
   www.xbdev.net
   Simple Effect (fx) Demo with DirectX (Fur)
*/
/*****************************************************************************/
/*
   The simple code, uses two triangles to create a square plane, which are
   defined below.  The surface has texture coords and a normal, this is
   used to create a fur texture on it.

   The XFurTexture.h/.cpp class contain the XFurTexture class which generates
   a set of textures which are used for rendering across numberous layers on the
   triangle surface, to produce a hair/fur effect.

   This demo contains additional code in the HLSL (shader fur.fx) for basic
   lighting and forces so that you can watch the effects of gravity 
   etc.

   You can adjust the fur thickness, height, length, etc by pressing various
   keys at runtime.

   Demo key inputs:

	Up/Down Keys: Fur Lengths

	Home/End: Number of layers

	Left/Right: Scales texture (finer/chubbier fur effect)

	Also you can rotate the object using the mouse while pressing the
	left mouse button.

	Misc Other Additions:
	Q & A keys are used for looking at the various texture layers in the
	top left corner.

	Z & X for changing the fur density of the texture - slow as it has to
	destroy and rebuid the texture.

	G - switching forces/gravity on and off.
	....
*/
/*****************************************************************************/


#define SZ_FX_FILE "fur.fx" // Our Effect File!


#define STRICT
#define WIN32_LEAN_AND_MEAN

#define D3D_DEBUG_INFO

#include <windows.h>
#include <assert.h>
#include <d3d9.h>
#include <d3dx9.h>

#include <stdio.h>  //sprintf(..)

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")


#include "XFurTexture.h"

/*****************************************************************************/
/*
   GLOBALS
   As much as we all hate them, its sometimes a little easier...so we've got
   a few globals for the essential of our code.
/*
/*****************************************************************************/

HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;


XFurTexture				g_FurTexture;

static int				g_NumLayers         = 60 ;    // 20
static float			g_FurLength         = 1.3f;   // 0.7
static float			g_UVScale			= 1.0f;
static bool				g_Shadow			= false;
static bool				g_bGravity			= true;
static bool				g_Wire				= false;
static int				g_Density			= 2000;

static float			g_fSpinX			= -25.0f;
static float			g_fSpinY			= 0.0f;
static float			g_fZoom             = 5.0f;
static UINT				g_Layer				= 0;


static D3DXVECTOR3      g_vGravity          = D3DXVECTOR3(0,0,0);
static D3DXVECTOR3      g_vDirection        = D3DXVECTOR3(0,0.05,0); //0.1


//*************************************************************************//

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

IDirect3DVertexBuffer9*	g_pVertexBuffer		= NULL;

const D3DVERTEXELEMENT9 declaration[] = 
{
	{ 0, 0 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION, 0 },  // pos
	{ 0, 3 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },   // normal
	{ 0, 6 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // tex1
	D3DDECL_END()
};

/***************************************************************************/
// PROTOTYPES                                                              //
/***************************************************************************/
void init(void);         // - called at the start of our program
void render(void);       // - our mainloop..called over and over again
void shutDown(void);     // - last function we call before we end!


/***************************************************************************/
/*                                                                         */
/* Handle all messages for the main window here                            */
/*                                                                         */
/***************************************************************************/
long _stdcall MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
    static POINT ptLastMousePosit;
    static POINT ptCurrentMousePosit;
    static bool bMousing;
	static bool bZooming;
    
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
					
				case VK_HOME:	g_NumLayers += 1.0f;		break;
				case VK_END:	g_NumLayers -= 1.0f;		break;

				case VK_LEFT:	g_UVScale	-= 0.1f;		break;
				case VK_RIGHT:  g_UVScale   += 0.1f;		break;

				case 'S':		g_Shadow=!g_Shadow;			break;
				case 'G':		g_bGravity=!g_bGravity;		break;
				case 'W':		g_Wire=!g_Wire;				break;

				case 'Z':		g_Density-=150;				
								if(g_Density<60) g_Density=60;
								g_FurTexture.Release();
								g_FurTexture.Create(383832, 128, 20, g_Density);
								break;

				case 'X':		g_Density+=150;				
								g_FurTexture.Release();
								g_FurTexture.Create(383832, 128, 20, g_Density);
								break;

				case 'A':		g_Layer=max(g_Layer-1,0);	break;
				case 'Q':		g_Layer+=1;					break;

				case 'N':		g_vGravity.y-=0.4f;				break;
				case 'M':		g_vGravity.y+=0.4f;				break;
            }
			char buf[200];
			sprintf(buf, "layers: %d", g_NumLayers);
			SetWindowText(hWnd, buf);

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
    char szname[] = "DirectX3D";
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      szname, NULL };
    RegisterClassEx( &wc );
    g_hWnd = CreateWindowEx(WS_EX_APPWINDOW,
                     szname, "Basic Bones Sample", 
                     WS_OVERLAPPEDWINDOW,//for fullscreen make into WS_POPUP
                     50, 50, 500,500,    //for full screen 
										  // GetSystemMetrics(SM_CXSCREEN), 
										  // GetSystemMetrics(SM_CYSCREEN),
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
	//g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );


	// Lets enable alpha blending so we can have transparency
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	//g_pd3dDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	g_pd3dDevice->CreateVertexBuffer(sizeof(cvVertices), D3DUSAGE_WRITEONLY, 
					FVF_VERTEX,D3DPOOL_DEFAULT, &g_pVertexBuffer, NULL /*DX 9*/);
	VOID* pVertices;
    g_pVertexBuffer->Lock(0, sizeof(cvVertices), (void**)&pVertices,  0);
    memcpy(pVertices, cvVertices, sizeof(cvVertices));
	g_pVertexBuffer->Unlock();


	// Initialise our Texture and Mesh Classes
	g_FurTexture.Create(383832, 128, 20, g_Density);


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
	if( g_vGravity.x > 1.0f || g_vGravity.x < -1.0f ) g_vDirection.x= -g_vDirection.x;
	if( g_vGravity.y > 1.0f || g_vGravity.y < -1.0f ) g_vDirection.y= -g_vDirection.y;

	//Loading fx Effect File
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

	// Setting Shader Constants
	{		
		D3DXMATRIX matTrans;
		D3DXMATRIX matRot;
        D3DXMATRIX matProj,matView,matWorld;

		D3DXMatrixTranslation( &matTrans, 0.0f, -0.5f, g_fZoom );
		
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


	// If the value BCLR is set, get it and use it to clear our background
	// colour to this value, else set it to green.
	DWORD dwColour  = D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,1.0f); 
	pEffect->GetValue( "BCLR", &dwColour, sizeof(DWORD) );
	

	if( g_Wire == true )
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	else
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );


	LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration = NULL;
	g_pd3dDevice->CreateVertexDeclaration( declaration, &pVertexDeclaration );
	g_pd3dDevice->SetVertexDeclaration( pVertexDeclaration );

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, dwColour, 1.0f, 0 );
    g_pd3dDevice->BeginScene();
	
	g_pd3dDevice->SetStreamSource(0, g_pVertexBuffer, 0 /*DX 9*/, sizeof(CUSTOMVERTEX));


	// Get the 0th layer of our texture fur, and set it to our shader.
	IDirect3DTexture9 * pTexture;
	g_FurTexture.GetTexture(3, &pTexture);
	pEffect->SetTexture("FurTexture", pTexture);

	// You could skip this little bit if you wanted!  So that our fur isn't just
	// see through, I do a single render so you get a solid black center for the fur
	{
		pEffect->Begin(&cPasses, 0);	
		pEffect->BeginPass(2);	// PS_NoFur_Textured is Pass 2
		//Draw our mesh
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
		//End of our Effect Render
		pEffect->EndPass();
		pEffect->End();
	}
	


	// Starting Rendering of our effect
	{
		pEffect->Begin(&cPasses, 0);
		
		// Render Our Shells!
		for(int i=0; i<g_NumLayers; i+=1)
		{
			float layer = float(i+1) / g_NumLayers;
			float length = g_FurLength * layer;
			float scale = -g_FurLength * (1.0*layer*layer + 0.4*layer);

			// Set variables in the shader
			//pEffect->SetFloat("FurScale", scale);
			pEffect->SetFloat("FurScale", scale);
			pEffect->SetFloat("FurLength", length);
			pEffect->SetFloat("Layer", layer);
			pEffect->SetFloatArray("vGravity",(float*)&g_vGravity, sizeof(D3DXVECTOR3) );
			
			pEffect->SetFloat("UVScale", g_UVScale);

			// Get the texture layer which will be used for the different shells
			// Sort of like an onion!..different textures for the different layers :)
			IDirect3DTexture9 * pTexture;
			g_FurTexture.GetTexture(layer, &pTexture);
			pEffect->SetTexture("FurTexture", pTexture);

			//HRESULT hr = pEffect->SetTexture("ColourTexture", pTextureA);

			// Render SHADOW
			if( g_Shadow )
			{
			pEffect->BeginPass(1);
			// Draw our mesh object
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
			// End of our effect drawing pass
			pEffect->EndPass();
			}//End if(..)

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
		IDirect3DTexture9 * pTexture;
		g_FurTexture.GetTexture(float(g_Layer+1) / g_NumLayers, &pTexture);
		//g_FurTexture.GetTexture(g_Layer, &pTexture);

		g_pd3dDevice->SetTexture( 0, pTexture );
		g_pd3dDevice->SetVertexShader( NULL );
		g_pd3dDevice->SetFVF( FVF_TLVERTEX );
		g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, Vertex, sizeof( TLVERTEX ) );
	}
	#endif




    g_pd3dDevice->EndScene();
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	//pTextureA->Release();
	pVertexDeclaration->Release();
	pEffect->Release();

}//End Render()
