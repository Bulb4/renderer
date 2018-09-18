#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // ! _CRT_SECURE_NO_WARNINGS

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif // ! _USE_MATH_DEFINES

#include <d3d9.h>
#include <d3dx9.h>
#include <Windows.h>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <map>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

using std::vector;
using std::map;
using std::pair;

#define RELEASE_INTERFACE(pInterface) if (pInterface) { pInterface->Release(); pInterface = nullptr; }

#include "color.hpp"
#include "font.hpp"


enum RenderDrawType : uint32_t
{
	RenderDrawType_None = 0,
	RenderDrawType_Outlined = 1 << 0,
	RenderDrawType_Filled = 1 << 1,
	RenderDrawType_Gradient = 1 << 2,
	RenderDrawType_OutlinedGradient = RenderDrawType_Outlined | RenderDrawType_Gradient,
	RenderDrawType_FilledGradient = RenderDrawType_Filled | RenderDrawType_Gradient
};


class cRender
{
public:
	cRender(IDirect3DDevice9* device, bool bUseDynamicSinCos = false);
	~cRender();

	void BeginDraw();
	void EndDraw();

	void OnLostDevice();
	void OnResetDevice();

	inline void PushRenderState(const D3DRENDERSTATETYPE dwState, DWORD dwValue);

	//if outlined function become 5 times slower
	void DrawString(int16_t x, int16_t y, color_t color, cFont* font, bool outlined, bool centered, const char* text, ...);
	void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, color_t color);
	void DrawFilledBox(int16_t x, int16_t y, int16_t width, int16_t height, color_t color);
	//use DrawBox without thickness argument if thickness == 1
	void DrawBox(int16_t x, int16_t y, int16_t width, int16_t height, int16_t thickness, color_t color);
	void DrawBox(int16_t x, int16_t y, int16_t width, int16_t height, color_t color);
	void DrawGradientBox(int16_t x, int16_t y, int16_t width, int16_t height, color_t color1, color_t color2, color_t color3, color_t color4);
	//use RenderDrawType_Filled for filledcircle, RenderDrawType_Gradient for gradient circle and RenderDrawType_Outlined for outlined circle
	void DrawCircle(int16_t x, int16_t y, int16_t radius, uint16_t points, RenderDrawType flags, color_t color1, color_t color2);
	void DrawCircleSector(int16_t x, int16_t y, int16_t radius, uint16_t points, uint16_t angle1, uint16_t angle2, color_t color1, color_t color2);
	void DrawRing(int16_t x, int16_t y, int16_t radius1, int16_t radius2, uint16_t points, RenderDrawType flags, color_t color1, color_t color2);
	void DrawRingSector(int16_t x, int16_t y, int16_t radius1, int16_t radius2, uint16_t points, uint16_t angle1, uint16_t angle2, color_t color1, color_t color2);
	void DrawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, RenderDrawType flags, color_t color1, color_t color2, color_t color3);

	//frames per second
	int16_t GetFramerate() const { return m_iFramerate; }
	//milliseconds
	void SetFramerateUpdateRate(const uint16_t iUpdateRate) { m_iFramerateUpdateRate = iUpdateRate; }

private:
	struct SinCos_t { float flSin = 0.f, flCos = 0.f; };
	map<uint16_t, SinCos_t*> m_SinCosContainer;
	//we dont need to calculate sin and cos every frame, we just calculate it one time
	SinCos_t* GetSinCos(uint16_t key)
	{
		if (!m_SinCosContainer.count(key))
		{
			SinCos_t* temp_array = new SinCos_t[key + 1];

			uint16_t i = 0;
			for (float angle = 0.0; angle <= 2 * D3DX_PI; angle += (2 * D3DX_PI) / key)
				temp_array[i++] = SinCos_t{ sin(angle), cos(angle) };

			m_SinCosContainer.insert(pair<uint16_t, SinCos_t*>(key, temp_array));
		}

		return m_SinCosContainer[key];
	}

	bool m_bUseDynamicSinCos;

	uint16_t m_iFramerate = 0, m_iFramerateUpdateRate = 1000;
protected:
	struct Vertex_t
	{
		Vertex_t() { }

		Vertex_t(int _x, int _y, color_t _color)
		{
			x = static_cast<float>(_x);
			y = static_cast<float>(_y);
			z = 0;
			rhw = 1;
			color = _color.color;
		}

		Vertex_t(float _x, float _y, color_t _color)
		{
			x = _x;
			y = _y;
			z = 0;
			rhw = 1;
			color = _color.color;
		}

		float x, y, z, rhw;
		color_t color = 0;
	};

	IDirect3DDevice9* m_pDevice;
	
	struct RenderState_t
	{
		D3DRENDERSTATETYPE dwState;
		DWORD dwValue;
	};

	vector<RenderState_t>m_RenderStates;
};