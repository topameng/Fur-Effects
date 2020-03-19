/*****************************************************************************/
/*                                                                           */
/* File: XFurTexture.h                                                       */
/*                                                                           */
/*****************************************************************************/

#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#define MAX_TEXTURES 30 

class XFurTexture
{
public:
	XFurTexture();
	~XFurTexture();
	bool Create(UINT seed, UINT size, UINT num, UINT density);
	void Release();

	void GetTexture(float layer, IDirect3DTexture9** pTexture) const;

private:
	UINT m_Size;
	UINT m_NumLayers;

	IDirect3DTexture9* m_Textures[MAX_TEXTURES];

};//End FurTexture Class
