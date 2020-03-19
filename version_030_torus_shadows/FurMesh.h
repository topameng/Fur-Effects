/*****************************************************************************/
/*                                                                           */
/* File: FurMesh.h                                                           */
/*                                                                           */
/*****************************************************************************/

#pragma once


#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")


class FurMesh
{
public:
	typedef D3DXVECTOR3 (*FUNC)(float, float);
	struct PARAM
	{
		FUNC func;
		int num_u, num_v;
		float fur_scale_s, fur_scale_t;
		const char *name;
	};

	FurMesh();
	~FurMesh();
	bool Create(const PARAM &param);
	void Render() const;

private:
	const char *m_Name;
	UINT m_NumVertex;
	UINT m_NumIndex;
	IDirect3DVertexBuffer9  * m_VertexBuffer;
	IDirect3DIndexBuffer9   * m_IndexBuffer;

};//End FurMesh(..) class


/*****************************************************************************/

static D3DXVECTOR3 torus(float u, float v)
{
	u *= 2*D3DX_PI;
	v *= 2*D3DX_PI;
	D3DXVECTOR3 vec;
	vec.x = sinf(u) * (2.1f + sinf(v));
	vec.y = cosf(u) * (2.1f + sinf(v));
	vec.z = cosf(v);
	return vec;
}//End torus(..)

/*****************************************************************************/

static const FurMesh::PARAM MeshParams[] =
{
	{ torus,   24, 15,   6, 3,  "Torus" },
};
