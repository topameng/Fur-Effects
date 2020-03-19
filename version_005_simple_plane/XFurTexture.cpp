/*****************************************************************************/
/*                                                                           */
/* File: XFurTexture.cpp                                                     */
/*                                                                           */
/*****************************************************************************/

#include "XFurTexture.h"

#include <d3d9.h>
#include <d3dx9.h>

const float INV_RAND_MAX = 1.0 / (RAND_MAX + 1);
inline float rnd(float max=1.0) { return max * INV_RAND_MAX * rand(); }
inline float rnd(float min, float max) { return min + (max - min) * INV_RAND_MAX * rand(); }

extern LPDIRECT3DDEVICE9       g_pd3dDevice;


/*****************************************************************************/
/*                                                                           */
/* XFurTexture()                                                              */
/*                                                                           */
/*****************************************************************************/
XFurTexture::XFurTexture()
{
	m_Size		= 0;
	m_NumLayers	= 0;
}//End XFurTexture()


/*****************************************************************************/
/*                                                                           */
/* ~XFurTexture()                                                             */
/*                                                                           */
/*****************************************************************************/
XFurTexture::~XFurTexture()
{
	for (UINT layer = 0; layer < m_NumLayers; layer++)
		m_Textures[layer]->Release();
}//End ~XFurTexture()

/*****************************************************************************/
/*                                                                           */
/* Release()                                                                 */
/*                                                                           */
/*****************************************************************************/
void XFurTexture::Release()
{
	for (UINT layer = 0; layer < m_NumLayers; layer++)
	{
		m_Textures[layer]->Release();
		m_Textures[layer]=0;
	}
	m_Size = 0;
	m_NumLayers=0;

}//End ~XFurTexture()


/*****************************************************************************/
/*                                                                           */
/* Create(..)                                                                */
/*                                                                           */
/*****************************************************************************/
bool XFurTexture::Create(UINT seed, UINT size, UINT num, UINT idensity)
{
	srand(seed);
	m_Size      = size;
	m_NumLayers = num;

	D3DXCOLOR *data = new D3DXCOLOR[m_NumLayers*m_Size*m_Size];
	#define DATA(layer, x, y) data[m_Size*m_Size*(layer) + m_Size*(y) + (x)]

	// Set all the pixels to the same colour, transparent black!
	{
	for(UINT layer=0; layer<m_NumLayers; layer++)
		for (UINT x = 0; x < m_Size; x++)
			for (UINT y = 0; y < m_Size; y++)
				DATA(layer, x, y) = D3DXCOLOR(0,0,0,0);
	}

	
	// Lets plot a load of random pixels on our square texture, red
	// Same across all the layers, so we get spikes
	/*
	{
		for(int density=0; density<idensity; density++)
		{
			int xrand = rnd(0, m_Size);
			int yrand = rnd(0, m_Size);
			
			for(UINT layer=0; layer<m_NumLayers; layer++)
			{
				DATA(layer, xrand, yrand) = D3DXCOLOR(1,0,0,1.0f);
			}
		}
	}
	*/


	// This is slightly different than above, we now do it so that
	// as we move to the outer layers, we have less and less strands of hair
	
	{
		for(UINT layer=0; layer<m_NumLayers; layer++)
		{	
			float length = float(layer)/m_NumLayers; // 0 to 1
			length = 1 - length; // 1 to 0
			// *3 is just a value I picked by trial and error so the fur looks thick enough
			// doesn't really need to be here though!...can be adjusted externally
			int density = idensity * length* 3;

			// Alternatives for increasing density - creating different fur effects
			// Increasing by power
			// int density = idensity * pow(length,3);
			// Increasing sine
			 //int density = idensity * sin(length*(D3DX_PI/2));
			srand(28382);
			for(int i=0; i<density; i++)
			{
				int xrand = rnd(0, m_Size);
				int yrand = rnd(0, m_Size);
				
				DATA(layer, xrand, yrand) = D3DXCOLOR(0,0.5,0.5,1.0f);
			}
		}
	}
	

	// Loop across all the pixels, and make the top layer of pixels more
	// transparent (top hairs)
	{
		for (UINT x = 0; x < m_Size; x++)
			for (UINT y = 0; y < m_Size; y++)
				for(UINT layer=0; layer<m_NumLayers; layer++)
				{
				// length of the hair
				float length = (float)layer/m_NumLayers; // 0 to 1
			
				// tip of the hair is semi-transparent
				float alpha = DATA(layer, x, y).a;
				if( alpha > 0.0f )
					DATA(layer, x, y).a = 1-length; // More transparent as we get closer to 
				                                    // the tip ,length is from 0 to 1, so the
				                                    // tip or outer layer is 1.
				}
	}

	
	// Well now, hairs that are closer to the center are darker as they get less light
	// so lets make hairs closer to the center darker?
	{
		for (UINT x = 0; x < m_Size; x++)
			for (UINT y = 0; y < m_Size; y++)
				for(UINT layer=0; layer<m_NumLayers; layer++)
				{
					// length of the hair
					float length = (float)layer/m_NumLayers; // 0 to 1
				
					// tip of the hair is semi-transparent
					float scale = 1-length;
					scale = max(scale, 0.9);
					float alpha = DATA(layer, x, y).a;
					if( alpha > 0.0f )
					{
						D3DXCOLOR cc = DATA(layer, x, y);
						DATA(layer, x, y).r *= scale;
						DATA(layer, x, y).g *= scale;
						DATA(layer, x, y).b *= scale;
					}
				}
	}



	// Create all the textures
	for (UINT layer = 0; layer < m_NumLayers; layer++)
	{
		DWORD *pixels = new DWORD[m_Size*m_Size];
		DWORD *p = pixels;
		D3DXCOLOR *pColor = &DATA(layer, 0, 0);
		for(UINT i=0; i<m_Size*m_Size; i++)
			*p++ = DWORD(*pColor++);

		/*
			m_pd3dDevice->CreateTexture(512, 512, 1  // this won't work with LockRect(..)
                                 , D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8
                                 , D3DPOOL_DEFAULT, &m_pTexture );
		*/

		//IDirect3DTexture9* pTexture;

		HRESULT hr = g_pd3dDevice->CreateTexture(m_Size, m_Size, 1
                                 , 0, D3DFMT_A8R8G8B8
                                 , D3DPOOL_MANAGED, &m_Textures[layer], NULL );

		/*
		HRESULT hr = g_pd3dDevice->CreateTexture(m_Size, m_Size, 1
                                 , 0, D3DFMT_X8R8G8B8
                                 , D3DPOOL_MANAGED, &pTexture , NULL);
		*/

		/*
		DAMM!   A FEW HOURS OF MY LIFE WASTED!
		Well people, its always important to read the small print in docs,
		nearly 3am, and spent ages wondering why my LockRect(..) funciton wouldn't work,
		well its seems, you can't use 'D3DPOOL_DEFAULT' if your going to use LockRect(..)
		it will always fail.
		But then you'll also have problems if you try and use D3DUSAGE_RENDERTARGET mixed
		with other ones etc...worth looking into...bit of trial and error etc, but it
		works!
		*/

		IDirect3DTexture9* pTexture = m_Textures[layer];

		D3DLOCKED_RECT d3dlr;
		pTexture->LockRect(0,&d3dlr,NULL,0 );

		// pitch value should be h * w * 4 = 128 x 4 = 

		DWORD * pDst = (DWORD *)d3dlr.pBits;
		int DPitch = d3dlr.Pitch/4;
		DWORD * pSrc = (DWORD *)pixels;
		int SPitch = d3dlr.Pitch/4;	// bad assumption, but lets assume there the same pitch!
		
		for (int i=0; i<m_Size/*height*/; ++i)
			for (int j=0; j<m_Size/*width*/; ++j)
				pDst[i*DPitch + j] = pSrc[i*SPitch + j];				
		
		pTexture->UnlockRect (0);

		delete [] pixels;
	}

	delete [] data;

	return true;
}//End Create(..)


/*****************************************************************************/
/*                                                                           */
/* GetTexture(..)                                                            */
/*                                                                           */
/*****************************************************************************/
void XFurTexture::GetTexture(float layer, IDirect3DTexture9** pTexture) const
{
	int temp = (int)floor(m_NumLayers * max(0.0, min(0.9999, layer)));

	*pTexture = m_Textures[temp];
}//End GetTexture(..)

