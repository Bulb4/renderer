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

static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp;

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
	HWND hwnd = CreateWindow(WINDOW_NAME, "DirectX 9 Test", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

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
	pRender->AddFont(&font1, "Consolas", 48, false);

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
			pRender->BeginDraw();
			
			pRender->DrawGradientBox(20, 20, 200, 50, Color::Blue, 0xFFCCCC00, true);

			pRender->DrawFilledBox(240, 20, 200, 50, Color::SkyBlue);
			pRender->DrawBox(460, 20, 200, 50, 4, Color::Black);
			pRender->DrawBox(680, 20, 200, 50, Color::Black);
			pRender->DrawGradientBox(900, 20, 200, 50, Color::Blue, Color::Green, Color::Red, Color::Yellow);

			pRender->DrawCircle(60, 190, 100, 32, Color::Red, true);
			pRender->DrawCircle(340, 190, 100, 32, Color::Yellow);
			pRender->DrawGradientCircle(560, 190, 100, 32, Color::Green, 0);

			pRender->DrawTriangle(120, 310, 20, 480, 220, 480, Color::Green);
			pRender->DrawTriangle(340, 310, 240, 480, 440, 480, Color::SkyBlue, true);
			pRender->DrawGradientTriangle(560, 310, 460, 480, 660, 480, Color::Yellow, Color::Green, Color::Red);
		
			//text panel
			{
				static int cpu_usage = 0;
				static uint16_t last_fps = 0;

				const uint16_t current_fps = pRender->GetFramerate();
				
				if (last_fps != current_fps)
					cpu_usage = GetUsageOfCPU();

				last_fps = current_fps;

				pRender->DrawString(
					680, 90,
					Color::White,
					font1, true, false,
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

	if (g_pd3dDevice)
		g_pd3dDevice->Release();

	if (pD3D) 
		pD3D->Release();

	DestroyWindow(hwnd);
	UnregisterClass(WINDOW_NAME, wc.hInstance);

	return 0;
}