#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#include <stdio.h>
#include <vector>
using std::vector;

#define DEF_COLOR(data, name) namespace Color { static const D3DCOLOR name = data; }

DEF_COLOR(0xFF000000, Black);
DEF_COLOR(0xFFFFFFFF, White);

DEF_COLOR(0xFFFF0000, Red);
DEF_COLOR(0xFF00FF00, Green);
DEF_COLOR(0xFF0000FF, Blue);

DEF_COLOR(0xFFFFFF00, Yellow);
DEF_COLOR(0xFF00FFFF, SkyBlue);
DEF_COLOR(0xFFFF00FF, Purple);

class cRender 
{
public:
	cRender(IDirect3DDevice9* device);
	~cRender();

	void BeginDraw();
	void EndDraw();

	void PushRenderState(const D3DRENDERSTATETYPE dwState, DWORD dwValue);

	void OnLostDevice();
	void OnResetDevice();

	bool AddFont(ID3DXFont** pFont, const char* szName, byte iSize = 14, bool bAntiAliased = false);

	void DrawString(short x, short y, D3DCOLOR color, ID3DXFont* font, bool outlined, bool centered, const char* text, ...);
	void DrawLine(short x1, short y1, short x2, short y2, D3DCOLOR color = 0);
	void DrawFilledBox(short x, short y, short width, short height, D3DCOLOR color = 0);
	//use DrawBox without thickness argument if thickness == 1
	void DrawBox(short x, short y, short width, short height, short thickness, D3DCOLOR color = 0);
	void DrawBox(short x, short y, short width, short height, D3DCOLOR color = 0);
	void DrawGradientBox(short x, short y, short width, short height, D3DCOLOR color1 = 0, D3DCOLOR color2 = 0, bool vertical = false);
	void DrawGradientBox(short x, short y, short width, short height, D3DCOLOR color1 = 0, D3DCOLOR color2 = 0, D3DCOLOR color3 = 0, D3DCOLOR color4 = 0);
	//points should be divideable by 4, >= 8 && <= 32
	void DrawCircle(short x, short y, short radius, byte points, D3DCOLOR color = 0, bool filled = false);
	//points should be divideable by 4, >= 8 && <= 32
	void DrawGradientCircle(short x, short y, short radius, byte points, D3DCOLOR color1 = 0, D3DCOLOR color2 = 0);
	void DrawTriangle(short x1, short y1, short x2, short y2, short x3, short y3, D3DCOLOR color = 0, bool filled = false);
	void DrawGradientTriangle(short x1, short y1, short x2, short y2, short x3, short y3, D3DCOLOR color1 = 0, D3DCOLOR color2 = 0, D3DCOLOR color3 = 0);
	
	//frames per second
	short GetFramerate() { return m_iFramerate; }
	//milliseconds
	void SetFramerateUpdateRate(short iUpdateRate) {	m_iFramerateUpdateRate = iUpdateRate; }
private:
	struct cossin_t { float flCos, flSin; };
	cossin_t m_SinCosTable[7][34];

	short m_iFramerate = 0U, m_iFramerateUpdateRate = 1000U;

protected:
	struct Vertex_t
	{
		float x, y, z, rhw;
		D3DCOLOR color;
	};

	struct Font_t
	{
		IDirect3DDevice9* pDevice;
		ID3DXFont** pFont;
		char szName[64];
		byte iSize;
		bool bAntiAliased;

		bool update()
		{
			return D3DXCreateFontA(pDevice, iSize, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
				bAntiAliased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY, DEFAULT_PITCH, szName, pFont) == D3D_OK;
		};
	};

	IDirect3DDevice9* m_pDevice;
	vector<Font_t>m_FontsList;
	
	struct RenderState_t
	{
		D3DRENDERSTATETYPE dwState;
		DWORD dwValue;
	};

	vector<RenderState_t>m_RenderStates;

	void CreateVertex(short x, short y, D3DCOLOR color, Vertex_t* pVertex);
};