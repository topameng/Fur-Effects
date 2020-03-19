/*****************************************************************************/
/*                                                                           */
/* File: FurMesh.cpp                                                         */
/*                                                                           */
/*****************************************************************************/

#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")

#include "FurMesh.h"

extern LPDIRECT3DDEVICE9       g_pd3dDevice; // defined at the top of main.cpp

struct FURVERTEX
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 tex1;
	D3DXVECTOR2 tex2;
//	D3DXVECTOR2 scale;
//	D3DXVECTOR2 radius;
//	D3DXVECTOR3 ds;
//	D3DXVECTOR3 dt;
};

UINT FVF_FURVERTEX_FORMAT  = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1;

LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration = NULL;


const D3DVERTEXELEMENT9 declaration[] = 
{
	{ 0, 0 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION, 0 },  // pos
	{ 0, 3 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },   // normal
	//{ 0, 6 * sizeof(float),D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
	{ 0, 6 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // tex1
	{ 0, 8 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 }, // tex2

//	{ 0, 10 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 }, // scale
//	{ 0, 12 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 }, // radius
//	{ 0, 14 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 }, // ds
//	{ 0, 16 * sizeof(float),D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5 }, // dt
	D3DDECL_END()
};

// partial derivatives of bivariate function

const float DELTA = 0.01;
inline D3DXVECTOR3 deriv_u(FurMesh::FUNC f, float u, float v)
{ return (f(u+DELTA, v) - f(u-DELTA, v)) / (2*DELTA); }
inline D3DXVECTOR3 deriv_v(FurMesh::FUNC f, float u, float v)
{ return (f(u, v+DELTA) - f(u, v-DELTA)) / (2*DELTA); }
inline D3DXVECTOR3 deriv_uu(FurMesh::FUNC f, float u, float v)
{ return (deriv_u(f, u+DELTA, v) - deriv_u(f, u-DELTA, v)) / (2*DELTA); }
inline D3DXVECTOR3 deriv_vv(FurMesh::FUNC f, float u, float v)
{ return (deriv_v(f, u, v+DELTA) - deriv_v(f, u, v-DELTA)) / (2*DELTA); }
inline D3DXVECTOR3 deriv_uv(FurMesh::FUNC f, float u, float v)
{ return (deriv_u(f, u, v+DELTA) - deriv_u(f, u, v-DELTA)) / (2*DELTA); }


/*****************************************************************************/
/*                                                                           */
/* FurMesh()                                                                 */
/* Class Default Constructor                                                 */
/*                                                                           */
/*****************************************************************************/
FurMesh::FurMesh()
{
	m_Name         = "";
	m_NumVertex    = 0;
	m_NumIndex     = 0;
	m_VertexBuffer = 0;
	m_IndexBuffer  = 0;
}//End FurMesh()


/*****************************************************************************/
/*                                                                           */
/* ~FurMesh()                                                                */
/* Class Default De-Constructor                                              */
/*                                                                           */
/*****************************************************************************/
FurMesh::~FurMesh()
{
	pVertexDeclaration->Release();
	m_VertexBuffer->Release();
	m_IndexBuffer->Release();
}//End ~FurMesh()



/*****************************************************************************/
/*                                                                           */
/* Create(..)                                                                */
/* The initialisation function - called once to set up variables and         */
/* allocate memory for vertex buffers etc                                    */
/*                                                                           */
/*****************************************************************************/
bool FurMesh::Create(const PARAM &param)
{
	m_Name = param.name;

	int num_u = param.num_u;
	int num_v = param.num_v;
	FUNC func = param.func;
	float scale_s = param.fur_scale_s;
	float scale_t = param.fur_scale_t;

	m_NumVertex = (num_u + 1) * (num_v + 1);
	m_NumIndex  = num_u * num_v * 6;

	// vertex buffer
	{
		int i, j;
		FURVERTEX *pVertices = new FURVERTEX[m_NumVertex];
		FURVERTEX *p = pVertices;

		for (j = 0; j <= num_v; j++)
		for (i = 0; i <= num_u; i++)
		{
			float u = float(i) / num_u;
			float v = float(j) / num_v;

			// vertex position
			p->pos = func(u, v);

			// texture coord for normal map
			p->tex1 = D3DXVECTOR2(u, v);

			// texture coord for fur texture
			p->tex2 = D3DXVECTOR2(scale_s * u, scale_t * v);

			// partial derivatives with respect to (u, v)
			D3DXVECTOR3& Pu  = deriv_u(func, u, v);
			D3DXVECTOR3& Pv  = deriv_v(func, u, v);
			D3DXVECTOR3& Puu = deriv_uu(func, u, v);
		//	D3DXVECTOR3& Puv = deriv_uv(func, u, v);
			D3DXVECTOR3& Pvv = deriv_vv(func, u, v);

			// basis vectors of local texture space
//			D3DXVec3Normalize(&p->ds, &Pu);
//			D3DXVec3Normalize(&p->dt, &Pv);
			D3DXVec3Normalize(&p->normal, D3DXVec3Cross(&p->normal, &Pu, &Pv));

			// scale factor
//			p->scale.x = scale_s / D3DXVec3Length(&Pu);
//			p->scale.y = scale_t / D3DXVec3Length(&Pv);

			// radii of curvature
			{
				// first and second fundamental forms
				// http://mathworld.wolfram.com/FundamentalForms.html
				float E = D3DXVec3LengthSq(&Pu);
			//	float F = D3DXVec3Dot(&Pu, &Pv);
				float G = D3DXVec3LengthSq(&Pv);
				float e = D3DXVec3Dot(&p->normal, &Puu);
			//	float f = D3DXVec3Dot(&p->normal, &Puv);
				float g = D3DXVec3Dot(&p->normal, &Pvv);

				// normal curvatures in the directions of u and v
				// http://mathworld.wolfram.com/NormalCurvature.html
				float curvature_u = -e / E;
				float curvature_v = -g / G;

				// radii of curvature corresponding to the normal curvatures
				// http://mathworld.wolfram.com/RadiusofCurvature.html
				if (curvature_u < 0.001) curvature_u = 0.001;
//				p->radius.x = 1.0 / curvature_u;
				if (curvature_v < 0.001) curvature_v = 0.001;
//				p->radius.y = 1.0 / curvature_v;

				// to be accurate, principal curvatures should be taken into account...
				// http://mathworld.wolfram.com/PrincipalCurvatures.html
			}

			p++;
		}

		g_pd3dDevice->CreateVertexBuffer( m_NumVertex*sizeof(FURVERTEX), 0, FVF_FURVERTEX_FORMAT, 
                                                                   D3DPOOL_MANAGED, &m_VertexBuffer, NULL);
		
		void * pdata;
		m_VertexBuffer->Lock( 0, m_NumVertex*sizeof(FURVERTEX), (void**)&pdata, 0 );
		memcpy( pdata, pVertices, m_NumVertex*sizeof(FURVERTEX) );
		m_VertexBuffer->Unlock();

		delete [] pVertices;

	}

	// index buffer (triangle list)
	{
		int i, j, k;
		WORD *pIndices = new WORD[m_NumIndex];
		WORD *p = pIndices;

		for (j = 0; j < num_v; j++)
		{
			k = j * (num_u + 1);
			for (i = 0; i < num_u; i++, k++)
			{
				*p++ = k;
				*p++ = k + 1;
				*p++ = k + (num_u + 1);
				*p++ = k + (num_u + 1) + 1;
				*p++ = k + (num_u + 1);
				*p++ = k + 1;
			}
		}

		HRESULT hr = g_pd3dDevice->CreateIndexBuffer( m_NumIndex * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, 
                                                                   D3DPOOL_DEFAULT, &m_IndexBuffer, NULL); 

		void * pdata;
		m_IndexBuffer->Lock(0, m_NumIndex*sizeof(WORD), &pdata, 0);
		memcpy(pdata, pIndices,m_NumIndex*sizeof(WORD));
		m_IndexBuffer->Unlock();

		delete [] pIndices;
	}

	HRESULT hr = g_pd3dDevice->CreateVertexDeclaration( declaration, &pVertexDeclaration );

	return true; // Return true - all okay!
}//End Create(..)



/*****************************************************************************/
/*                                                                           */
/* Render()                                                                  */
/* As the name says, its renders our mesh to the screen...                   */
/*                                                                           */
/*****************************************************************************/
void FurMesh::Render() const
{
	g_pd3dDevice->SetVertexDeclaration( pVertexDeclaration );

	g_pd3dDevice->SetStreamSource(0, m_VertexBuffer, 0, sizeof(FURVERTEX));

	g_pd3dDevice->SetIndices( m_IndexBuffer ); 

	  //  The ending point in the index buffer (e.g. which triangle)------
      //  The starting point in the index buffer-----------------------   |
      //                                                               |  |
      //  The end in the vertex buffer (e.g. how many points)---       |  |
      //  Starting vertex in the vertex buffer (e.g. x,y,z)--   |      |  |
      //                                                     |  |      |  |
      //                                                    \ /\ /    \ /\ /
      //                                                     |  |      |  |           
      //g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_NumVertex,     0, m_NumIndex);
	//g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_NumVertex,     0, 2160/4 +50);
	//g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_NumVertex,     0, 2158); // ?????????????/
	//g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLES, 0, 0, m_NumVertex,     0, 2158); // ?????????????/


	g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_NumVertex,     0, 2158/3 + 1);


}//End Render(..)
