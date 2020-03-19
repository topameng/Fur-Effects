/**********************************************************************************/
/*                                                                                */
/* File: MeshObject.h                                                             */
/* Author: bkenwright@xbdev.net                                                   */
/* URL: www.xbdev.net                                                             */
/* Date: 09-09-2004                                                               */
/* Desciption: Mesh Object Class                                                  */
/* Simple wrapper class taht allows you to load and render simple mesh objects    */
/* without worrying - flexible and simple to understand.                          */
/*                                                                                */
/* Updated: 09-09-2004 by bkenwright                                              */
/*                                                                                */
/**********************************************************************************/

#pragma once


#pragma comment(lib, "D3d9.lib")
#pragma comment(lib, "D3dx9.lib")
#include <d3dx9.h>


////////////////////////////////////////////////////////////////////////////////////
//                                                                                \\
// Name: CMeshObject class                                                        \\
// Date Created: 20-08-2004                                                       \\
// Description:  This is a really useful class, its can be used all over the      \\
// place - it is used to load in a class and any textures or materials that       \\
// would be used in conjunciton with it.  All self contained and easy to use :)   \\
// Use like this:                                                                 \\
// <1> CMeshObject mm;                                                            \\
// <2> mm.Create( &g_pd3dDevice, "coolmesh.x", "" );                              \\
// Call this next part in our main render loop                                    \\
// <3> mm.Render();                                                               \\
// When we've finished, just release it.                                          \\
// <4> mm.Release();                                                              \\
//                                                                                \\
////////////////////////////////////////////////////////////////////////////////////
class CMeshObject
{
protected:
	IDirect3DDevice9 * m_pd3dDevice;

	LPD3DXBUFFER m_pMtrlBuffer;
	DWORD        m_dwNumMaterials;
	LPD3DXMESH   m_mesh;

	D3DMATERIAL9* m_pMeshMaterials;
	LPDIRECT3DTEXTURE9* m_pMeshTextures;

public:

	const LPD3DXMESH GetMesh(){ return m_mesh; }

	HRESULT Create(IDirect3DDevice9** pDevice, const char * szFileName, const char * szFolder );
	HRESULT Release();

	void Render();

	void ConvertFVF( DWORD dwFVF );

};//End of CMeshObject class





