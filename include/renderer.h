#pragma once

#ifndef  _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // ! _CRT_SECURE_NO_WARNINGS

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif // ! _USE_MATH_DEFINES
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")


#include <cmath>
#include <stdio.h>
#include <vector>
#include <map>

using std::vector;
using std::map;
using std::pair;

#define DEF_COLOR(data, name) namespace Colors { static const D3DCOLOR name = data; }

DEF_COLOR(0xFF000000, Black);
DEF_COLOR(0xFFFFFFFF, White);

DEF_COLOR(0xFFFF0000, Red);
DEF_COLOR(0xFF00FF00, Green);
DEF_COLOR(0xFF0000FF, Blue);

DEF_COLOR(0xFFFFFF00, Yellow);
DEF_COLOR(0xFF00FFFF, SkyBlue);
DEF_COLOR(0xFFFF00FF, Pink);

//RDT - render draw type xddd

#define RDT_FILLED		0x1U
#define RDT_GRADIENT	0x2U
#define RDT_OUTLINED	0x2U
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
	void DrawString(int16_t x, int16_t y, D3DCOLOR color, ID3DXFont* font, bool outlined, bool centered, const char* text, ...);
	void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, D3DCOLOR color = 0);
	void DrawFilledBox(int16_t x, int16_t y, int16_t width, int16_t height, D3DCOLOR color = 0);
	//use DrawBox without thickness argument if thickness == 1
	void DrawBox(int16_t x, int16_t y, int16_t width, int16_t height, int16_t thickness = 2, D3DCOLOR color = 0);
	void DrawBox(int16_t x, int16_t y, int16_t width, int16_t height, D3DCOLOR color = 0);
	void DrawGradientBox(int16_t x, int16_t y, int16_t width, int16_t height, D3DCOLOR color1 = 0, D3DCOLOR color2 = 0, bool vertical = false);
	void DrawGradientBox(int16_t x, int16_t y, int16_t width, int16_t height, D3DCOLOR color1 = 0, D3DCOLOR color2 = 0, D3DCOLOR color3 = 0, D3DCOLOR color4 = 0);
	//use RDT_FILLED for filledcircle, RDT_GRADIENT for gradient circle and RDT_OUTLINED for outlined circle
	void DrawCircle(int16_t x, int16_t y, int16_t radius, uint16_t points = 32, uint8_t flags = 0, D3DCOLOR color1 = 0xFF, D3DCOLOR color2 = 0);
	void DrawCircleSector(int16_t x, int16_t y, int16_t radius, uint16_t points, uint16_t angle1, uint16_t angle2, D3DCOLOR color1, D3DCOLOR color2 = 0);
	void DrawRing(int16_t x, int16_t y, int16_t radius1, int16_t radius2, uint16_t points, uint8_t flags, D3DCOLOR color1, D3DCOLOR color2 = 0);
	void DrawRingSector(int16_t x, int16_t y, int16_t radius1, int16_t radius2, uint16_t points, uint16_t angle1, uint16_t angle2, D3DCOLOR color1, D3DCOLOR color2 = 0);
	void DrawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint8_t flags = 0, D3DCOLOR color1 = 0xFF, D3DCOLOR color2 = 0, D3DCOLOR color3 = 0);
	//frames per second
	int16_t GetFramerate() { return m_iFramerate; }
	//milliseconds
	void SetFramerateUpdateRate(uint16_t iUpdateRate) { m_iFramerateUpdateRate = iUpdateRate; }
private:
	IDirect3DStateBlock9* m_pStateBlock;

	struct SinCos_t { float flSin = 0.f, flCos = 0.f; };
	map<uint16_t, vector<SinCos_t>> m_SinCosContainer;

	SinCos_t* GetSinCos(uint16_t key)
	{
		if (!m_SinCosContainer.count(key))
		{
			vector<SinCos_t>temp_array(key + 1);

			uint16_t i = 0;
			for (float angle = 0.0; angle <= 2 * D3DX_PI; angle += (2 * D3DX_PI) / key)
				temp_array[i++] = SinCos_t{ sin(angle), cos(angle) };

			m_SinCosContainer.insert(std::pair<uint16_t, vector<SinCos_t>>(key, temp_array));
		}

		return &m_SinCosContainer[key][0];
	}

	uint16_t m_iFramerate = 0, m_iFramerateUpdateRate = 1000;

protected:
	struct Vertex_t
	{
		Vertex_t() { }

		Vertex_t(int _x, int _y, D3DCOLOR _color)
		{
			x = float(_x);
			y = float(_y);
			z = 0;
			rhw = 1;
			color = _color;
		}

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