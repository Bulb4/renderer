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
	/*
	PushRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	PushRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	PushRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	PushRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PushRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	PushRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	*/

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

	static int16_t iFrames = 0;
	iFrames++;

	if (dwElapsedTime >= m_iFramerateUpdateRate)
	{
		m_iFramerate = int16_t(iFrames * 1000.f / dwElapsedTime);
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

void cRender::DrawString(int16_t x, int16_t y, D3DCOLOR color, ID3DXFont* font, bool outlined, bool centered, const char* text, ...)
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
		font->DrawTextA(NULL, buf, size, &size_rect, DT_CALCRECT | DT_NOCLIP, 0);

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

void cRender::DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, D3DCOLOR color) 
{
	Vertex_t pVertex[2];
	pVertex[0] = Vertex_t(x1, y1, color);
	pVertex[1] = Vertex_t(x2, y2, color);

	m_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, pVertex, sizeof(Vertex_t));
}

void cRender::DrawFilledBox(int16_t x, int16_t y, int16_t width, int16_t height, D3DCOLOR color) 
{
	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x, y, color);
	pVertex[1] = Vertex_t(x + width, y, color);
	pVertex[2] = Vertex_t(x, y + height, color);
	pVertex[3] = Vertex_t(x + width, y + height, color);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawBox(int16_t x, int16_t y, int16_t width, int16_t height, int16_t thickness, D3DCOLOR color)
{
	DrawFilledBox(x, y, width, thickness, color);
	DrawFilledBox(x, y, thickness, height, color);
	DrawFilledBox(x + width - thickness, y, thickness, height, color);
	DrawFilledBox(x, y + height - thickness, width, thickness, color);
}

void cRender::DrawBox(int16_t x, int16_t y, int16_t width, int16_t height, D3DCOLOR color)
{
	Vertex_t pVertex[5];

	pVertex[0] = Vertex_t(x, y, color);
	pVertex[1] = Vertex_t(x + width, y, color);
	pVertex[2] = Vertex_t(x + width, y + height, color);
	pVertex[3] = Vertex_t(x, y + height, color);
	pVertex[4] = pVertex[0];

	m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, pVertex, sizeof(Vertex_t));
}

void cRender::DrawGradientBox(int16_t x, int16_t y, int16_t width, int16_t height, D3DCOLOR color1, D3DCOLOR color2, bool vertical) 
{
	DrawGradientBox(x, y, width, height, color1, vertical ? color1 : color2, vertical ? color2 : color1, color2);
}

void cRender::DrawGradientBox(int16_t x, int16_t y, int16_t width, int16_t height, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3, D3DCOLOR color4)
{
	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x, y, color1);
	pVertex[1] = Vertex_t(x + width, y, color2);
	pVertex[2] = Vertex_t(x, y + height, color3);
	pVertex[3] = Vertex_t(x + width, y + height, color4);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawCircle(int16_t x, int16_t y, int16_t radius, uint16_t points, uint8_t flags, D3DCOLOR color1, D3DCOLOR color2)
{
	const bool gradient = (flags & RDT_GRADIENT);
	const bool filled = (flags & RDT_FILLED) || gradient;

	Vertex_t* verticles = new Vertex_t[points + gradient + 1];
	SinCos_t* pSinCos = GetSinCos(points);
	
	if (gradient)
		verticles[0] = Vertex_t(x, y, color2);

	for (uint16_t i = gradient; i < points + gradient; i++)
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

void cRender::DrawCircleSector(int16_t x, int16_t y, int16_t radius, uint16_t points, uint16_t angle1, uint16_t angle2, D3DCOLOR color1, D3DCOLOR color2)
{
	angle1 += 270;
	angle2 += 270;

	if (angle1 > angle2)
		angle2 += 360;

	Vertex_t* verticles = new Vertex_t[points + 2];

	const float stop = 2 * D3DX_PI * float(angle2) / 360.f;
	verticles[points + 1] = Vertex_t(x + cos(stop) * radius, y + sin(stop) * radius, color1);
	verticles[0] = Vertex_t(x, y, color2);

	float angle = 2 * D3DX_PI * float(angle1) / 360.f;
	const float step = ((2 * D3DX_PI * float(angle2) / 360.f) - angle) / points;

	for (uint16_t i = 1; i != points + 1; angle += step, i++)
		verticles[i] = Vertex_t(x + cos(angle) * radius, y + sin(angle) * radius, color1);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, points, verticles, sizeof(Vertex_t));
	delete[] verticles;
}

void cRender::DrawRing(int16_t x, int16_t y, int16_t radius1, int16_t radius2, uint16_t points, uint8_t flags, D3DCOLOR color1, D3DCOLOR color2)
{
	if (!(flags & RDT_GRADIENT))
		color2 = color1;

	if (flags & RDT_OUTLINED)
	{
		DrawCircle(x, y, radius1, points, RDT_OUTLINED, color1);
		DrawCircle(x, y, radius2, points, RDT_OUTLINED, color2);
		return;
	}

	const uint8_t modifier = 4;
	Vertex_t* verticles = new Vertex_t[points * modifier];
	SinCos_t* pSinCos = GetSinCos(points);
	
	for (uint16_t i = 0; i < points; i++)
	{
		uint16_t it = i * modifier;

		verticles[it + 0] = Vertex_t(x + pSinCos[i].flCos * radius1, y + pSinCos[i].flSin * radius1, color1);
		verticles[it + 1] = Vertex_t(x + pSinCos[i].flCos * radius2, y + pSinCos[i].flSin * radius2, color2);
		verticles[it + 2] = Vertex_t(x + pSinCos[i + 1].flCos * radius1, y + pSinCos[i + 1].flSin * radius1, color1);
		verticles[it + 3] = Vertex_t(x + pSinCos[i + 1].flCos * radius2, y + pSinCos[i + 1].flSin * radius2, color2);

		for (uint8_t a = 0; a < modifier; a++)
		{
			static const float angle = D3DX_PI / 180.f, flSin = sin(angle), flCos = cos(angle);

			verticles[it].x = float(x + flCos * (verticles[it].x - x) - flSin * (verticles[it].y - y));
			verticles[it].y = float(y + flSin * (verticles[it].x - x) + flCos * (verticles[it].y - y));

			it++;
		}
	}

	points--;

	verticles[points * modifier + 0] = verticles[0];
	verticles[points * modifier + 1] = verticles[1];
	
	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, points * modifier, verticles, sizeof(Vertex_t));
	delete[] verticles;
}

void cRender::DrawRingSector(int16_t x, int16_t y, int16_t radius1, int16_t radius2, uint16_t points, uint16_t angle1, uint16_t angle2, D3DCOLOR color1, D3DCOLOR color2)
{
	angle1 += 270;
	angle2 += 270;

	if (angle1 > angle2)
		angle2 += 360;

	const uint8_t modifier = 4;
	Vertex_t* verticles = new Vertex_t[points * modifier];
	
	const float start = 2 * D3DX_PI * float(angle1) / 360.f;
	const float stop = 2 * D3DX_PI * float(angle2) / 360.f;
	const float step = (stop - start) / points;
	
	SinCos_t sincos[2] = { sin(start), cos(start) };

	for (uint16_t i = 0; i < points; i++)
	{
		const uint16_t it = i * modifier;

		const float temp_angle = start + step * i;
		sincos[!bool(i % 2)] = { sin(temp_angle + step), cos(temp_angle + step) };

		verticles[it] = Vertex_t(x + sincos[0].flCos * radius1, y + sincos[0].flSin * radius1, color1);
		verticles[it + 1] = Vertex_t(x + sincos[1].flCos * radius2, y + sincos[1].flSin * radius2, color2);
		verticles[it + 2] = verticles[it];
		verticles[it + 3] = verticles[it + 1];
	}

	points--;

	verticles[0] = Vertex_t(x, y, color2);
	verticles[1] = Vertex_t(x + cos(start) * radius2, y + sin(start) * radius2, color2);
	verticles[points * modifier + 0] = Vertex_t(x + cos(stop) * radius1 + 1, y + sin(stop) * radius1, color1);
	verticles[points * modifier + 1] = Vertex_t(x + cos(stop) * radius2 + 1, y + sin(stop) * radius2, color2);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, points * modifier, verticles, sizeof(Vertex_t));
	delete[] verticles;
}

void cRender::DrawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint8_t flags, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3)
{
	if (!(flags & RDT_GRADIENT))
	{
		color2 = color1;
		color3 = color1;
	}

	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x1, y1, color1);
	pVertex[1] = Vertex_t(x2, y2, color2);
	pVertex[2] = Vertex_t(x3, y3, color3);
	pVertex[3] = pVertex[0];

	m_pDevice->DrawPrimitiveUP(flags & RDT_FILLED ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP, 3, pVertex, sizeof(Vertex_t));
}