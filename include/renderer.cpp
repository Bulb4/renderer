#include "renderer.h"

cRender::cRender(IDirect3DDevice9* device)
{
	m_pDevice = device;
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

	m_pDevice->SetTexture(0, NULL);
	m_pDevice->SetPixelShader(NULL);
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

	static short iFrames = 0;
	iFrames++;

	if (dwElapsedTime >= m_iFramerateUpdateRate)
	{
		m_iFramerate = short(iFrames * 1000.f / dwElapsedTime);
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

bool cRender::AddFont(ID3DXFont** pFont, const char* szName, uint8_t iSize, bool bAntiAliased)
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

void cRender::PushRenderState(const D3DRENDERSTATETYPE dwState, DWORD dwValue)
{
	DWORD dwTempValue;
	m_pDevice->GetRenderState(dwState, &dwTempValue);
	m_RenderStates.push_back({ dwState, dwTempValue });
	m_pDevice->SetRenderState(dwState, dwValue);
}

void cRender::DrawString(short x, short y, D3DCOLOR color, ID3DXFont* font, bool outlined, bool centered, const char* text, ...)
{
	va_list args;
	char buf[256];
	va_start(args, text);
	vsprintf_s(buf, text, args);
	va_end(args);

	const size_t size = strlen(buf);

	RECT rect = { x, y };
	
	if (centered)
	{
		RECT size_rect = { 0 };
		font->DrawTextA(NULL, buf, size, &size_rect, DT_CALCRECT, 0);

		rect.left -= size_rect.right / 2;
		rect.top -= size_rect.bottom / 2;
	}

	if (outlined) 
	{
		rect.top++;
		font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, Color::Black);//x; y + 1
		rect.left++; rect.top--;
		font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, Color::Black);//x + 1; y
		rect.left--; rect.top--;
		font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, Color::Black);//x; y - 1
		rect.left--; rect.top++;
		font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, Color::Black);//x - 1; y
		rect.left++;
	}

	font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, color);
}

void cRender::DrawLine(short x1, short y1, short x2, short y2, D3DCOLOR color) 
{
	Vertex_t pVertex[2];
	pVertex[0] = Vertex_t(x1, y1, color);
	pVertex[1] = Vertex_t(x2, y2, color);

	m_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, pVertex, sizeof(Vertex_t));
}

void cRender::DrawFilledBox(short x, short y, short width, short height, D3DCOLOR color) 
{
	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x, y, color);
	pVertex[1] = Vertex_t(x + width, y, color);
	pVertex[2] = Vertex_t(x, y + height, color);
	pVertex[3] = Vertex_t(x + width, y + height, color);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawBox(short x, short y, short width, short height, short thickness, D3DCOLOR color)
{
	DrawFilledBox(x, y, width, thickness, color);
	DrawFilledBox(x, y, thickness, height, color);
	DrawFilledBox(x + width - thickness, y, thickness, height, color);
	DrawFilledBox(x, y + height - thickness, width, thickness, color);
}

void cRender::DrawBox(short x, short y, short width, short height, D3DCOLOR color)
{
	Vertex_t pVertex[5];

	pVertex[0] = Vertex_t(x, y, color);
	pVertex[1] = Vertex_t(x + width, y, color);
	pVertex[2] = Vertex_t(x + width, y + height, color);
	pVertex[3] = Vertex_t(x, y + height, color);
	pVertex[4] = pVertex[0];

	m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, pVertex, sizeof(Vertex_t));
}

void cRender::DrawGradientBox(short x, short y, short width, short height, D3DCOLOR color1, D3DCOLOR color2, bool vertical) 
{
	DrawGradientBox(x, y, width, height, color1, vertical ? color1 : color2, vertical ? color2 : color1, color2);
}

void cRender::DrawGradientBox(short x, short y, short width, short height, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3, D3DCOLOR color4)
{
	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x, y, color1);
	pVertex[1] = Vertex_t(x + width, y, color2);
	pVertex[2] = Vertex_t(x, y + height, color3);
	pVertex[3] = Vertex_t(x + width, y + height, color4);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawCircle(short x, short y, short radius, uint8_t points, uint8_t flags, D3DCOLOR color1, D3DCOLOR color2)
{
	const bool gradient = (flags & CIRCLE_GRADIENT);
	const bool filled = (flags & CIRCLE_FILLED) || gradient;

	Vertex_t* verticles = new Vertex_t[points + gradient + 1];
	SinCos_t* pSinCos = GetSinCos(points);

	if (gradient)
		verticles[0] = Vertex_t(x, y, color2);

	for (uint8_t i = gradient; i < points + gradient; i++)
	{
		verticles[i] = Vertex_t(x + pSinCos[i - gradient].flCos * radius, y + pSinCos[i - gradient].flSin * radius, color1);
		
		if (filled)
		{
			static const float angle = D3DX_PI / 180.f, flSin = sin(angle), flCos = cos(angle);

			verticles[i - gradient].x = float(x + flCos * (verticles[i - gradient].x - x) - flSin * (verticles[i - gradient].y - y));
			verticles[i - gradient].y = float(y + flSin * (verticles[i - gradient].x - x) + flCos * (verticles[i - gradient].y - y));
		}
	}

	verticles[points + gradient] = verticles[gradient];

	m_pDevice->DrawPrimitiveUP(filled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP, points, verticles, sizeof(Vertex_t));
	delete[] verticles;
}

void cRender::DrawTriangle(short x1, short y1, short x2, short y2, short x3, short y3, D3DCOLOR color, bool filled)
{
	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x1, y1, color);
	pVertex[1] = Vertex_t(x2, y2, color);
	pVertex[2] = Vertex_t(x3, y3, color);
	pVertex[3] = pVertex[0];

	m_pDevice->DrawPrimitiveUP(filled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP, 3, pVertex, sizeof(Vertex_t));
}

void cRender::DrawGradientTriangle(short x1, short y1, short x2, short y2, short x3, short y3, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3)
{
	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x1, y1, color1);
	pVertex[1] = Vertex_t(x2, y2, color2);
	pVertex[2] = Vertex_t(x3, y3, color3);
	pVertex[3] = pVertex[0];

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 3, pVertex, sizeof(Vertex_t));
}