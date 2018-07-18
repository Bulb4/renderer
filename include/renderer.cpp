#include "renderer.h"

cRender::cRender(IDirect3DDevice9* device)
{
	m_pDevice = device;

	for (byte n = 0; n <= 6; n++)
		for (byte i = 0; i < 34; i++)
		{
			const float temp = D3DX_PI * (i / ((n * 4 + 8)/ 2.f));
			m_SinCosTable[n][i].flCos = cos(temp);
			m_SinCosTable[n][i].flSin = sin(temp);
		}
}

cRender::~cRender()
{
	m_pDevice = nullptr;
}

void cRender::BeginDraw()
{
	PushRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	PushRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	PushRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	PushRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PushRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	PushRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
}

void cRender::EndDraw()
{
	//pop render states
	for (auto a : m_RenderStates)
		m_pDevice->SetRenderState(a.dwState, a.dwValue);

	m_RenderStates.clear();

	//framerate calculator
	const DWORD dwCurrentTime = GetTickCount();
	static DWORD dwLastUpdateTime = 0, dwElapsedTime = 0;

	dwElapsedTime = dwCurrentTime - dwLastUpdateTime;

	static uint16_t iFrames = 0;
	iFrames++;

	if (dwElapsedTime >= m_iFramerateUpdateRate)
	{
		m_iFramerate = uint16_t(iFrames * 1000.f / dwElapsedTime);
		iFrames = 0;
		dwLastUpdateTime = dwCurrentTime;
	}
}

void cRender::OnLostDevice()
{
	for (auto a : m_FontsList)
		if (*a.pFont)
			(*a.pFont)->OnLostDevice();
}

void cRender::OnResetDevice()
{
	for (auto a : m_FontsList)
		if (*a.pFont)
		{
			(*a.pFont)->OnResetDevice();
			a.update();
		}
}

bool cRender::AddFont(ID3DXFont** pFont, const char* szName, byte iSize, bool bAntiAliased)
{
	Font_t font;
	
	font.pDevice = m_pDevice;
	font.pFont = pFont;
	font.iSize = iSize;
	font.bAntiAliased = bAntiAliased;
	sprintf_s(font.szName, szName);
	
	if (!font.update())
		return false;

	m_FontsList.push_back(font);

	return true;
}

inline void cRender::CreateVertex(uint16_t x, uint16_t y, D3DCOLOR color, Vertex_t* pVertex)
{
	pVertex->x = (float)x;
	pVertex->y = (float)y;
	pVertex->z = 0.f;
	pVertex->rhw = 1.f;
	pVertex->color = color;
}

void cRender::PushRenderState(const D3DRENDERSTATETYPE dwState, DWORD dwValue)
{
	DWORD dwTempValue;
	m_pDevice->GetRenderState(dwState, &dwTempValue);
	m_RenderStates.push_back({ dwState, dwTempValue });
	m_pDevice->SetRenderState(dwState, dwValue);
}

void cRender::DrawString(uint16_t x, uint16_t y, D3DCOLOR color, ID3DXFont* font, bool outlined, bool centered, const char* text, ...)
{
	va_list args;
	char buf[128];
	va_start(args, text);
	vsprintf_s(buf, text, args);
	va_end(args);

	const size_t size = strlen(buf);

	const DWORD dwFlags = centered ? DT_CENTER : DT_NOCLIP;

	RECT pRect;
	
	if (outlined) 
	{
		pRect.left = x - 1;
		pRect.top = y;
		font->DrawTextA(NULL, buf, size, &pRect, dwFlags, Color::Black);
		pRect.left = x + 1;
		pRect.top = y;
		font->DrawTextA(NULL, buf, size, &pRect, dwFlags, Color::Black);
		pRect.left = x;
		pRect.top = y - 1;
		font->DrawTextA(NULL, buf, size, &pRect, dwFlags, Color::Black);
		pRect.left = x;
		pRect.top = y + 1;
		font->DrawTextA(NULL, buf, size, &pRect, dwFlags, Color::Black);
	}

	pRect.left = x;
	pRect.top = y;

	font->DrawTextA(NULL, buf, size, &pRect, dwFlags, color);
}

void cRender::DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, D3DCOLOR color) 
{
	Vertex_t pVertex[2];
	CreateVertex(x1, y1, color, &pVertex[0]);
	CreateVertex(x2, y2, color, &pVertex[1]);

	m_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, pVertex, sizeof(Vertex_t));
}

void cRender::DrawFilledBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, D3DCOLOR color) 
{
	Vertex_t pVertex[4];
	CreateVertex(x, y, color, &pVertex[0]);
	CreateVertex(x + width, y, color, &pVertex[1]);
	CreateVertex(x, y + height, color, &pVertex[2]);
	CreateVertex(x + width, y + height, color, &pVertex[3]);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness, D3DCOLOR color)
{
	DrawFilledBox(x, y, width, thickness, color);
	DrawFilledBox(x, y, thickness, height, color);
	DrawFilledBox(x + width - thickness, y, thickness, height, color);
	DrawFilledBox(x, y + height - thickness, width, thickness, color);
}

void cRender::DrawBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, D3DCOLOR color)
{
	Vertex_t pVertex[5];
	CreateVertex(x, y, color, &pVertex[0]);
	CreateVertex(x + width, y, color, &pVertex[1]);
	CreateVertex(x, y + height, color, &pVertex[3]);
	CreateVertex(x + width, y + height, color, &pVertex[2]);
	CreateVertex(x, y, color, &pVertex[4]);

	m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, pVertex, sizeof(Vertex_t));
}

void cRender::DrawGradientBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, D3DCOLOR color1, D3DCOLOR color2, bool vertical) 
{
	Vertex_t pVertex[4];
	CreateVertex(x, y, color1, &pVertex[0]);
	CreateVertex(x + width, y, vertical ? color1 : color2, &pVertex[1]);
	CreateVertex(x, y + height, vertical ? color2 : color1, &pVertex[2]);
	CreateVertex(x + width, y + height, color2, &pVertex[3]);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawGradientBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3, D3DCOLOR color4)
{
	Vertex_t pVertex[4];
	CreateVertex(x, y, color1, &pVertex[0]);
	CreateVertex(x + width, y, color2, &pVertex[1]);
	CreateVertex(x, y + height, color3, &pVertex[2]);
	CreateVertex(x + width, y + height, color4, &pVertex[3]);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawCircle(uint16_t x, uint16_t y, uint16_t radius, byte points, D3DCOLOR color, bool filled)
{
	Vertex_t* pVertex = new Vertex_t[points + 1];

	for (byte i = 0; i <= points; i++)
	{
		cossin_t& sincos = m_SinCosTable[(points - 8) / 4][i];
		CreateVertex(x + radius * sincos.flCos, y + (filled ? 1 : -1) * radius * sincos.flSin, color, &pVertex[i]);
	}

	m_pDevice->DrawPrimitiveUP(filled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP, points, pVertex, sizeof(Vertex_t));
	delete[] pVertex;
}

void cRender::DrawGradientCircle(uint16_t x, uint16_t y, uint16_t radius, byte points, D3DCOLOR color1, D3DCOLOR color2)
{
	Vertex_t* pVertex = new Vertex_t[points + 2];

	CreateVertex(x, y, color2, pVertex);

	for (byte i = 1; i <= points + 1; i++)
	{
		cossin_t& sincos = m_SinCosTable[(points - 8) / 4][i];
		CreateVertex(x + radius * sincos.flCos, y - radius * sincos.flSin, color1, &pVertex[i]);
	}

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, points, pVertex, sizeof(Vertex_t));
	delete[] pVertex;
}

void cRender::DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, D3DCOLOR color, bool filled)
{
	Vertex_t pVertex[4];
	CreateVertex(x1, y1, color, &pVertex[0]);
	CreateVertex(x2, y2, color, &pVertex[2]);
	CreateVertex(x3, y3, color, &pVertex[1]);
	CreateVertex(x1, y1, color, &pVertex[3]);

	m_pDevice->DrawPrimitiveUP(filled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP, 3, pVertex, sizeof(Vertex_t));
}

void cRender::DrawGradientTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3)
{
	Vertex_t pVertex[4];
	CreateVertex(x1, y1, color1, &pVertex[0]);
	CreateVertex(x2, y2, color2, &pVertex[2]);
	CreateVertex(x3, y3, color3, &pVertex[1]);
	CreateVertex(x1, y1, color1, &pVertex[3]);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 3, pVertex, sizeof(Vertex_t));
}