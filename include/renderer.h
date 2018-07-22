#pragma once


#define _CRT_SECURE_NO_WARNINGS

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdio.h>
#include <vector>
#include <map>
using std::vector;
using std::map;
using std::pair;

#define DEF_COLOR(data, name) namespace Color { static const D3DCOLOR name = data; }

DEF_COLOR(0xFF000000, Black);
DEF_COLOR(0xFFFFFFFF, White);

DEF_COLOR(0xFFFF0000, Red);
DEF_COLOR(0xFF00FF00, Green);
DEF_COLOR(0xFF0000FF, Blue);

DEF_COLOR(0xFFFFFF00, Yellow);
DEF_COLOR(0xFF00FFFF, SkyBlue);
DEF_COLOR(0xFFFF00FF, Purple);

#define CIRCLE_FILLED           '\1'
#define CIRCLE_GRADIENT         '\2'
/*
#define CIRCLE_CUSTOM_RADIUS    '\4'//default: 0
#define CIRCLE_CUSTOM_SECTOR    '\8'//default: 0 - 360
*/

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

	bool AddFont(ID3DXFont** pFont, const char* szName, uint8_t iSize = 14, bool bAntiAliased = false);
	//if outlined function become 5 times slower
	void DrawString(short x, short y, D3DCOLOR color, ID3DXFont* font, bool outlined, bool centered, const char* text, ...);
	void DrawLine(short x1, short y1, short x2, short y2, D3DCOLOR color = 0);
	void DrawFilledBox(short x, short y, short width, short height, D3DCOLOR color = 0);
	//use DrawBox without thickness argument if thickness == 1
	void DrawBox(short x, short y, short width, short height, short thickness = 2, D3DCOLOR color = 0);
	void DrawBox(short x, short y, short width, short height, D3DCOLOR color = 0);
	void DrawGradientBox(short x, short y, short width, short height, D3DCOLOR color1 = 0, D3DCOLOR color2 = 0, bool vertical = false);
	void DrawGradientBox(short x, short y, short width, short height, D3DCOLOR color1 = 0, D3DCOLOR color2 = 0, D3DCOLOR color3 = 0, D3DCOLOR color4 = 0);
	//use CIRCLE_FILLED for filledcircle, CIRCLE_GRADIENT for gradient circle and 0 for outlined circle
	void DrawCircle(short x, short y, short radius, uint8_t points = 32, uint8_t flags = 0, D3DCOLOR color1 = 0xFF, D3DCOLOR color2 = 0);
	void DrawTriangle(short x1, short y1, short x2, short y2, short x3, short y3, D3DCOLOR color = 0, bool filled = false);
	void DrawGradientTriangle(short x1, short y1, short x2, short y2, short x3, short y3, D3DCOLOR color1 = 0xFF, D3DCOLOR color2 = 0, D3DCOLOR color3 = 0);
	//frames per second
	short GetFramerate() { return m_iFramerate; }
	//milliseconds
	void SetFramerateUpdateRate(short iUpdateRate) { m_iFramerateUpdateRate = iUpdateRate; }
private:
	struct SinCos_t { float flSin = 0.f, flCos = 0.f; };
	map<uint8_t, vector<SinCos_t>> m_SinCosContainer;

	SinCos_t* GetSinCos(uint8_t key)
	{
		if (!m_SinCosContainer.count(key))
		{
			vector<SinCos_t>temp_array(key);

			uint8_t i = 0;
			for (float angle = 0.0; angle <= 2 * D3DX_PI; angle += (2 * D3DX_PI) / key)
				temp_array[i++] = SinCos_t{ sin(angle), cos(angle) };

			m_SinCosContainer.insert(std::pair<uint8_t, vector<SinCos_t>>(key, temp_array));
		}

		return &m_SinCosContainer[key][0];
	}

	short m_iFramerate = 0, m_iFramerateUpdateRate = 1000;

protected:
	struct Vertex_t
	{
		Vertex_t() { }

		Vertex_t(float _x, float _y, D3DCOLOR _color)
		{
			x = _x;
			y = _y;
			z = 0;
			rhw = 1;
			color = _color;
		}

		float x, y, z, rhw;
		D3DCOLOR color;
	};

	struct Font_t
	{
		IDirect3DDevice9* pDevice;
		ID3DXFont** pFont;
		char szName[64];
		uint8_t iSize;
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
};