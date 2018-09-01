#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <iostream>

#define DIRECTINPUT_VERSION 0x0800
#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")
#include <dinput.h>
#include <tchar.h>

#include "../include/renderer.h"

static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp;

LPDIRECT3DTEXTURE9 pRadarTexture = nullptr;
LPD3DXSPRITE pRadarSprite = nullptr;


short GetUsageOfCPU()
{
	const static HANDLE hCurrentProcess = GetCurrentProcess();

	static DWORD dwNumberOfProcessors = 0;

	if (!dwNumberOfProcessors)
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		dwNumberOfProcessors = info.dwNumberOfProcessors;
	}

	FILETIME now, creation_time, exit_time, kernel_time, user_time;

	GetSystemTimeAsFileTime(&now);

	if (!GetProcessTimes(hCurrentProcess, &creation_time, &exit_time, &kernel_time, &user_time))
		return -1;

	static auto QuadPartFt = [](const FILETIME* ft) -> uint64_t
	{
		LARGE_INTEGER li;
		li.LowPart = ft->dwLowDateTime;
		li.HighPart = ft->dwHighDateTime;

		return li.QuadPart;
	};

	const int64_t system_time = (QuadPartFt(&kernel_time) + QuadPartFt(&user_time)) / dwNumberOfProcessors;
	const int64_t time = QuadPartFt(&now);

	static int64_t last_system_time = 0, last_time = 0;

	if (!last_system_time || !last_time)
	{
		last_system_time = system_time;
		last_time = time;
		return -1;
	}

	const int64_t time_delta = time - last_time;

	if (!time_delta)
		return -1;

	const auto ret = short(((system_time - last_system_time) * 100 + time_delta / 2) / time_delta);

	last_system_time = system_time;
	last_time = time;

	return ret;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int CALLBACK WinMain(
	_In_ HINSTANCE, _In_ HINSTANCE,
	_In_ LPSTR,	_In_ int)
{
#define WINDOW_NAME "DirectX9 Example"

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, WINDOW_NAME, NULL };
	RegisterClassEx(&wc);
	HWND hwnd = CreateWindow(WINDOW_NAME, "DirectX 9 Test", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, NULL, NULL, wc.hInstance, NULL);

	LPDIRECT3D9 pD3D;
	if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		UnregisterClass(WINDOW_NAME, wc.hInstance);
		return 0;
	}

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;//D3DPRESENT_INTERVAL_ONE;

	if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
	{
		pD3D->Release();
		UnregisterClass(WINDOW_NAME, wc.hInstance);
		return 0;
	}
	
	D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
	pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &AdapterIdentifier);

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	cRender* pRender = new cRender(g_pd3dDevice);

	ID3DXFont* font1 = nullptr;
	pRender->AddFont(&font1, "System", 24, false);

	pRender->SetFramerateUpdateRate(400U);
	
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

		g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB(100, 150, 240), 1.0f, 0);
		
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			static float time = 0;

			time += 0.05f;

			if (time > 360.f)
				time -= 360.f;

			const uint8_t size = 3;
			color_t rainbow_color[size];

			for (uint8_t i = 1; i <= size; i++)
				rainbow_color[i - 1].SetHSV(time / 360.f + 1.f / i, 1.f, 1.f);

			pRender->BeginDraw();
			pRender->PushRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
			
			pRender->DrawLine(20, 10, 880, 10, Colors::Black);

			pRender->DrawBox(20, 20, 200, 50, Colors::White);
			pRender->DrawFilledBox(240, 20, 200, 50, 0x8000CCCC);
			pRender->DrawGradientBox(460, 20, 200, 50, rainbow_color[0], rainbow_color[1], rainbow_color[2], rainbow_color[3]);
			pRender->DrawBox(680, 20, 200, 50, 8, Colors::Pink);

			pRender->DrawCircle(120, 190, 100, 32, RenderDrawType_Outlined, Colors::Red);
			pRender->DrawCircle(340, 190, 100, 32, RenderDrawType_Filled, Colors::Green);
			pRender->DrawCircle(560, 190, 100, 32, RenderDrawType_Gradient, rainbow_color[0], rainbow_color[1]);

			pRender->DrawRing(780, 190, 100, 80, 64, RenderDrawType_Filled, Colors::Blue);

			pRender->DrawTriangle(120, 310, 20, 480, 220, 480, RenderDrawType_Outlined, Colors::Green);
			pRender->DrawTriangle(340, 310, 240, 480, 440, 480, RenderDrawType_Filled, Colors::SkyBlue);
			pRender->DrawTriangle(560, 310, 460, 480, 660, 480, RenderDrawType_FilledGradient, rainbow_color[0], rainbow_color[1], rainbow_color[2]);

			pRender->DrawCircleSector(780, 380, 80, 30, time, time + 45, rainbow_color[0], rainbow_color[1]);
			pRender->DrawRingSector(780, 380, 65, 80, 30, time + 180, time + 225, rainbow_color[2], rainbow_color[2]);

			//text panel
			{
				static int cpu_usage = 0;
				static uint16_t last_fps = 0;

				const uint16_t current_fps = pRender->GetFramerate();

				if (last_fps != current_fps)
					cpu_usage = GetUsageOfCPU();

				last_fps = current_fps;

				pRender->DrawString(
					900, 10, Colors::White, font1, true, false,
					"CPU: %i%%\nFPS: %d\nCPU Cores: %i\n%s",
					cpu_usage, current_fps,
					info.dwNumberOfProcessors,
					AdapterIdentifier.Description);
			}

			pRender->EndDraw();
			
			g_pd3dDevice->EndScene();
		}

		if (g_pd3dDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST &&
			g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			pRender->OnLostDevice();

			if (g_pd3dDevice->Reset(&g_d3dpp) >= 0)
				pRender->OnResetDevice();
		}
	}

	delete pRender;

	if (g_pd3dDevice)
		g_pd3dDevice->Release();

	if (pD3D) 
		pD3D->Release();

	DestroyWindow(hwnd);
	UnregisterClass(WINDOW_NAME, wc.hInstance);

	return 0;
}