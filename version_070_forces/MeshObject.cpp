/**********************************************************************************/
/*                                                                                */
/* File: MeshObject.cpp                                                           */
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


#include "MeshObject.h"

// # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX
// # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX


const D3DVERTEXELEMENT9 declaration[] = 
{
	{ 0, 0 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION, 0 },  // pos
	{ 0, 3 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },   // normal
	{ 0, 6 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // tex1
	D3DDECL_END()
};


// # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX
// # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX


////////////////////////////////////////////////////////////////////////////////////
//                                                                                \\
// Name: CMeshObject::Render()                                                    \\
// Date Created: 20-08-2004                                                       \\
// Description:  It takes our m_mesh object and renders it to our device screen.  \\
// It also applies any texturing or materials that where loaded with it on        \\
// creation.                                                                      \\
// Note: We don't apply any world matrix or anything like that to move it around  \\
//                                                                                \\
////////////////////////////////////////////////////////////////////////////////////
void CMeshObject::Render()
{
	LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration = NULL;
	HRESULT hr = m_pd3dDevice->CreateVertexDeclaration( declaration, &pVertexDeclaration );
	m_pd3dDevice->SetVertexDeclaration( pVertexDeclaration );

	// Actually drawing something here!
	for(DWORD i = 0; i < m_dwNumMaterials; i++)
	{
			//m_pd3dDevice->SetMaterial(&m_pMeshMaterials[i]);
			//m_pd3dDevice->SetTexture(0, m_pMeshTextures[i]);
	    
			m_mesh->DrawSubset(i);
	}

	pVertexDeclaration->Release();

}//End Render(..)


////////////////////////////////////////////////////////////////////////////////////
//                                                                                \\
// Name: CMeshObject::Release()                                                   \\
// Date Created: 20-08-2004                                                       \\
// Description:  Its name says it all!  Release any resources that where loaded   \\
// in when the class was created.  Which is the mesh data, materials and textures \\
//                                                                                \\
////////////////////////////////////////////////////////////////////////////////////
HRESULT CMeshObject::Release()
{
	m_pMtrlBuffer->Release();

	// Tidy up!
	for(DWORD i = 0; i < m_dwNumMaterials; i++)
	{
		if(m_pMeshTextures[i] != NULL)
			m_pMeshTextures[i]->Release();
	}
	delete[] m_pMeshMaterials;
	delete[] m_pMeshTextures;
	
	m_mesh->Release();

	return S_OK;
}//End Create(..)


////////////////////////////////////////////////////////////////////////////////////
//                                                                                \\
// Name: CMeshObject::Create(..)                                                  \\
// Date Created: 20-08-2004                                                       \\
// Description:  We call Create(..) at the beginning, as it does the all the      \\
// main work.  Which is we pass the files location, and the location of any       \\
// texture files.  If there with the executable file we just use "" for this.     \\
// We also pass it the Device Screen which it keeps a reference copy of.          \\
//                                                                                \\
////////////////////////////////////////////////////////////////////////////////////
HRESULT CMeshObject::Create(IDirect3DDevice9** pDevice, const char * szFileName, 
						                                const char * szFolder )
{
	HRESULT hr = 0;

	m_pd3dDevice = *pDevice;

		    // File Name ---------+
		    //                    |
		    //                   \|/
		    //                    |
	hr = D3DXLoadMeshFromX( (char*)szFileName, D3DXMESH_MANAGED, m_pd3dDevice,
						NULL, &m_pMtrlBuffer, NULL,/*NULL - dx9 ,*/ &m_dwNumMaterials, &m_mesh);

	if( hr != S_OK ) return S_FALSE;

	//Create two arrays. One to hold the materials and only to hold the textures
	m_pMeshMaterials = new D3DMATERIAL9[m_dwNumMaterials];
	m_pMeshTextures  = new LPDIRECT3DTEXTURE9[m_dwNumMaterials];
	D3DXMATERIAL* matMaterials = (D3DXMATERIAL*)m_pMtrlBuffer->GetBufferPointer();
	// Loads of allocation etc here!
	for(DWORD i = 0; i < m_dwNumMaterials; i++)
	{
		//Copy the material
		m_pMeshMaterials[i] = matMaterials[i].MatD3D;
		//Set the ambient color for the material (D3DX does not do this)
		m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;

		// Add the location of the texture onto the texture filename
		char szTextureFile[255];
		strcpy(szTextureFile, szFolder);
		if( matMaterials[i].pTextureFilename != 0 )
			strcat(szTextureFile, matMaterials[i].pTextureFilename);

		//Create the texture
		if(FAILED(D3DXCreateTextureFromFile(m_pd3dDevice, 
											szTextureFile, 
											&m_pMeshTextures[i])))
		{
			m_pMeshTextures[i] = NULL;
		};
	}//End for loop(..)


	// # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX
	// # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX # HACK FIX
	// Just for this demo - where using a pre-done format!  Hence the added line!
	ConvertFVF(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1);

	return S_OK;
}// End Create(...)


////////////////////////////////////////////////////////////////////////////////////
//                                                                                \\
// Name: CMeshObject::ConvertFVF(..)                                              \\
// Date Created: 20-08-2004                                                       \\
// Description:  With certain demos, we sometimes need to make sure that our mesh \\
// is in the form that we want.  As we assume that it will have a normal?  Or it  \\
// wont... or if its just vertex and texture information.  Well we can force      \\
// our mesh data into a format that we want here.  It creates a copy of the mesh  \\
// and modifies it to the Flexible Vertex Format (FVF) that we want :)            \\
//                                                                                \\
////////////////////////////////////////////////////////////////////////////////////

void CMeshObject::ConvertFVF( DWORD dvFVF )
{
	// Simple error checking
	if( m_mesh == NULL ) return;

	DWORD oldFVF = m_mesh->GetFVF();

	LPD3DXMESH pMesh;
	//Make sure that the normals are setup for our mesh
    m_mesh->CloneMeshFVF(D3DXMESH_MANAGED, dvFVF, m_pd3dDevice, &pMesh);

	m_mesh->Release();

	m_mesh = pMesh;

	// If our old vertex data didn't have normals, and our new FVF format
	// has them, then we'll generate them here.
	if( !(oldFVF & D3DFVF_NORMAL) && 
		( dvFVF & D3DFVF_NORMAL ) )
	{
		 D3DXComputeNormals(m_mesh, NULL /* , NULL - dx9*/);
	}
}//End ConvertFVF(..)

