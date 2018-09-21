#pragma once

#include "renderer.h"

class cFont
{
public:
	cFont() { }
	cFont(IDirect3DDevice9* pDevice)
	{
		this->pDevice = pDevice;
		Update();
	}
	cFont(IDirect3DDevice9* pDevice, const char* szName, uint8_t iSize)
	{
		sprintf(this->szName, szName);

		this->iSize = iSize;
		this->pDevice = pDevice;

		Update();
	}
	cFont(IDirect3DDevice9* pDevice, const char* szName, uint8_t iSize, uint16_t iFontWeight, uint16_t iCharset, bool bItalic, bool bAntiAliased)
	{
		sprintf(this->szName, szName);

		this->iSize = iSize;
		this->iFontWeight = iFontWeight;
		this->iCharset = iCharset;
		this->bItalic = bItalic;
		this->bAntiAliased = bAntiAliased;
		this->pDevice = pDevice;

		Update();
	}

	~cFont()
	{
		RELEASE_INTERFACE(pFont);
	}

	IDirect3DDevice9* pDevice;
	char szName[32] = "System";
	int32_t iSize = 14;
	//0, 100, 200 ... 1000
	uint32_t iFontWeight = FW_NORMAL;
	uint32_t iCharset = DEFAULT_CHARSET;
	bool bItalic = false;
	bool bAntiAliased = false;

	bool Update()
	{
		RELEASE_INTERFACE(pFont);

		return D3DXCreateFontA(pDevice, iSize, 0, 
			iFontWeight, 1, BOOL(bItalic), iCharset, OUT_DEFAULT_PRECIS, 
			bAntiAliased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY, 
			DEFAULT_PITCH, szName, &pFont) == D3D_OK;
	}

	ID3DXFont* GetFont() { return pFont; }

	void OnLostDevice()
	{
		if (pFont)
			pFont->OnLostDevice();
	}
	void OnResetDevice()
	{
		if (pFont)
		{
			pFont->OnResetDevice();
			Update();
		}
	}
private:
	ID3DXFont* pFont = nullptr;
};