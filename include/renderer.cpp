#include "renderer.h"

cRender::cRender(IDirect3DDevice9* device)
{
	if (!device)
		throw std::exception("device == nullptr");

	m_pDevice = device;
}

cRender::~cRender()
{
	for (auto i = m_SinCosContainer.begin(); i != m_SinCosContainer.end(); i++)
	{
		delete[] i->second;
		i->second = nullptr;
	}

	for (auto& a : m_FontsList)
		if (*a.pFont)
		{
			(*a.pFont)->Release();
			*a.pFont = nullptr;
		}

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
	PushRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	PushRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	m_pDevice->SetTexture(0, NULL);
	m_pDevice->SetPixelShader(NULL);

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

	static int16_t iFrames = 0;
	iFrames++;

	if (dwElapsedTime >= m_iFramerateUpdateRate)
	{
		m_iFramerate = int16_t(iFrames * 1000.f / dwElapsedTime);
		iFrames = 0;
		dwLastUpdateTime = dwCurrentTime;
	}
}

void cRender::PushRenderState(const D3DRENDERSTATETYPE dwState, DWORD dwValue)
{
	DWORD dwTempValue;
	m_pDevice->GetRenderState(dwState, &dwTempValue);
	m_RenderStates.push_back({ dwState, dwTempValue });
	m_pDevice->SetRenderState(dwState, dwValue);
}

void cRender::OnLostDevice()
{
	for (auto& a : m_FontsList)
		if (*a.pFont)
			(*a.pFont)->OnLostDevice();
}

void cRender::OnResetDevice()
{
	for (auto& a : m_FontsList)
		if (*a.pFont)
		{
			(*a.pFont)->OnResetDevice();
			a.update();
		}
}

bool cRender::AddFont(ID3DXFont** pFont, const char* szName, uint8_t iSize, bool bAntiAliased)
{
	font_t font;

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

void cRender::DrawString(int16_t x, int16_t y, color_t color, ID3DXFont* font, bool outlined, bool centered, const char* text, ...)
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
		//black with alpha from "color"
		auto outline_color = static_cast<const color_t>((color.color >> 24) << 24);

		rect.top++;
		font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, outline_color);//x; y + 1
		rect.left++; rect.top--;
		font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, outline_color);//x + 1; y
		rect.left--; rect.top--;
		font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, outline_color);//x; y - 1
		rect.left--; rect.top++;
		font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, outline_color);//x - 1; y
		rect.left++;
	}

	font->DrawTextA(NULL, buf, size, &rect, DT_NOCLIP, color);
}

void cRender::DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, color_t color)
{
	Vertex_t pVertex[2];
	pVertex[0] = Vertex_t(x1, y1, color);
	pVertex[1] = Vertex_t(x2, y2, color);

	m_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, pVertex, sizeof(Vertex_t));
}

void cRender::DrawFilledBox(int16_t x, int16_t y, int16_t width, int16_t height, color_t color)
{
	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x, y, color);
	pVertex[1] = Vertex_t(x + width, y, color);
	pVertex[2] = Vertex_t(x, y + height, color);
	pVertex[3] = Vertex_t(x + width, y + height, color);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawBox(int16_t x, int16_t y, int16_t width, int16_t height, int16_t thickness, color_t color)
{
	DrawFilledBox(x, y, width, thickness, color);
	DrawFilledBox(x, y, thickness, height, color);
	DrawFilledBox(x + width - thickness, y, thickness, height, color);
	DrawFilledBox(x, y + height - thickness, width, thickness, color);
}

void cRender::DrawBox(int16_t x, int16_t y, int16_t width, int16_t height, color_t color)
{
	Vertex_t pVertex[5];

	pVertex[0] = Vertex_t(x, y, color);
	pVertex[1] = Vertex_t(x + width, y, color);
	pVertex[2] = Vertex_t(x + width, y + height, color);
	pVertex[3] = Vertex_t(x, y + height, color);
	pVertex[4] = pVertex[0];

	m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, pVertex, sizeof(Vertex_t));
}

void cRender::DrawGradientBox(int16_t x, int16_t y, int16_t width, int16_t height, color_t color1, color_t color2, bool vertical)
{
	DrawGradientBox(x, y, width, height, color1, vertical ? color1 : color2, vertical ? color2 : color1, color2);
}

void cRender::DrawGradientBox(int16_t x, int16_t y, int16_t width, int16_t height, color_t color1, color_t color2, color_t color3, color_t color4)
{
	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x, y, color1);
	pVertex[1] = Vertex_t(x + width, y, color2);
	pVertex[2] = Vertex_t(x, y + height, color3);
	pVertex[3] = Vertex_t(x + width, y + height, color4);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(Vertex_t));
}

void cRender::DrawCircle(int16_t x, int16_t y, int16_t radius, uint16_t points, RenderDrawType flags, color_t color1, color_t color2)
{
	const bool gradient = (flags & RenderDrawType_Gradient);
	const bool filled = (flags & RenderDrawType_Filled) || gradient;

	Vertex_t* verticles = new Vertex_t[points + gradient + 1];

#if _USE_DYNAMIC_SIN_COS != 1
	SinCos_t* pSinCos = GetSinCos(points);
#endif // !_USE_DYNAMIC_SIN_COS

	if (gradient)
		verticles[0] = Vertex_t(x, y, color2);
	
	for (uint16_t i = gradient; i < points + gradient; i++)
	{
		verticles[i] = Vertex_t(x + pSinCos[i - gradient].flCos * radius, y + pSinCos[i - gradient].flSin * radius, color1);

		if (filled)
		{
			static const float angle = D3DX_PI / 180.f, flSin = sin(angle), flCos = cos(angle);

			verticles[i - gradient].x = static_cast<float>(x + flCos * (verticles[i - gradient].x - x) - flSin * (verticles[i - gradient].y - y));
			verticles[i - gradient].y = static_cast<float>(y + flSin * (verticles[i - gradient].x - x) + flCos * (verticles[i - gradient].y - y));
		}
	}

	verticles[points + gradient] = verticles[gradient];

	m_pDevice->DrawPrimitiveUP(
		(flags & RenderDrawType_Outlined) ? D3DPT_LINESTRIP :
		filled ? D3DPT_TRIANGLEFAN :D3DPT_POINTLIST,
		points, verticles, sizeof(Vertex_t));

	delete[] verticles;
}

void cRender::DrawCircleSector(int16_t x, int16_t y, int16_t radius, uint16_t points, uint16_t angle1, uint16_t angle2, color_t color1, color_t color2)
{
	angle1 += 270;
	angle2 += 270;

	if (angle1 > angle2)
		angle2 += 360;
	
	Vertex_t* verticles = new Vertex_t[points + 2];

	const float stop = 2 * D3DX_PI * static_cast<float>(angle2) / 360.f;
	verticles[points + 1] = Vertex_t(x + cos(stop) * radius, y + sin(stop) * radius, color1);
	verticles[0] = Vertex_t(x, y, color2);

	float angle = 2 * D3DX_PI * static_cast<float>(angle1) / 360.f;
	const float step = ((2 * D3DX_PI * static_cast<float>(angle2) / 360.f) - angle) / points;

	for (uint16_t i = 1; i != points + 1; angle += step, i++)
		verticles[i] = Vertex_t(x + cos(angle) * radius, y + sin(angle) * radius, color1);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, points, verticles, sizeof(Vertex_t));
	delete[] verticles;
}

void cRender::DrawRing(int16_t x, int16_t y, int16_t radius1, int16_t radius2, uint16_t points, RenderDrawType flags, color_t color1, color_t color2)
{
	if (!(flags & RenderDrawType_Gradient))
		color2 = color1;

	if (flags & RenderDrawType_Outlined)
	{
		DrawCircle(x, y, radius1, points, RenderDrawType_Outlined, color1);
		DrawCircle(x, y, radius2, points, RenderDrawType_Outlined, color2);
		return;
	}

	constexpr uint8_t modifier = 4;
	Vertex_t* verticles = new Vertex_t[points * modifier];

	SinCos_t* pSinCos = GetSinCos(points);

	for (uint16_t i = 0; i < points; i++)
	{
		uint16_t it = i * modifier;

		verticles[it] =		Vertex_t(x + pSinCos[i].flCos * radius1, y + pSinCos[i].flSin * radius1, color1);
		verticles[it + 1] = Vertex_t(x + pSinCos[i].flCos * radius2, y + pSinCos[i].flSin * radius2, color2);
		verticles[it + 2] = Vertex_t(x + pSinCos[i + 1].flCos * radius1, y + pSinCos[i + 1].flSin * radius1, color1);
		verticles[it + 3] = Vertex_t(x + pSinCos[i + 1].flCos * radius2, y + pSinCos[i + 1].flSin * radius2, color2);

		for (uint8_t a = 0; a < modifier; a++)
		{
			static const float angle = D3DX_PI / 180.f, flSin = sin(angle), flCos = cos(angle);

			verticles[it].x = static_cast<float>(x + flCos * (verticles[it].x - x) - flSin * (verticles[it].y - y));
			verticles[it].y = static_cast<float>(y + flSin * (verticles[it].x - x) + flCos * (verticles[it].y - y));

			++it;
		}
	}

	--points;

	verticles[points * modifier] = verticles[0];
	verticles[points * modifier + 1] = verticles[1];

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, points * modifier, verticles, sizeof(Vertex_t));
	delete[] verticles;
}

void cRender::DrawRingSector(int16_t x, int16_t y, int16_t radius1, int16_t radius2, uint16_t points, uint16_t angle1, uint16_t angle2, color_t color1, color_t color2)
{
	angle1 += 270;
	angle2 += 270;

	if (angle1 > angle2)
		angle2 += 360;

	const uint8_t modifier = 4;
	Vertex_t* verticles = new Vertex_t[points * modifier];

	const float start = 2 * D3DX_PI * angle1 / 360.f;
	const float stop = 2 * D3DX_PI * angle2 / 360.f;
	const float step = (stop - start) / points;

	SinCos_t sincos[2] = { sin(start), cos(start) };

	for (uint16_t i = 0; i < points; i++)
	{
		const float temp_angle = start + step * i;
		sincos[!(i % 2)] = { sin(temp_angle + step), cos(temp_angle + step) };

		const uint16_t it = i * modifier;

		verticles[it] =		Vertex_t(x + sincos[0].flCos * radius1, y + sincos[0].flSin * radius1, color1);
		verticles[it + 1] = Vertex_t(x + sincos[1].flCos * radius2, y + sincos[1].flSin * radius2, color2);
		verticles[it + 2] = verticles[it];
		verticles[it + 3] = verticles[it + 1];
	}

	--points;

	verticles[0] = Vertex_t(x, y, color2);
	verticles[1] = Vertex_t(x + cos(start) * radius2, y + sin(start) * radius2, color2);
	verticles[points * modifier] =		Vertex_t(x + cos(stop) * radius1 + 1, y + sin(stop) * radius1, color1);
	verticles[points * modifier + 1] =	Vertex_t(x + cos(stop) * radius2 + 1, y + sin(stop) * radius2, color2);

	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, points * modifier, verticles, sizeof(Vertex_t));
	delete[] verticles;
}

void cRender::DrawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, RenderDrawType flags, color_t color1, color_t color2, color_t color3)
{
	if (!(flags & RenderDrawType_Gradient))
	{
		color2 = color1;
		color3 = color1;
	}

	Vertex_t pVertex[4];
	pVertex[0] = Vertex_t(x1, y1, color1);
	pVertex[1] = Vertex_t(x2, y2, color2);
	pVertex[2] = Vertex_t(x3, y3, color3);
	pVertex[3] = pVertex[0];

	m_pDevice->DrawPrimitiveUP(
		(flags & RenderDrawType_Outlined) ? D3DPT_LINESTRIP : 
		(flags & RenderDrawType_Filled) ? D3DPT_TRIANGLEFAN : 
		D3DPT_POINTLIST, 3, pVertex, sizeof(Vertex_t));
}