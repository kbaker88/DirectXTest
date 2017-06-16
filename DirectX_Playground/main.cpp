#include <Windows.h>
#include <D3d12.h>
#include <DXGI.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef HGLRC __stdcall wgl_create_context_attribs_arb(HDC hDC,
	HGLRC hShareContext, const int *attribList);

LRESULT CALLBACK WndProc(HWND WindowPtr, UINT msg, WPARAM wParam,
	LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int CommandShow)
{
	WNDCLASSEX WindowsClassStructure;
	WindowsClassStructure.cbSize = sizeof(WNDCLASSEX);
	WindowsClassStructure.style = CS_OWNDC;
	WindowsClassStructure.lpfnWndProc = WndProc;
	WindowsClassStructure.cbClsExtra = 0;
	WindowsClassStructure.cbWndExtra = 0;
	WindowsClassStructure.hInstance = hInstance;
	WindowsClassStructure.hIcon = LoadIcon(0, IDI_APPLICATION);
	WindowsClassStructure.hCursor = LoadCursor(0, IDC_ARROW);
	WindowsClassStructure.hbrBackground = (HBRUSH)(COLOR_WINDOW + 3);
	WindowsClassStructure.lpszMenuName = 0;
	WindowsClassStructure.lpszClassName =
		(const char*)"VulkanPlayground";
	WindowsClassStructure.hIconSm = LoadIcon(0, IDI_APPLICATION);

	if (!RegisterClassEx(&WindowsClassStructure))
	{
		MessageBox(0, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
	}

	HWND WindowPtr = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		(const char*)"VulkanPlayground",
		"Vulkan Playground",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
		0, 0, hInstance, 0);

	if (!WindowPtr)
	{
		MessageBox(0, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
	}

	ID3D12Device* DirectXDevice;
	IDXGIAdapter1* pAdapter = nullptr;
	IDXGIFactory1 * pFactory;
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	pFactory->EnumAdapters1(1, &pAdapter);

	if (D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&DirectXDevice)) != S_FALSE)//_uuidof(ID3D12Device), 0) != S_FALSE)
	{
		MessageBox(WindowPtr, "Error in creating d3d12device.", 0, 0);
	}

	
	DirectXDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&DirectXDevice));// __uuidof(ID3D12Fence), 0);

	UINT mRtvDescriptorSize = DirectXDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT mDsvDescriptorSize = DirectXDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	UINT mCbvSrvDescriptorSize = DirectXDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;	msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;

	DirectXDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels,
		sizeof(msQualityLevels)));
	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && “Unexpected MSAA quality
		level.”);
	
	UpdateWindow(WindowPtr);
	ShowWindow(WindowPtr, CommandShow);

	//HDC WindowDeviceContext = GetDC(WindowPtr);

	MSG Message = {};
	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}

	return Message.message;
}


LRESULT CALLBACK WndProc(HWND WindowPtr, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_CREATE:
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
	{

	} break;
	case WM_CHAR:
	{
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
	} break;

	default:
		return DefWindowProc(WindowPtr, Message, wParam, lParam);
	}

	return 0;
}
