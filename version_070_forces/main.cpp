/*****************************************************************************/
/*
   main.cpp
   www.xbdev.net
   Simple Effect (fx) Demo with DirectX (Fur)
*/
/*****************************************************************************/

//-----------------------------------------------------------------------------


#define SZ_FX_FILE				"fur.fx" // Our Effect File!
#define	SZ_TEXTURE_FILE			"tiger.jpg"


#define STRICT
#define WIN32_LEAN_AND_MEAN

#define D3D_DEBUG_INFO

#include <windows.h>
#include <assert.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h> // sprintf(..)

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")



#include "XFurTexture.h"
#include "MeshObject.h"  // X File Loader Class

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
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;

//FurTexture				g_FurTexture;
XFurTexture				g_FurTexture;

CMeshObject				g_MeshObject;

static int				g_NumLayers         = 20;    // 20
static float			g_FurLength         = 1.3f;  // 0.7
static float			g_UVScale			= 1.0f;
static bool				g_Shadow			= false;
static bool				g_bForce			= true;
static bool				g_Wire				= false;
static int				g_Density			= 2000;

static float			g_fSpinX			= -25.0f;
static float			g_fSpinY			= 0.0f;
static UINT				g_Layer				= 0;    // For rendering in top left corner


static D3DXVECTOR3      g_vForce            = D3DXVECTOR3(0,0,0);


float DELTAT = .01f;
float SEGLEN = 10.0f;
float SPRINGK = 10.0f;
float MASS = 1.0f;
float GRAVITY = 50.0f;
float RESISTANCE = 10.0f;
float STOPVEL = 0.1f;
float STOPACC = 0.1f;
float DOTSIZE = 11.0f;
float BOUNCE = 0.75f;

struct spring
{
	D3DXVECTOR3 oldpos;
	D3DXVECTOR3 newpos;
	D3DXVECTOR3 force;
	float mass;
	D3DXVECTOR3 velocity;
	D3DXVECTOR3 accel;

	spring()
	{
		reset();
	}

	void reset()
	{
		oldpos = newpos = D3DXVECTOR3(0,0,0);
		force = D3DXVECTOR3(0,0,0);
		velocity = accel = D3DXVECTOR3(0,0,0);
	}
};

spring g_spring;

#define SPRING_TOLERANCE           (0.05f)


/***************************************************************************/
/*                                                                         */
/* springForce()                                                           */
/* Using the two points, pa and pb, we can calculate the Force, from       */
/* hooks spring law, F=kx                                                  */
/* F=Force, k=spring constant, x=distance between still point and the      */
/* stretched spring.                                                       */
/*                                                                         */
/***************************************************************************/
void springForce(D3DXVECTOR3 *force, D3DXVECTOR3 *pa, D3DXVECTOR3 *pb, float k)
{
    D3DXVECTOR3    force_dir;
    float          distance;

    // get the distance
    float  dx =   pb->x - pa->x;
    float  dy =   pb->y - pa->y;
    float  dz =   pb->z - pa->z;

    distance = (float) sqrt ((dx*dx)+(dy*dy)+(dz*dz));

    if (distance < SPRING_TOLERANCE)
    {
        force->x =
        force->y =
        force->z = 0.0f;
        return;
    }//End if(..)

    force_dir.x = dx;
    force_dir.y = dy;
    force_dir.z = dz;

	// k relates to the spring, and how "hard" it will be.
	// The higher k the faster the mass will come back.
    force->x = force_dir.x*k;
    force->y = force_dir.y*k;
    force->z = force_dir.z*k;
}// End springForce(..)


/***************************************************************************/
/*                                                                         */
/* animate_movement()                                                      */
/* A simple function which is called from our render loop, here is where   */
/* we do the calculation of our force effects - basically using the values */
/* from the mouse dragging up and down to bias a small bouncy effect       */
/* of the fur.                                                             */
/* Using newtons second law F=MA and of course hooks spring law, F=ka      */
/*                                                                         */
/***************************************************************************/
void animate_movement()
{
	// if the boolean variable is true, this means that our fur surface
	// will be effected by forces such as gravity and our spring reaction
	// forces from the mouse dragging
	if( g_bForce )
		g_vForce = 0.02f * D3DXVECTOR3(g_spring.newpos.x , -g_spring.newpos.y, 0);

	
	// k relates to the spring, and how "hard" it will be.
	// The higher k the faster the mass will come back.
	float k = 0.1f;

	
	springForce( &g_spring.force,
				 &g_spring.oldpos,
				 &g_spring.newpos,
				 k);
				 
	// if our velocity or accel is less than some minimum amount then
	// our values go to zero!  Else we get a sort of jitterying about
	// the zero value
	if (abs(g_spring.velocity.x) < STOPVEL &&
        abs(g_spring.velocity.y) < STOPVEL &&
        abs(g_spring.accel.x) < STOPACC &&
        abs(g_spring.accel.y) < STOPACC) 
	{
		g_spring.velocity.x = 0;
        g_spring.velocity.y = 0;

		g_spring.newpos = D3DXVECTOR3(0,0,0);
	}//End if(..)
	


	// step distance
	float dt = 1.0f;

	// inertia relates to the quantity of energy that 
	// the spring will carry
	// inertia = 1 would mean that the spring doesn't 
	// loose any energy, and that it will oscillate
	// forever
	float inertia = 0.9f;


	g_spring.accel = g_spring.force / 0.3f; // f=ma or a=f/m

	g_spring.velocity = g_spring.velocity*inertia  +  dt*g_spring.accel;

	g_spring.newpos -= dt*g_spring.velocity;

}//End animate_movement()


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

				case 'S':		g_Shadow=!g_Shadow;			break;
				case 'G':		g_bForce=!g_bForce; 		break;
				case 'W':		g_Wire=!g_Wire;				break;

				case 'Z':		g_Density-=50;				
								if(g_Density<60) g_Density=60;
								g_FurTexture.Release();
								g_FurTexture.Create(383832, 128, 20, g_Density);
								break;

				case 'X':		g_Density+=50;				
								g_FurTexture.Release();
								g_FurTexture.Create(383832, 128, 20, g_Density);
								break;

				case 'A':		g_Layer=max(g_Layer-1,0);	break;
				case 'Q':		g_Layer+=1;					break;

				case 'N':		g_vForce.y-=0.4f;			break;
				case 'M':		g_vForce.y+=0.4f;			break;
            }
			//key = (short)wParam;
        }
        break;

        case WM_LBUTTONDOWN:
        {
            ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
            bMousing = true;
        }
        break;

        case WM_LBUTTONUP:
        {
            bMousing = false;
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
				//char buf[200];
				int dx = ptCurrentMousePosit.x - ptLastMousePosit.x;
				int dy = ptCurrentMousePosit.y - ptLastMousePosit.y;
				
				g_spring.reset();

				g_spring.newpos.x = (float)dx;
				g_spring.newpos.y = (float)dy;

				if( g_spring.newpos.x > 0.0f ) g_spring.newpos.x = 30.0f;
				if( g_spring.newpos.x < 0.0f ) g_spring.newpos.x = -30.0f;
	
				if( g_spring.newpos.y > 0.0f ) g_spring.newpos.y = 30.0f;
				if( g_spring.newpos.y < 0.0f ) g_spring.newpos.y = -30.0f;

				//sprintf(buf, "dx=%d dy=%d", dx, dy);
				//SetWindowText(hWnd, buf);
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
	//g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	//g_pd3dDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	// Initialise our Texture and Mesh Classes
	g_FurTexture.Create(383832, 128, 20, g_Density);
	g_MeshObject.Create(&g_pd3dDevice,"Bunny.x","");
	

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
	g_MeshObject.Release();

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
	// Add the additional spring/gravity forces!
	animate_movement();

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

		D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 10.0f );
		D3DXMatrixRotationYawPitchRoll( &matRot, D3DXToRadian(g_fSpinX), 
												 D3DXToRadian(g_fSpinY), 
												 0.0f );
		
		D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 500.0f );
	    
		g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

		matWorld = matRot * matTrans;

		D3DXMatrixIdentity( &matView ); // This sample is not really making use of a view matrix
		D3DXMATRIX worldViewProj = matWorld * matView * matProj;

		// or SetFloat(..), SetMatrix(..), SetTexture(..)
		pEffect->SetMatrix("worldViewProj", &worldViewProj);
		pEffect->SetMatrix("matWorld", &matWorld);
	}

	DWORD dwColour  = D3DCOLOR_COLORVALUE(0.0f,1.0f,0.0f,1.0f); 
	pEffect->GetValue( "BCLR", &dwColour, sizeof(DWORD) );
	

	if( g_Wire == true )
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	else
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	// Load a simple test texture for wrapping onto the fur surface.
	IDirect3DTexture9* pTextureA;
	D3DXCreateTextureFromFile(g_pd3dDevice, SZ_TEXTURE_FILE, &pTextureA);

	

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, dwColour, 1.0f, 0 );
    g_pd3dDevice->BeginScene();

	// You could skip this little bit if you wanted!  So that our fur isn't just
	// see through, I do a single render so you get a solid black center for the fur	
	{
		pEffect->Begin(&cPasses, 0);	
		//Set the basic textures so we see our mesh even when we have no fur!
		pEffect->SetTexture("ColourTexture", pTextureA);
		pEffect->SetTexture("FurTexture", pTextureA);

		pEffect->BeginPass(0);	
		//Draw our mesh
		g_MeshObject.Render(); // Draw our mesh object

		pEffect->EndPass();
		pEffect->End();
	}
	

	
	// Starting Rendering of our effect
	{
		pEffect->Begin(&cPasses, 0);
		
		// Render Our Shells!
		for(int i=0; i<g_NumLayers; i+=1)
		{
			pEffect->SetInt("shellnumber", -i);
			pEffect->SetInt("Level", i);
			//pEffect->SetFloat("FurDistance", fd);

			float layer = float(i+1) / g_NumLayers;
			float length = g_FurLength * layer;
			
			// Set variables in the shader
			pEffect->SetFloat("FurLength", length);
			pEffect->SetFloat("Layer", layer);

			pEffect->SetFloatArray("vGravity",(float*)&g_vForce, sizeof(D3DXVECTOR3) );
			
			pEffect->SetFloat("UVScale", g_UVScale);
		
			// Get the texture layer which will be used for the different shells
			// Sort of like an onion!..different textures for the different layers :)
			IDirect3DTexture9 * pTexture;
			g_FurTexture.GetTexture(layer, &pTexture);
			pEffect->SetTexture("FurTexture", pTexture);

			pEffect->SetTexture("ColourTexture", pTextureA);

			// Render SHADOW
			if( g_Shadow )
			{
			pEffect->BeginPass(1);	
			g_MeshObject.Render(); // Draw our mesh object
			pEffect->EndPass();
			}//End if(..)

			// Render Textured Fur
			pEffect->BeginPass(0);	
			g_MeshObject.Render(); // Draw our mesh object
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

	pTextureA->Release();

	pEffect->Release();

}//End Render()
