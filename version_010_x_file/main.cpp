/*****************************************************************************/
/*
   main.cpp
   www.xbdev.net
   Simple Effect (fx) Demo with DirectX
*/
/*****************************************************************************/

//-----------------------------------------------------------------------------


#define SZ_X_FILE "mouse.x"    // Our DirectX 3D Object X File

#define SZ_FX_FILE "fur.fx"    // Our Shader File for the Fur Effect


#define STRICT
#define WIN32_LEAN_AND_MEAN

#define D3D_DEBUG_INFO

#include <windows.h>
#include <assert.h>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")



#include "XFurTexture.h"      // Our code for generating the fur textures



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

XFurTexture				g_FurTexture;  // class defined in XFurTexture.h

static int				g_NumLayers          = 40;
static float			g_FurLength          = 5.0f;




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
    char szname[] = "Fur / Hair Shader Demo - www.xbdev.net";
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
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );


	// Lets enable alpha blending so we can have transparency
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );


	// Seed Value - for srand()
	// Size in pixels of texture (square x = y = size)
	// Number of layers
	// Maximum hair density

	g_FurTexture.Create(383832, 128,  20, 6000);
	//                    |      |    |     |
	// Seed---------------+      |    |     |
	// Size x == y --------------+    |     |
	// Num Layers---------------------+     |
	// Max Hair Density---------------------+

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
    g_pd3dDevice->Release();
    g_pd3dDevice = NULL;

    g_pD3D->Release();
    g_pD3D = NULL;
}// End shutDown(..)


/***************************************************************************/
/*                                                                         */
/* render()                                                                */
/* Our main render loop!  Well in this demo, its our gameloop, as its      */
/* called over and over again, and is responsible for clearing hte screen  */
/* and rendering our new informaiton to it.                                */
/*                                                                         */
/* MESSY! Just so newbies and people looking at this code are aware! You   */
/* wouldn't create and release textures and effect files in the main loop  */
/* it really slows your performance down - but it sure makes the code      */
/* easier to follow and understand for tutorials I find...so remember      */
/* if you want to expand on this, use creation and release funtions!       */
/*                                                                         */
/***************************************************************************/
void render()
{
	//Loading the X-File
	LPD3DXBUFFER pMtrlBuffer = NULL;
	DWORD numMaterials;
	LPD3DXMESH amesh;

	// File Name ---------+
	//                    |
	//                   \|/
	//                    |
	D3DXLoadMeshFromX( SZ_X_FILE, D3DXMESH_SYSTEMMEM, g_pd3dDevice,
								NULL, &pMtrlBuffer, NULL, &numMaterials, &amesh);      


	// We've loaded our mesh from file, but we only want, xyz position data and diffuse
	// colour information for each vertice.  So we create a new one with the correct
	// FVF taht we need and get ride of the old one.
	LPD3DXMESH mesh;
	amesh->CloneMeshFVF( D3DXMESH_SYSTEMMEM, D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1, 
		                                     g_pd3dDevice, &mesh );
	amesh->Release();


	// Just incase our mesh doesn't have normals, we'll computer them.
	//D3DXComputeNormals(mesh, NULL /* , NULL - dx9*/);

	//---------------------------------------------------------------------//

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

	LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration = NULL;

	const D3DVERTEXELEMENT9 declaration[] = 
	{
		{ 0, 0 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 6 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	
	g_pd3dDevice->CreateVertexDeclaration( declaration, &pVertexDeclaration );
	
	pEffect->SetTechnique("Fur");

	// Setting Shader Constants
	{	
		D3DXMATRIX matTrans;
		D3DXMATRIX matRot;
        D3DXMATRIX matProj,matView,matWorld;

		//D3DXMatrixTranslation( &matTrans, 0.0f, -2.5f, 7.0f );
		D3DXMatrixTranslation( &matTrans, 0.0f, -2.0f, 120.0f );
		D3DXMatrixRotationYawPitchRoll( &matRot, 2.8f, 0.1f, 0.0f );
		
		D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 500.0f );
	    // This line isn't really needed for shaders! As we are doing our own
		// transforms
		g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

		matWorld = matRot * matTrans;

		D3DXMatrixIdentity( &matView ); // This sample is not really making use of a view matrix

		D3DXMATRIX worldViewProj = matWorld * matView * matProj;

		pEffect->SetMatrix("worldViewProj", &worldViewProj);
	}




	DWORD dwColour  = D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,1.0f); 
	HRESULT hrError = pEffect->GetValue( "BCLR", &dwColour, sizeof(DWORD) );
	// if( hrError = 0x0 ) // If "BCLR" isn't set in our fx file, set it to light green

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         dwColour, 1.0f, 0 );

    g_pd3dDevice->BeginScene();

	// We set a Vertex Declaration, which defines to our Shader what
	// format our data is in!
	g_pd3dDevice->SetVertexDeclaration( pVertexDeclaration );

	// Starting Rendering of our effect
	{
		pEffect->Begin(&cPasses, 0);

		for(int i=0; i<g_NumLayers; i+=1)
		{

			float layer = float(i+1) / g_NumLayers;
			float length = g_FurLength * layer;

			pEffect->SetFloat("FurLength", length);
				
			IDirect3DTexture9 * pTexture;
			g_FurTexture.GetTexture(layer, &pTexture);
			pEffect->SetTexture("FurTexture", pTexture);
			
			//
			pEffect->BeginPass(0);	
			mesh->DrawSubset(2); // We just render the skin of our x-file
			                     // which in this demo is x mesh 2
			pEffect->EndPass();
		}// End for loop
			
		pEffect->End();
	}
	

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );


	// Release our X File Mesh and Materials
	pMtrlBuffer->Release();
	mesh->Release();

	pVertexDeclaration->Release();
	
	// Release our HLSL Effect
	pEffect->Release();

}//End Render()
