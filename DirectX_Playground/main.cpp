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

	ID3D12Device* DirectXDevice = 0;
	IDXGIAdapter1* pAdapter = 0;
	IDXGIFactory1* pFactory = 0;

	if (CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory)))
	{
		MessageBox(WindowPtr, "Error in creating DirectX Graphics Infastructure device.",
			0, 0);
	}
	pFactory->EnumAdapters1(1, &pAdapter);

	if (D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&DirectXDevice)))
	{
		MessageBox(WindowPtr, "Error in creating d3d12device.", 0, 0);
	}
	
	ID3D12Fence* Fence = 0;
	UINT64 FenceValue = 0;
	if (DirectXDevice->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, 
		IID_PPV_ARGS(&Fence)))//__uuidof(ID3D12Fence), reinterpret_cast<void**>(Fence)))
	{
		MessageBox(WindowPtr, "Failed to create Fence", 0, 0);
	}

	UINT mRtvDescriptorSize = 
		DirectXDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT mDsvDescriptorSize = 
		DirectXDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	UINT mCbvSrvDescriptorSize = 
		DirectXDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;

	//??????
	DirectXDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, 
		&msQualityLevels, sizeof(msQualityLevels));
	//m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	//assert(m4xMsaaQuality > 0 && “Unexpected MSAA quality
	//	level.”);


	// Command Objects
	//
	//

	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//CommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// NOTE: NodeMask is for if there are more than one GPUs
	CommandQueueDesc.NodeMask = 0;

	ID3D12CommandQueue *CommandQueue = 0;

	if (DirectXDevice->CreateCommandQueue(&CommandQueueDesc,
		IID_PPV_ARGS(&CommandQueue))) //__uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(CommandQueue)))
	{
		MessageBox(WindowPtr, "Failed to create Command Queue", 0, 0);
	}

	ID3D12CommandAllocator* CommandAllocator = 0;

	if (DirectXDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&CommandAllocator)))
	{
		MessageBox(WindowPtr, "Failed to create Command Allocator", 0, 0);
	}

	ID3D12PipelineState* PipelineState = 0;
	ID3D12GraphicsCommandList* CommandList; // NOTE: Needs to be closed

	if (DirectXDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		CommandAllocator, PipelineState, IID_PPV_ARGS(&CommandList)))
	{
		MessageBox(WindowPtr, "Failed to create Command List", 0, 0);
	}

	CommandList->Close();

	//Swap Chain
	//
	//
	DXGI_RATIONAL Rational;
	Rational.Denominator = 1;
	Rational.Numerator = 60;

	DXGI_MODE_DESC ModeDesc;
	ModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ModeDesc.Height = WINDOW_HEIGHT;
	ModeDesc.RefreshRate = Rational;
	ModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	ModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	ModeDesc.Width = WINDOW_WIDTH;

	DXGI_SAMPLE_DESC SampleDesc;
	SampleDesc.Count = 0;
	SampleDesc.Quality = 0;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.BufferDesc = ModeDesc;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.Flags = 0;
	SwapChainDesc.OutputWindow = WindowPtr;
	SwapChainDesc.SampleDesc = SampleDesc;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Windowed = 1;

	IDXGISwapChain* SwapChain;

	pFactory->CreateSwapChain(CommandQueue, &SwapChainDesc, &SwapChain);
	

	// Render Target View and Depth Stencil Descriptor Heaps
	//
	//

	D3D12_DESCRIPTOR_HEAP_DESC RTVDescHeap;
	RTVDescHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVDescHeap.NodeMask = 0;
	RTVDescHeap.NumDescriptors = 2;
	RTVDescHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ID3D12DescriptorHeap* RenderTargetViewDescHeap;

	if (DirectXDevice->CreateDescriptorHeap(&RTVDescHeap,
		IID_PPV_ARGS(&RenderTargetViewDescHeap)))
	{
		MessageBox(WindowPtr,
			"Error in making Render Target View Descriptor Heap", 0, 0);
	}
	
	D3D12_DESCRIPTOR_HEAP_DESC DSVDescHeap;
	DSVDescHeap.NumDescriptors = 1;
	DSVDescHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVDescHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVDescHeap.NodeMask = 0;

	ID3D12DescriptorHeap* DepthStencilViewDescHeap;

	if (DirectXDevice->CreateDescriptorHeap(&DSVDescHeap,
		IID_PPV_ARGS(&DepthStencilViewDescHeap)))
	{
		MessageBox(WindowPtr, 
			"Error in making Depth Stencil View Descriptor Heap.", 0, 0);
	}

	// On Resize
	//
	//
	// Fush command queue()
	UINT64 CurrentFence = 0;

	CommandQueue->Signal(Fence, CurrentFence);
	if (Fence->GetCompletedValue() < CurrentFence)
	{
		HANDLE EventHandle = CreateEventEx(0, false, false, EVENT_ALL_ACCESS);
		Fence->SetEventOnCompletion(CurrentFence, EventHandle);

		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
	// end flush command queue
	CommandList->Reset(CommandAllocator, 0);
	//for (int i = 0; i < 2; i++)
	//{
	//	
	//}
	SwapChain->ResizeBuffers(2, WINDOW_WIDTH, WINDOW_HEIGHT,
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	
	ID3D12Resource* SwapChainBuffer[2];

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHeapHandle = RenderTargetViewDescHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < 2; i++)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));
		DirectXDevice->CreateRenderTargetView(SwapChainBuffer[i],
			0, RTVHeapHandle);
		//RTVHeapHandle = RTVHeapHandle + 1;
	}
	
	//DepthStencilViewDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RESOURCE_DESC DepthStencilDesc;
	DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDesc.Alignment = 0;
	DepthStencilDesc.Width = WINDOW_WIDTH;
	DepthStencilDesc.Height = WINDOW_HEIGHT;
	DepthStencilDesc.DepthOrArraySize = 1;
	DepthStencilDesc.MipLevels = 1;
	DepthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	DepthStencilDesc.SampleDesc.Count = 1;
	DepthStencilDesc.SampleDesc.Quality = 0;
	DepthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	ID3D12Resource* DepthStencilBuffer = 0;
	D3D12_CLEAR_VALUE OptClear;
	OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	OptClear.DepthStencil.Depth = 1.0f;
	OptClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES HeapProperties;
	HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.CreationNodeMask = 0;
	HeapProperties.VisibleNodeMask = 0;

	DirectXDevice->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE, &DepthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON, &OptClear,
		IID_PPV_ARGS(&DepthStencilBuffer));

	//////////////////////
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	DirectXDevice->CreateDepthStencilView(DepthStencilBuffer, &dsvDesc,
		DepthStencilViewDescHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_RESOURCE_BARRIER ResourceBarrier;
	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//ResourceBarrier.Transition = ;
	//ResourceBarrier.Aliasing = ;
	//ResourceBarrier.UAV = ;

	// Transition the resource from its initial state to be used as a depth buffer.
	CommandList->ResourceBarrier(1, &ResourceBarrier);

	// Execute the resize commands.
	CommandList->Close();
	ID3D12CommandList* cmdsLists[] = { CommandList };
	CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	CurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	CommandQueue->Signal(Fence, CurrentFence);

	// Wait until the GPU has completed commands up to this fence point.
	if (Fence->GetCompletedValue() < CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		Fence->SetEventOnCompletion(CurrentFence, eventHandle);

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	D3D12_VIEWPORT ScreenViewport;

	// Update the viewport transform to cover the client area.
	ScreenViewport.TopLeftX = 0;
	ScreenViewport.TopLeftY = 0;
	ScreenViewport.Width = static_cast<float>(WINDOW_WIDTH);
	ScreenViewport.Height = static_cast<float>(WINDOW_HEIGHT);
	ScreenViewport.MinDepth = 0.0f;
	ScreenViewport.MaxDepth = 1.0f;

	D3D12_RECT ScissorRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	////////////////////
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

		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished execution on the GPU.
		DirectCmdListAlloc->Reset();

		// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
		// Reusing the command list reuses memory.
		CommandList->Reset(DirectCmdListAlloc, nullptr);

		// Indicate a state transition on the resource usage.
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
		CommandList->RSSetViewports(1, &ScreenViewport);
		CommandList->RSSetScissorRects(1, &ScissorRect);

		// Clear the back buffer and depth buffer.
		CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
		CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Specify the buffers we are going to render to.
		CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

		// Indicate a state transition on the resource usage.
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		// Done recording commands.
		CommandList->Close();

		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { CommandList };
		CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// swap the back and front buffers
		SwapChain->Present(0, 0);
		CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

		// Wait until frame commands are complete.  This waiting is inefficient and is
		// done for simplicity.  Later we will show how to organize our rendering code
		// so we do not have to wait per frame.
		mCurrentFence++;

		// Add an instruction to the command queue to set a new fence point.  Because we 
		// are on the GPU timeline, the new fence point won't be set until the GPU finishes
		// processing all the commands prior to this Signal().
		CommandQueue->Signal(Fence, mCurrentFence);

		// Wait until the GPU has completed commands up to this fence point.
		if (Fence->GetCompletedValue() < mCurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

			// Fire event when GPU hits current fence.  
			Fence->SetEventOnCompletion(mCurrentFence, eventHandle));

			// Wait until the GPU hits current fence event is fired.
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
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
