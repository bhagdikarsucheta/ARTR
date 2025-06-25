
// Header files
#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"VK.h"

//Vulkan related Macro define
#define VK_USE_PLATFORM_WIN32_KHR		//String Macro

//Vulkan related Header files
#include<vulkan/vulkan.h>

//Vulkan Instance
//VKInstance vkInstance = VK_NULL;

//Vulkan related libraries
#pragma comment(lib,"vulkan-1.lib")

//Vulkan Instance
VkInstance vkInstance = VK_NULL_HANDLE;

// Macro definitions
#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600

// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

const char* gpszAppName = "ARTR";
static DWORD dwStyle;
static WINDOWPLACEMENT wpPrev;

// Global variables
HWND ghwnd;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;
FILE* gpFile = NULL;

//Vulkan related Global variables


//Instance extension related variables
uint32_t enabledInstanceExtensionCount = 0;

//VK_KHR_SURFACE_EXTENSION_NAME and VK_KHR_WIN32_SURFACE_EXTENSION_NAME and VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[3];

//Vulkan presentation object
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

//Vulkan Physical device related
VkPhysicalDevice vkPhysicalDevice_Selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_Selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

//For vkInfo
uint32_t physicalDeviceCount = 0;
VkPhysicalDevice* vkPhysicalDevice_array = NULL;

////////For Device Extension Printing
//Device extension related variables
uint32_t enabledDeviceExtensionCount = 0;

//VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char* enabledDeviceExtensionNames_array[1];


//Vulkan Device
VkDevice vkDevice = VK_NULL_HANDLE;


//Device Queue
VkQueue vkQueue = VK_NULL_HANDLE;

//Color Format and Color Space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpacrKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

//Presentation Mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

//SwapChain releted global variables
int winWidth = WINDOW_WIDTH;
int winHeight = WINDOW_HEIGHT;
VkSwapchainKHR vkSwapchainKHR;
VkExtent2D vkExtent2D_Swapchain;


//Swapchain Image and Swapchain ImageView related variables
uint32_t swapchainImageCount = UINT32_MAX;
VkImage* swapchainImage_array = NULL;
VkImageView* swapchainImageView_array = NULL;


//Command Pool variables
VkCommandPool vkCommandPool = VK_NULL_HANDLE;


//Command Buffer
VkCommandBuffer* vkCommandBuffer_array = NULL;


//RenderPass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

//Framebuffer
VkFramebuffer* vkFrameBuffer_array = NULL;


//Semaphore
VkSemaphore vkSemaphore_BackBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_RenderComplete = VK_NULL_HANDLE;


//Fence
VkFence* vkFence_array = NULL;



//clear color vaues
VkClearColorValue vkClearColorValue;			//struct (has 3 arrays of 4 members, float32[4], int32[4],uint32[4], we will use float32)



//For blue screen
BOOL bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX;


//Validation
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char* enabledValidationLayerNames_array[1];					//For VK_LAYER_KHRONOS_validation 
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT  vkDestroyDebugReportCallbackEXT_fnptr = NULL;

//Veretx Buffer related variables
typedef struct
{
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;

}VertexData;


//Position
VertexData vertexData_Position;


//Shader related variables
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;



//Descriptor Set Layout
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;



//Pipeline Layout
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;


//
VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;

// Entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function declarations
	VkResult Initialize(void);
	VkResult Display(void);
	void Update(void);
	void Uninitialize(void);

	// Variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[255];
	BOOL bDone = FALSE;
	int iRetVal = 0;
	VkResult vkResult = VK_SUCCESS;



	// Code
	gpFile = fopen("VulkanLog.txt", "w");
	if (gpFile == NULL)
	{
		MessageBox(NULL, TEXT("Failed to create log file. Exiting now..."), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "WinMain()->Program Started successfully.\n");
	}
	wsprintf(szAppName, "%s", gpszAppName);
	// Initialize WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// Register above class
	RegisterClassEx(&wndclass);

	// Create the window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("SAB Vulkan : Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	// Initialize
	vkResult = Initialize();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nWinMain()->Initilize Function is Failed\n");
		DestroyWindow(hwnd);
		hwnd = NULL;;
	}
	else {
		fprintf(gpFile, "\nWinMain()->Initilize Function is Succeded\n");
	}

	// Show window
	ShowWindow(hwnd, iCmdShow);

	// Bring window to foreground and set focus on it
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game loop
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == TRUE)
			{
				// Render the scene
				vkResult = Display();
				if (vkResult != VK_FALSE && vkResult != VK_SUCCESS)
				{
					fprintf(gpFile, "\nWinMain()->Display() Function is Failed\n");  ///Display is Failed when display returned result is not VK_SUCCESS && its not an VK_FALSE
					bDone = TRUE;
				}

				// Update the scene
				Update();
			}
		}
	}

	Uninitialize();

	return ((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// Function declarations
	void ToggleFullScreen(void);
	VkResult Resize(int, int);
	void uninitialize(void);

	// Code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;

	case WM_ERASEBKGND:
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case 'f':
		case 'F':
			ToggleFullScreen();
			break;

		default:
			break;
		}
		break;
	case WM_CREATE:

		memset(0, 0, 0);
		wpPrev.length = sizeof(WINDOWPLACEMENT);

		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:	// Escape key
			DestroyWindow(hwnd);
			break;

		default:
			break;
		}
		break;

	case WM_SIZE:
		 Resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void ToggleFullScreen(void)
{
	// Variable declarations
	static MONITORINFO mi;

	// Code

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wpPrev) &&
				GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					(mi.rcMonitor.right - mi.rcMonitor.left),
					(mi.rcMonitor.bottom - mi.rcMonitor.top),
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
}


VkResult Initialize(void)
{
	// Function declarations
	VkResult createVulkanInstance(void);

	VkResult getSupportedSurface(void);

	VkResult getPhysicalDevice(void);

	VkResult printVKInfo(void);

	VkResult createVulkanDevice(void);

	void getDeviceQueue(void);

	VkResult createSwapchain(VkBool32);

	VkResult createImagesAndImageViews(void);

	VkResult createCommandPool(void);

	VkResult createCommandBuffers(void);

	VkResult createVertexBuffer(void);

	VkResult createShaders(void);

	VkResult createDescriptorSetLayout(void);

	VkResult createPipelineLayout(void);

	VkResult createRenderPass(void);

	VkResult createPipeline(void);

	VkResult createFrameBuffers(void);

	VkResult createSemaphores(void);

	VkResult createFences(void);

	VkResult buildComaandBuffers(void);







	// Variable declarations
	VkResult vkResult = VK_SUCCESS;
	//VkResult vkResult = VK_SUCCESS;




	//Variable declarations


	vkResult = createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createVulkanInstance  is Failed %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\nInitialize()->createVulkanInstance Function is Succeded\n");
	}



	//Create vulkan presentation surface
	vkResult = getSupportedSurface();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->getSupportedSurface is Failed %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\nInitialize()->getSupportedSurface Function is Succeded\n");
	}

	//enumerate and select physical device and its queue family index
	vkResult = getPhysicalDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "Initialize()->getPhysicalDevice Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "Initialize()->getPhysicalDevice Function is Succeded\n");
	}



	//print Vulkan Info vkInfo
	vkResult = printVKInfo();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->printVKInfo Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->printVKInfo Function is Succeded\n");
	}




	//Device Extension Names
	vkResult = createVulkanDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createVulkanDevice Function is %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\nInitialize()->createVulkanDevice Function is Succeded");
	}


	//get device queue
	getDeviceQueue();

	fprintf(gpFile, "********************************************************************************************\n");






	///////////Swapchain
	vkResult = createSwapchain(VK_FALSE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createSwapchain() Function is %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createSwapchain() Function is Succeded");
	}



	//create vulkan images and vulkan imageviews
	vkResult = createImagesAndImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createImagesAndImageViews() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createImagesAndImageViews() Function is Succeded");
	}


	//create command pool
	vkResult = createCommandPool();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createCommandPool() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createCommandPool() Function is Succeded");
	}


	//create command Buffers
	vkResult = createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createCommandBuffers() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createCommandBuffers() Function is Succeded");
	}

	//VertexBuffer
	vkResult = createVertexBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createVertexBuffer() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createVertexBuffer() Function is Succeded\n");
	}



	//Shaders	
	vkResult = createShaders();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createShaders() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createShaders() Function is Succeded\n");
	}


	//Descriptor Set Layout
	vkResult = createDescriptorSetLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createDescriptorSetLayout() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createDescriptorSetLayout() Function is Succeded\n");
	}


	vkResult = createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createPipelinelayout() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createPipelinelayout() Function is Succeded\n");
	}


	//RenderPass
	vkResult = createRenderPass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createRenderPass() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createRenderPass() Function is Succeded");
	}
	vkResult = createPipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createPipeline() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createPipeline() Function is Succeded\n");
	}


	//FrameBuffer		
	vkResult = createFrameBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createFrameBuffer() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createFrameBuffer() Function is Succeded");
	}




	//Semaphores		
	vkResult = createSemaphores();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createSemaphores() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createSemaphores() Function is Succeded");
	}



	//Fences

	vkResult = createFences();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->createFences() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->createFences() Function is Succeded");
	}



	//initialize clear color values
	memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));			//Analogous to glClearColor();
	vkClearColorValue.float32[0] = 0.0f;  //R		
	vkClearColorValue.float32[1] = 0.0f;  //G
	vkClearColorValue.float32[2] = 1.0f;  //B
	vkClearColorValue.float32[3] = 0.0f;  //A




	// Build command buffers

	vkResult = buildComaandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nInitialize()->buildComaandBuffers() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nInitialize()->buildComaandBuffers() Function is Succeded\n");
	}


	///////////////////////////////////Initialization is completed /////////////////////////////////////

	bInitialized = TRUE;


	fprintf(gpFile, "\n*****Initialization is completed is Successfully*****\n");




	return vkResult;
}


VkResult Resize(int width, int height)
{
	//Function declarations
	VkResult createSwapchain(VkBool32);
	VkResult createImagesAndImageViews(void);
	VkResult createRenderPass(void);
	VkResult createCommandBuffers(void);
	VkResult createPipelineLayout(void);
	VkResult createPipeline(void);	
	VkResult createFrameBuffers(void);
	VkResult buildComaandBuffers(void);



	//Variable 
	VkResult vkResult = VK_SUCCESS;




	// Code
	if (height <= 0)
		height = 1;


	//Check the bInitalized variable
	if (bInitialized == FALSE)
	{
		fprintf(gpFile, "\nResize()->Initialization Yet not completed or Failed\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	//As recreation of Swapchain is needed we are going to repeat many steps of Initialized again hence set bInitialized = FALSE again
	bInitialized = FALSE;

	//Set Global winWidth and winHeight variables
	winWidth = width;
	winHeight = height;

	//Wait For Device to complete in-hand task
	if (vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);
		fprintf(gpFile, "\n vkDeviceWaitIdle() is Done\n");
	}

	//Destroy Framebuffers
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);
	}

	if (vkFrameBuffer_array)
	{
		free(vkFrameBuffer_array);
		vkFrameBuffer_array = NULL;
		fprintf(gpFile, "\nResize()->Free vkFrameBuffer_array freed\n");

	}
	//Destroy Commandbuffers
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
		fprintf(gpFile, "\nResize()->Free commandbuffers freed\n");
	}
	if (vkCommandBuffer_array)
	{
		free(vkCommandBuffer_array);
		vkCommandBuffer_array = NULL;
	}
	//Destroy Pipeline
	if (vkPipeline)
	{
		vkDestroyPipeline(vkDevice, vkPipeline, NULL);
		vkPipeline = VK_NULL_HANDLE;
		fprintf(gpFile, "\nResize()->Free vkPipeline freed\n");
	}

	//Destroy PipelineLayout
	if (vkPipelineLayout)
	{
		vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
		vkPipelineLayout = VK_NULL_HANDLE;
		fprintf(gpFile, "\nResize()->Free vkPipelineLayout freed\n");
	}
	//Destroy Renderpass
	if (vkRenderPass)
	{
		vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
		vkRenderPass = VK_NULL_HANDLE;
		fprintf(gpFile, "\nResize()->Free vkRenderPass freed\n");
	}
	

	//Destroy Images and ImageViews
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
		fprintf(gpFile, "\nResize()->Free swapchainImage_array images freed\n");
	}
	if (swapchainImageView_array)
	{
		free(swapchainImageView_array);
		swapchainImageView_array = NULL;
	}

	///Use Case for Validation (No need to destroy swapchain images, they get destroyed along with swapchain with swapchinKHR)
		//for (uint32_t i = 0; i < swapchainImageCount; i++)
		//{
		//	vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
		//	
		//	//fflush(gpFile);
		//	/**/
		//	fprintf(gpFile, "\nFree swapchainImage_array images freed\n");
		//}

	if (swapchainImage_array)
	{
		free(swapchainImage_array);
		swapchainImage_array = NULL;
	}

	//Destroy Swapchain
	if (vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
		vkSwapchainKHR = NULL;		//Bhanda swachha
		fprintf(gpFile, "\n Resize()->vkSwapchainKHR is Freed\n");

	}



	////////////////////////////////// RECREATE FOR RESIZE //////////////////////////////////////////

	//Create Swapchain
	vkResult = createSwapchain(VK_FALSE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nResize()->createSwapchain() Function is %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	
	//Create Swapchain Images and Imageviews
	//create vulkan images and vulkan imageviews
	vkResult = createImagesAndImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nResize()->createImagesAndImageViews() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	//Create Renderpass
	vkResult = createRenderPass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nResize()->createRenderPass() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	//Create PipelineLayout
	vkResult = createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nResize()->createPipelinelayout() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	//Create Pipeline
	vkResult = createPipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nResize()->createPipeline() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	//Creat Framebuffers
	vkResult = createFrameBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nResize()->createFrameBuffer() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	//Create Command Buffers
	vkResult = createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nResize()->createCommandBuffers() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	
	//BuildCommandBuffers
	vkResult = buildComaandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nResize()->buildComaandBuffers() Function is failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}















	bInitialized = TRUE;

	return vkResult;
}


VkResult Display(void)
{
	//Display() : This function will submit our work from acquired queue_selected, our work is inside command buffers and command buffers will put and push it inside queue.This is called submission
	//vkQueueSubmit() : before this vkSubmitInfo structure we will have to fill. This function will take multiple command buffers at a time


	//First USE_CASE : OUPUT CORRECT BUT LOG FILE GENERATING ERROS


	//Variable 
	VkResult vkResult = VK_SUCCESS;

	// Code
	//if control comes here before initialization gets completed, return false
	if (bInitialized == FALSE)
	{
		fprintf(gpFile, "\nDisplay()->bInitialized initialization yet not completed \n");
		return (VkResult)VK_FALSE;
	}

	//Acquire index of next swapchain image
	//UINT64_MAX : nanosec timeout timer
	//Here we are waiting for swapchain to give us image
	//If this func could not get image within timeout then it will return not ready VK_NOTREADY
	//4th param is not waiting for swapchain to return image, it is waiting for another queue 
	//swapchain's next image renerable, give me that image
	vkResult = vkAcquireNextImageKHR(vkDevice, vkSwapchainKHR, UINT64_MAX, vkSemaphore_BackBuffer, VK_NULL_HANDLE, &currentImageIndex);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n\nvkAcquireNextImageKHR()->Display Function is Failed\n");
		return vkResult;
	}

	//Use Fence to allow Host to wait for completion of execution of previous command buffer
	//if you have array of fences give the count, we will be giving 1 at a time, it has capability of multiple
	//VK_TRUE : if possible i will wait for all fences to get signaled so give me TRUE or give it FALSE (we have only 1) wait for all array to get signaled 
	//UINT64_MAX : Timeout
	vkResult = vkWaitForFences(vkDevice, 1, &vkFence_array[currentImageIndex], VK_TRUE, UINT64_MAX);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n\nvkWaitForFences()->Display Function is Failed\n");
		return vkResult;
	}

	//Now ready the Fences for execution of Next Command buffer
	vkResult = vkResetFences(vkDevice, 1, &vkFence_array[currentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n\nvkResetFences()->Display Function is Failed\n");
		return vkResult;
	}



	//one of the member of vksubmit info structures requires array of pipeline stages, we have only one of completion of color attachment output, still we ned one member array
	const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	//DECLARE MEMSET AND INITIALIZE vksubmitinfo structure
	VkSubmitInfo vkSubmitInfo;
	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));

	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
	vkSubmitInfo.waitSemaphoreCount = 1;
	vkSubmitInfo.pWaitSemaphores = &vkSemaphore_BackBuffer;
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
	vkSubmitInfo.signalSemaphoreCount = 1;
	vkSubmitInfo.pSignalSemaphores = &vkSemaphore_RenderComplete;

	//Now submit above work to the queue
	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, vkFence_array[currentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n\nvkQueueSubmit()->Display Function is Failed\n");
		return vkResult;
	}


	//we are going to present rendered image after declaring, initalizing vkpresentinfoKHR structure

	VkPresentInfoKHR vkPresentInfoKHR;
	memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

	vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	vkPresentInfoKHR.pNext = NULL;
	vkPresentInfoKHR.swapchainCount = 1;
	vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
	vkPresentInfoKHR.pImageIndices = &currentImageIndex;
	vkPresentInfoKHR.waitSemaphoreCount = 1;
	vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_RenderComplete;



	//Now present the queue
	vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n\nvkQueuePresentKHR()->Display Function is Failed\n");
		return vkResult;
	}



	//First USE_CASE : OUPUT CORRECT BUT LOG FILE GENERATING ERRORS
	vkDeviceWaitIdle(vkDevice);

	//Second USE_CASE : NO OUPUT. VALIDATION ERROR IS THERE





	return vkResult;

}


void Update(void)
{
	// Code
}


void Uninitialize(void)
{
	// Function declarations
	void ToggleFullScreen(void);

	// Code
	if (gbFullScreen == TRUE)
	{
		ToggleFullScreen();
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}


	//No need to destroy/uninitialize vkQueue


	//Vulkan related any destruction **HAS TO BE AFTER VkDevice**
	//because any resources related to vulkan device ae all done so resource freeing 

	//Destroy vulkan device
	if (vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);
		fprintf(gpFile, "\n vkDeviceWaitIdle() is Done\n");


		//Free swapchain Images

		for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroyFence(vkDevice, vkFence_array[i], NULL);
			fprintf(gpFile, "vkDestroyFence vkFence_array i = %d Completedestroyed!\n", i);
		}
		if (vkFence_array)
		{
			free(vkFence_array);
			vkFence_array = NULL;
			fprintf(gpFile, "\nFree vkFence_array freed\n");
		}

		if (vkSemaphore_RenderComplete)
		{
			vkDestroySemaphore(vkDevice, vkSemaphore_RenderComplete, NULL);
			vkSemaphore_RenderComplete = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vkSemaphore_RenderComplete freed\n");


		}
		if (vkSemaphore_BackBuffer)
		{
			vkDestroySemaphore(vkDevice, vkSemaphore_BackBuffer, NULL);
			vkSemaphore_BackBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vkSemaphore_BackBuffer freed\n");

		}

		//Framebuffer free
		for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);

		}

		if (vkFrameBuffer_array)
		{
			free(vkFrameBuffer_array);
			vkFrameBuffer_array = NULL;
			fprintf(gpFile, "\nFree commandbuffers freed\n");

		}
		if (vertexData_Position.vkDeviceMemory)
		{
			vkFreeMemory(vkDevice, vertexData_Position.vkDeviceMemory, NULL);
			vertexData_Position.vkDeviceMemory = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vertexData_Position.vkDeviceMemory freed\n");

		}
		if (vertexData_Position.vkBuffer)
		{

			vkDestroyBuffer(vkDevice, vertexData_Position.vkBuffer, NULL);
			vertexData_Position.vkBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vertexData_Position.vkBuffer freed\n");

		}

		if (vkDescriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
			vkDescriptorSetLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vkDescriptorSetLayout freed\n");
		}

		if (vkPipelineLayout)
		{
			vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
			vkPipelineLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vkPipelineLayout freed\n");

		}
		if (vkPipeline)
		{
			vkDestroyPipeline(vkDevice, vkPipeline, NULL);
			vkPipeline = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vkPipeline freed\n");

		}
		if (vkRenderPass)
		{
			vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
			vkRenderPass = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vkRenderPass freed\n");

		}
		//For Shader modules : destory
		//fragment
		if (vkShaderModule_fragment_shader)
		{
			vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
			vkShaderModule_fragment_shader = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vkShaderModule_fragment_shader freed\n");

		}

		//Vertex
		if (vkShaderModule_vertex_shader)
		{
			vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
			vkShaderModule_vertex_shader = VK_NULL_HANDLE;
			fprintf(gpFile, "\nFree vkShaderModule_vertex_shader freed\n");

		}

		for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
			//vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
			fprintf(gpFile, "\nFree commandbuffers freed\n");
		}
		if (vkCommandBuffer_array)
		{
			free(vkCommandBuffer_array);
			vkCommandBuffer_array = NULL;
		}

		vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
		//vkCommandPool = VK_NULL_HANDLE;
		fprintf(gpFile, "\n vkCommandPool is Freed\n");


		for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
			fprintf(gpFile, "\nFree swapchainImage_array images freed\n");
		}
		if (swapchainImageView_array)
		{
			free(swapchainImageView_array);
			swapchainImageView_array = NULL;
		}

		///Use Case for Validation (No need to destroy swapchain images, they get destroyed along with swapchain with swapchinKHR)
		//for (uint32_t i = 0; i < swapchainImageCount; i++)
		//{
		//	vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
		//	
		//	//fflush(gpFile);
		//	/**/
		//	fprintf(gpFile, "\nFree swapchainImage_array images freed\n");
		//}

		if (swapchainImage_array)
		{
			free(swapchainImage_array);
			swapchainImage_array = NULL;
		}




		//Destroy Fences





		//Command buffer free
		//actual array free



		//destroy 	swapchain
		if (vkSwapchainKHR)
		{
			vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
			vkSwapchainKHR = NULL;		//Bhanda swachha
			fprintf(gpFile, "\n vkSwapchainKHR is Freed\n");

		}




		vkDestroyDevice(vkDevice, NULL);
		vkDevice = VK_NULL_HANDLE;
		fprintf(gpFile, "\n vkDestroyDevice() is Done\n");

	}
	//No need to Destroy selected physical device





	if (vkSurfaceKHR)
	{
		vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
		vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "\nvkDestroySurfaceKHR Done\n");
	}

	if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
	{
		vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
		vkDebugReportCallbackEXT = VK_NULL_HANDLE;
		vkDestroyDebugReportCallbackEXT_fnptr = NULL;
		fprintf(gpFile, "\nvkDestroyDebugReportCallbackEXT_fnptr Done\n");

	}

	//uninitialize/destroy vulkan instance
	if (vkInstance)
	{
		vkDestroyInstance(vkInstance, NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "\nvkDestroyInstance Done\n");
	}


	if (gpFile)
	{
		fprintf(gpFile, "\nUninitialize->Program Terminated Successfully.\n");
		fclose(gpFile);
		gpFile = NULL;
	}




}


//////////////////////////////////////////////////////////////////////////////////// Definition of Vulkan related Functions ///////////////////////////////////////////////////////////////////////////////

VkResult createVulkanInstance(void)
{

	// Function declarations
	VkResult fillInstanceExtensionNames();

	//Functions for validation : validation lauers
	VkResult fillValidationLayerNames(void);
	VkResult createValidationCallbackFunction(void);


	VkResult vkResult = VK_SUCCESS;

	// Code
	vkResult = fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateVulkanInstance()->fillInstanceExtensionNames Function is Failed\n");
		return vkResult;
	}
	else {
		fprintf(gpFile, "\ncreateVulkanInstance()->fillInstanceExtensionNames Function is Succeded\n");
	}

	//For Validation

	if (bValidation == TRUE)
	{
		vkResult = fillValidationLayerNames();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\ncreateVulkanInstance()->fillValidationLayerNames Function is Failed\n");
			return vkResult;
		}
		else {
			fprintf(gpFile, "\ncreateVulkanInstance()->fillValidationLayerNames Function is Succeded\n");
		}
	}


	//Initialize struct VkApplicationInfo
	VkApplicationInfo vkApplicationInfo;
	memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

	vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkApplicationInfo.pNext = NULL;
	vkApplicationInfo.pApplicationName = gpszAppName;
	vkApplicationInfo.applicationVersion = 1;
	vkApplicationInfo.pEngineName = gpszAppName;
	vkApplicationInfo.engineVersion = 1;
	vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

	//INITIALIZE STRUCT
	VkInstanceCreateInfo vkInstanceCreateInfo;
	memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pNext = NULL;
	vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;
	if (bValidation == TRUE)
	{
		vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
		vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_array;
	}
	else
	{
		vkInstanceCreateInfo.enabledLayerCount = 0;
		vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
	}


	//CALL create instance by calling vkInstance
	vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpFile, "vkCreateInstance() failed due to incompatible driver %d\n", vkResult);
		return vkResult;
	}
	else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpFile, "vkCreateInstance() failed due to extension not present driver %d\n", vkResult);
		return vkResult;
	}
	else if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "vkCreateInstance() failed due to Unknown reason driver %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "vkCreateInstance() succeded");
	}

	//Do for Validation Callbacks
	if (bValidation == TRUE)
	{
		vkResult = createValidationCallbackFunction();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\ncreateVulkanInstance()->createValidationCallbackFunction Function is Failed\n");
			return vkResult;
		}
		else {
			fprintf(gpFile, "\ncreateVulkanInstance()->createValidationCallbackFunction Function is Succeded\n");
		}
	}
	return vkResult;
}


VkResult fillInstanceExtensionNames(void)
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//1.Find how many extensions are supported by vulkan driver of this version and keep it in local variable
	uint32_t instanceExtensionCount = 0;

	//First vulkan API (Enumerate means to retrieve the list of all possible options)
	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	//NULL : Layer name supported by Vulkan,  &instanceextensioncount : count or size, NULL: Extension properties array(parameterised returm value)
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "First call to fillInstanceExtensionNames()->vkEnumerateInstanceExtensionProperties Function is Failed");
		return (vkResult);
	}
	else {
		fprintf(gpFile, "First call to fillInstanceExtensionNames()->vkEnumerateInstanceExtensionProperties Function is Succeded");
	}

	//2. Allocate and Fill VkExtensionProperties struct properties corresponding to above count  
	VkExtensionProperties* vkExtensionProperties_array = NULL;
	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);			//Allocate memory for VkExxtension
	//For the sake of bravety no malloc checking code


	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "Second call to fillInstanceExtensionNames()->vkEnumerateInstanceExtensionProperties Function is Failed");
		return (vkResult);
	}
	else {
		fprintf(gpFile, "Second call to fillInstanceExtensionNames()->vkEnumerateInstanceExtensionProperties Function is Succeded");
	}


	//3. Fill and display a local string array of extension names obtained from vkExtensionProperties structure array
	char** instanceExtensionNames_array = NULL;
	instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
		fprintf(gpFile, "fillInstanceExtensionNames()->vulkanInstanceExtensionName = %s\n", instanceExtensionNames_array[i]);
	}

	//4. As not required here onward free the vkExtension properties array
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;		//bhanda swachha 


	//5.Find weather above extensions contain our required 2 extensions
	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 win32SurfaceExtensionFound = VK_FALSE;
	VkBool32 debugReportExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanSurfaceExtensionFound = VK_TRUE;
			enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;

		}

		if (strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			win32SurfaceExtensionFound = VK_TRUE;
			enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		}

		if (strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
		{
			debugReportExtensionFound = VK_TRUE;
			if (bValidation == TRUE)
			{
				enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			}
			else
			{
				//Array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
			}
		}
	}

	//6 As not needed henceforth free the local strings array
	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		free(instanceExtensionNames_array[i]);
	}
	free(instanceExtensionNames_array);


	//7.Display weather required instance extensions supported or not
	if (vulkanSurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;			//Return hardcoded failure
		fprintf(gpFile, "\nfillInstanceExtensionNames()->VK_KHR_SURFACE_EXTENSION_NAME not found");
		return (vkResult);
	}
	else {
		fprintf(gpFile, "\nfillInstanceExtensionNames()->VK_KHR_SURFACE_EXTENSION_NAME  is found");
	}


	if (win32SurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;			//Return hardcoded failure
		fprintf(gpFile, "\nfillInstanceExtensionNames()->VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found");
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "\nfillInstanceExtensionNames()->VK_KHR_WIN32_SURFACE_EXTENSION_NAME  is found");
	}

	if (debugReportExtensionFound == VK_FALSE)
	{
		if (bValidation == TRUE)
		{
			vkResult = VK_ERROR_INITIALIZATION_FAILED;			//Return hardcoded failure
			fprintf(gpFile, "\nfillInstanceExtensionNames()->Validation is ON but requierd VK_EXT_DEBUG_REPORT_EXTENSION_NAME not Supported\n");
			return (vkResult);
		}
		else
		{
			fprintf(gpFile, "\nfillInstanceExtensionNames()->Validation is OFF and VK_EXT_DEBUG_REPORT_EXTENSION_NAME  is not Supported");
		}
	}
	else
	{
		if (bValidation == TRUE)
		{
			fprintf(gpFile, "\nfillInstanceExtensionNames()->Validation is ON but requierd VK_EXT_DEBUG_REPORT_EXTENSION_NAME is Supported\n");
		}
		else
		{
			fprintf(gpFile, "\nfillInstanceExtensionNames()->Validation is OFF and VK_EXT_DEBUG_REPORT_EXTENSION_NAME  is Supported");
		}
	}


	//8. Print only enabled extension names
	for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
	{
		fprintf(gpFile, "\nfillInstanceExtensionNames()->EnabledVulkanExtensionName = %s\n", enabledInstanceExtensionNames_array[i]);
		//return (vkResult);
	}

	return (vkResult);
}





VkResult fillValidationLayerNames(void)
{

	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//Code
	uint32_t validationLayerCount = 0;


	//First vulkan API (Enumerate means to retrieve the list of all possible options)
	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
	//NULL : Layer name supported by Vulkan,  &instanceextensioncount : count or size, NULL: Extension properties array(parameterised returm value)
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nFirst call to fillValidationLayerNames()->vkEnumerateInstanceLayerProperties Function is Failed\n");
		return (vkResult);
	}
	else {
		fprintf(gpFile, "\nFirst call to fillValidationLayerNames()->vkEnumerateInstanceLayerProperties Function is Succeded\n");
	}

	//2. Allocate and Fill VkExtensionProperties struct properties corresponding to above count  

	VkLayerProperties* vkLayerProperties_array = NULL;
	vkLayerProperties_array = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * validationLayerCount);


	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
	//NULL : Layer name supported by Vulkan,  &instanceextensioncount : count or size, NULL: Extension properties array(parameterised returm value)
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nSecond call to fillValidationLayerNames()->vkEnumerateInstanceLayerProperties Function is Failed\n");
		return (vkResult);
	}
	else {
		fprintf(gpFile, "\nSecond call to fillValidationLayerNames()->vkEnumerateInstanceLayerProperties Function is Succeded\n");
	}


	char** validationLayerNames_array = NULL;
	validationLayerNames_array = (char**)malloc(sizeof(char*) * validationLayerCount);
	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		validationLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
		memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
		fprintf(gpFile, "\nfillValidationLayerNames()->	 = %s\n", validationLayerNames_array[i]);
	}


	free(vkLayerProperties_array);
	vkLayerProperties_array = NULL;		//bhanda swachha 



	//5.Find weather above validation contain our required 1 extensions
	VkBool32 validationLayerFound = VK_FALSE;

	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		if (strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
		{
			validationLayerFound = VK_TRUE;
			enabledValidationLayerNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
		}
	}

	//6 As not needed henceforth free the local strings array
	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		free(validationLayerNames_array[i]);
	}
	free(validationLayerNames_array);
	validationLayerNames_array = NULL;

	if (validationLayerFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;			//Return hardcoded failure
		fprintf(gpFile, "\nfillValidationLayerNames()->VK_LAYER_KHRONOS_validation not Supported");
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "\nfillValidationLayerNames()->VK_LAYER_KHRONOS_validation Supported");

	}

	for (uint32_t i = 0; i < enabledValidationLayerCount; i++)
	{
		fprintf(gpFile, "\nfillValidationLayerNames()->EnabledVulkanValidationLayerName = %s\n", enabledValidationLayerNames_array[i]);
		//return (vkResult);
	}


	return vkResult;



}








VkResult createValidationCallbackFunction(void)
{

	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//Code

	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*);

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

	// Get the required function pointers
	vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
	if (vkCreateDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s() :vkGetInstanceProcAddr: failed to get function pointer for vkCreateDebugReportCallbackEXT_fnptr.\n", __FUNCTION__);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "%s() :vkGetInstanceProcAddr: succeeded to get function pointer for vkCreateDebugReportCallbackEXT_fnptr.\n", __FUNCTION__);
	}

	vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
	if (vkDestroyDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s() :vkGetInstanceProcAddr: failed to get function pointer for vkDestroyDebugReportCallbackEXT_fnptr.\n", __FUNCTION__);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "%s() :vkGetInstanceProcAddr: succeeded to get function pointer for vkDestroyDebugReportCallbackEXT_fnptr.\n", __FUNCTION__);
	}


	// get the vulkan debug report callback object
	VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
	memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
	vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
	vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
	vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;

	vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s() :vkCreateDebugReportCallbackEXT_fnptr failed.\n", __FUNCTION__);
	}
	else
	{
		fprintf(gpFile, "%s() :vkCreateDebugReportCallbackEXT_fnptr succeeded.\n", __FUNCTION__);
	}


	return vkResult;


}



////presentation surface

VkResult getSupportedSurface(void)
{

	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;

	memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

	vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkWin32SurfaceCreateInfoKHR.pNext = NULL;
	vkWin32SurfaceCreateInfoKHR.flags = 0;
	vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
	vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

	vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getSupportedSurface()->vkCreateWin32SurfaceKHR Function is Failed");
		return vkResult;
	}
	else {
		fprintf(gpFile, "getSupportedSurface()->vkCreateWin32SurfaceKHR Function is Succeded");
	}

	return vkResult;

}


///Physical Device
VkResult getPhysicalDevice()
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//2. Call to vkEnumeratePhysicalDevices() to get physical device count
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDevice() - vkEnumeratePhysicalDevices() First call failedis Failed %d\n", vkResult);
		return vkResult;
	}
	else if (physicalDeviceCount == 0)
	{
		fprintf(gpFile, "getPhysicalDevice() - vkEnumeratePhysicalDevices resulted in 0 physicalDevices \n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDevice() - vkEnumeratePhysicalDevices()  First call succeeded\n");
	}


	//3. Allocate vkPhysicalDevice_array an array according to above count


	vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
	//error checking for malloc should be done

	//  4. Call vkEnumeratePhysicalDevices() again to fill above array
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDevice() - vkEnumeratePhysicalDevices() Second call failed is Failed %d\n", vkResult);
		return(vkResult);
	}
	else
	{
		fprintf(gpFile, "getPhysicalDevice() - vkEnumeratePhysicalDevices() Second call succeeded\n");
	}


	//5 Start a loop using Physical Device count and physical device array
	VkBool32 bFound = VK_FALSE;
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		//a
		uint32_t queueCount = UINT32_MAX;

		//b
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);

		//c
		VkQueueFamilyProperties* vkQueFamilyProperties_array = NULL;
		vkQueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);


		//vkResult = vkEnumeratePhysicalDevices(vkInstance, &queueCount, vkQueFamilyProperties_array);


		//d
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueFamilyProperties_array);

		//e
		VkBool32* isQueueSurfaceSupported_array = NULL;
		isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * queueCount);
		//f
		for (uint32_t j = 0; j < queueCount; j++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);
		}
		//g
		fprintf(gpFile, "\n================================= Queue Family Count = %u =================================\n", queueCount);


		for (uint32_t j = 0; j < queueCount; j++)
		{
			if (vkQueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (isQueueSurfaceSupported_array[j] == VK_TRUE)
				{
					vkPhysicalDevice_Selected = vkPhysicalDevice_array[i];
					graphicsQueueFamilyIndex_Selected = j;
					bFound = VK_TRUE;
					break;
				}
			}
		}
		//h

		if (isQueueSurfaceSupported_array)
		{
			free(isQueueSurfaceSupported_array);
			isQueueSurfaceSupported_array = NULL;
			fprintf(gpFile, "\ngetPhysicalDevice()-> isQueueSurfaceSupported_array Succeded to Free()");
		}
		if (vkQueFamilyProperties_array)
		{
			free(vkQueFamilyProperties_array);
			vkQueFamilyProperties_array = NULL;
			fprintf(gpFile, "\ngetPhysicalDevice()-> vkQueFamilyProperties_array Succeded to Free()");
		}
		if (bFound == VK_TRUE)
		{
			break;
		}
	} // Main for loop


	if (bFound == VK_TRUE)
	{
		fprintf(gpFile, "getPhysicalDevice() is Succeeded to select the required physical device with graphics enabled\n");
	}
	else
	{
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
		fprintf(gpFile, "getPhysicalDevice() failed to select the required physical device with graphics enabled\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkResult);
	}

	//7
	memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

	//8
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_Selected, &vkPhysicalDeviceMemoryProperties);

	//9
	VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
	memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

	vkGetPhysicalDeviceFeatures(vkPhysicalDevice_Selected, &vkPhysicalDeviceFeatures);



	if (vkPhysicalDeviceFeatures.tessellationShader)
	{
		fprintf(gpFile, "\n getPhysicalDevice()-> vkPhysicalDeviceFeatures is supports tessellation shader");
	}
	else
	{
		fprintf(gpFile, "\n getPhysicalDevice()-> vkPhysicalDeviceFeatures is not supported tessellation shader");
	}
	if (vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(gpFile, "\n getPhysicalDevice()-> vkPhysicalDeviceFeatures is supports geometryShader shader");
	}
	else
	{
		fprintf(gpFile, "\n getPhysicalDevice()-> vkPhysicalDeviceFeatures is not supportsB geometryShader shader");
	}
	return vkResult;
}

VkResult printVKInfo(void)
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//Code
	fprintf(gpFile, "\n*********************************** Vulkan Information*********************************************\n");

	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		VkPhysicalDeviceProperties vkPhysicalDeviceproperties;
		memset((void*)&vkPhysicalDeviceproperties, 0, sizeof(VkPhysicalDeviceProperties));
		vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceproperties);

		uint32_t majorVersion = VK_VERSION_MAJOR(vkPhysicalDeviceproperties.apiVersion);
		uint32_t minorVersion = VK_VERSION_MINOR(vkPhysicalDeviceproperties.apiVersion);
		uint32_t patchVersion = VK_VERSION_PATCH(vkPhysicalDeviceproperties.apiVersion);

		//API Version
		fprintf(gpFile, "\n API Version = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);

		//Device Name
		fprintf(gpFile, "\n Device Name = %s\n", vkPhysicalDeviceproperties.deviceName);

		//Device Type
	//	fprintf(gpFile, "\n Device Name = %s\n", vkPhysicalDeviceproperties.deviceType);
		//
		switch (vkPhysicalDeviceproperties.deviceType)
		{

		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			fprintf(gpFile, "\n Device Type = Integrated GPU(iGPU)\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			fprintf(gpFile, "\n Device Type = Discrete GPU(dGPU)\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			fprintf(gpFile, "\n Device Type = Virtual GPU(vGPU)\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			fprintf(gpFile, "\n Device Type = CPU \n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			fprintf(gpFile, "\n Device Type = Other\n");
			break;


		default:
			fprintf(gpFile, "\n Device Type = UNKNOWN\n");
			break;
		}

		//vendor id
		fprintf(gpFile, "\n Vendor ID  = 0x%04x\n", vkPhysicalDeviceproperties.vendorID);


		//device ID
		fprintf(gpFile, "\n Device ID  = 0x%04x\n", vkPhysicalDeviceproperties.deviceID);

	}

	//free global physical device array
	if (vkPhysicalDevice_array)
	{
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
		fprintf(gpFile, "getPhysicalDevice() Succeeded to free vkPhysicalDevice_array\n");
	}




	return vkResult;


}


/////////// Device Extension Names




VkResult fillDeviceExtensionNames(void)
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//1.Find how many extensions are supported by vulkan driver of this version and keep it in local variable
	uint32_t deviceExtensionCount = 0;

	//First vulkan API
	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_Selected, NULL, &deviceExtensionCount, NULL);
	//NULL : Layer name supported by Vulkan,  &instanceextensioncount : count or size, NULL: Extension properties array(parameterised returm value)
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "First call to fillDeviceExtensionNames()->vkEnumerateInstanceExtensionProperties Function is Failedis Failed %d\n", vkResult);
		return (vkResult);
	}
	else {
		fprintf(gpFile, "First call to fillDeviceExtensionNames()->vkEnumerateInstanceExtensionProperties Function is Succeded");
	}

	//2. Allocate and Fill VkExtensionProperties struct properties corresponding to above count  
	VkExtensionProperties* vkExtensionProperties_array = NULL;
	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);			//Allocate memory for VkExxtension
	//For the sake of bravety no malloc checking code


	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_Selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "Second call to fillDeviceExtensionNames()->vkEnumerateInstanceExtensionProperties Function is Failedis Failed %d\n", vkResult);
		return (vkResult);
	}
	else {
		fprintf(gpFile, "Second call to fillDeviceExtensionNames()->vkEnumerateInstanceExtensionProperties Function is Succeded");
	}


	//3. Fill and display a local string array of extension names obtained from vkExtensionProperties structure array
	char** deviceExtensionNames_array = NULL;
	deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
		memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
		fprintf(gpFile, "fillDeviceExtensionNames()->vulkanDeviceExtensionName = %s\n", deviceExtensionNames_array[i]);
	}
	fprintf(gpFile, "\n\n%s() : Device Extension Count = %u\n", __FUNCTION__, deviceExtensionCount);

	//4. As not required here onward free the vkExtension properties array
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;		//bhanda swachha 


	//5.Find weather above extensions contain our required 1 extensions
	VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;


	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if (strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			vulkanSwapChainExtensionFound = VK_TRUE;
			enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}
	}

	//6 As not needed henceforth free the local strings array
	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		free(deviceExtensionNames_array[i]);
	}
	free(deviceExtensionNames_array);


	//7.Display weather required instance extensions supported or not
	if (vulkanSwapChainExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;			//Return hardcoded failure
		fprintf(gpFile, "fillDeviceExtensionNames()->VK_KHR_SWAPCHAIN_EXTENSION_NAME not found");
		return (vkResult);
	}
	else {
		fprintf(gpFile, "fillDeviceExtensionNames()->VK_KHR_SWAPCHAIN_EXTENSION_NAME  is found");
	}




	//8. Print only enabled extension names
	for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
	{
		fprintf(gpFile, "\n\nfillDeviceExtensionNames()->EnabledVulkanDeviceExtensionName = %s\n", enabledDeviceExtensionNames_array[i]);
		return (vkResult);
	}


	return (vkResult);
}



VkResult createVulkanDevice(void)
{

	////////Function declaration
	// Function declarations
	VkResult fillDeviceExtensionNames();



	// Variable declarations
	VkResult vkResult = VK_SUCCESS;
	vkResult = fillDeviceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVulkanDevice()->fillDeviceExtensionNames Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVulkanDevice()->fillDeviceExtensionNames Function is Succeded\n");
	}

	//Newly Added
	float queuePriorities[1];
	queuePriorities[0] = 1.0f;
	//float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
	memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.pNext = NULL;
	vkDeviceQueueCreateInfo.flags = 0;
	vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;
	vkDeviceQueueCreateInfo.queueCount = 1;
	vkDeviceQueueCreateInfo.pQueuePriorities = &queuePriorities;



	//3. Initialize VkDeviceCreateInfo structure

	VkDeviceCreateInfo vkDeviceCreateInfo;
	memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pNext = NULL;
	vkDeviceCreateInfo.flags = 0;
	vkDeviceCreateInfo.queueCreateInfoCount = 1;
	vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo; //newly added
	vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
	vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
	vkDeviceCreateInfo.enabledLayerCount = 0;
	vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
	vkDeviceCreateInfo.pEnabledFeatures = NULL;



	//5 Now call vkCreateDevice() vulkan api to create actually vulkan device
	vkResult = vkCreateDevice(vkPhysicalDevice_Selected, &vkDeviceCreateInfo, NULL, &vkDevice);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVulkanDevice()->vkCreateDevice Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createVulkanDevice()->vkCreateDevice Function is Succeded\n");
	}



	return vkResult;

}




void getDeviceQueue(void)
{

	//Code
	vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_Selected, 0, &vkQueue);
	//logical channels 
	//queues are internally HW GPU cores are channels
	//its software representation means devicequeue
	//0 : queue index in the family
	if (vkQueue == VK_NULL_HANDLE)
	{
		fprintf(gpFile, "\ngetDeviceQueue()->vkGetDeviceQueue Function returned NULL for vkQueue\n ");
		return;
	}
	else
	{
		fprintf(gpFile, "\ngetDeviceQueue()->vkGetDeviceQueue Function Succeded vkQueue\n ");

	}


	//void return value means all param are correct





}


VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{

	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//Code
	//get the count of supported color surface color VkFormats
	uint32_t formatCount = 0;

	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &formatCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace()->vkGetPhysicalDeviceSurfaceFormatsKHR Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else if (formatCount == 0)
	{
		fprintf(gpFile, "vkGetPhysicalDeviceSurfaceFormatsKHR() failed due to no surface format count not present driver %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace()->vkGetPhysicalDeviceSurfaceFormatsKHR Function is Succeded\n");
	}


	//declare and allocate vkSurfaceFormatKHR Array
	VkSurfaceFormatKHR* vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));


	//filling the array
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace()->vkGetPhysicalDeviceSurfaceFormatsKHR second Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace()->vkGetPhysicalDeviceSurfaceFormatsKHR second Function is Succeded\n");
	}

	//Decide the surface color format first
	if (formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
	{
		vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		vkFormat_color = vkSurfaceFormatKHR_array[0].format;
	}

	//Decide the color space
	vkColorSpacrKHR = vkSurfaceFormatKHR_array[0].colorSpace;

	//free the array
	if (vkSurfaceFormatKHR_array)
	{
		free(vkSurfaceFormatKHR_array);
		vkSurfaceFormatKHR_array = NULL;
		fprintf(gpFile, "vkSurfaceFormatKHR_array is Free\n");
	}

	return vkResult;
}




VkResult getPhysicalDevicePresentMode(void)
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	uint32_t presentModeCount = 0;

	//First call
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &presentModeCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode()->vkGetPhysicalDeviceSurfacePresentModesKHR first Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else if (presentModeCount == 0)
	{
		fprintf(gpFile, "vkGetPhysicalDeviceSurfacePresentModesKHR() failed due to no surface format count not present driver %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode()->vkGetPhysicalDeviceSurfacePresentModesKHR first Function is Succeded\n");
	}


	VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));


	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode()->vkGetPhysicalDeviceSurfacePresentModesKHR second Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "getPhysicalDevicePresentMode()->vkGetPhysicalDeviceSurfacePresentModesKHR second Function is Succeded\n");
	}



	//Decide the presentation Mode
	for (uint32_t i = 0; i < presentModeCount; i++)
	{
		if (vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			fprintf(gpFile, "MAILBOX\n", vkPresentModeKHR_array[i]);

			vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}


	}

	if (vkPresentModeKHR_array != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		fprintf(gpFile, "NOT A MAILBOX\n", vkPresentModeKHR_array);

		vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
	}

	//FREE
	if (vkPresentModeKHR_array)
	{
		free(vkPresentModeKHR_array);
		vkPresentModeKHR_array = NULL;
		fprintf(gpFile, "vkPresentModeKHR_array is Free\n");

	}

	return vkResult;


}



VkResult createSwapchain(VkBool32 vsync)
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	///Function Declaration
	VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);

	VkResult getPhysicalDevicePresentMode(void);



	///////////// Code Color format and color space

	//Step 1
	vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSwapchain()->getPhysicalDeviceSurfaceFormatAndColorSpace Function is %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\ncreateSwapchain()->getPhysicalDeviceSurfaceFormatAndColorSpace Function is Succeded");
	}

	//Step 2 GetPhysicalDeviceSurfaceCapabilities()
	//structure declare
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSwapchain()->vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Function is %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\ncreateSwapchain()->vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Function is Succeded");
	}

	///Step 3 minImageCount and maxImageCount : Find out desired number of swapchain image count
	uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
	uint32_t desiredNumberOfSwapchainImages = 0;

	if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
	{
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
		fprintf(gpFile, "\nmaxImageCount\n");

	}
	else
	{
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
		fprintf(gpFile, "\nminImageCount\n");

	}

	fprintf(gpFile, "\n\n\n!!!%d %d %d !!!\n", vkSurfaceCapabilitiesKHR.minImageCount, vkSurfaceCapabilitiesKHR.maxImageCount, desiredNumberOfSwapchainImages);


	///Step 4 choose size of the swapchain image

	memset((void*)&vkExtent2D_Swapchain, 0, sizeof(VkExtent2D));
	if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
	{
		vkExtent2D_Swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		vkExtent2D_Swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
		fprintf(gpFile, "\nIn IF createSwapchain()->SwapchainImage WidthxHeight = %d x %d\n", vkSurfaceCapabilitiesKHR.currentExtent.width, vkSurfaceCapabilitiesKHR.currentExtent.height);
	}
	else
	{
		//if surfacesize is already defined thn swapchainimage size muct match with it
		VkExtent2D vkExtent2D;
		memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

		vkExtent2D.width = (uint32_t)winWidth;
		vkExtent2D.height = (uint32_t)winHeight;

		vkExtent2D_Swapchain.width = max(vkSurfaceCapabilitiesKHR.minImageExtent.width, min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
		vkExtent2D_Swapchain.height = max(vkSurfaceCapabilitiesKHR.minImageExtent.height, min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
		fprintf(gpFile, "\ncreateSwapchain()->SwapchainImage Width x Height = %d x %d\n", vkSurfaceCapabilitiesKHR.currentExtent.width, vkSurfaceCapabilitiesKHR.currentExtent.height);

	}

	//Step 5 Set Swapchain image usage Flag
	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	//Step 6 whether to consider pre-transform/flipping or not
	VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;		//this is enum so no memset()

	if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
	}

	/////////Step 7 Call Presentation Mode
	vkResult = getPhysicalDevicePresentMode();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSwapchain()->getPhysicalDevicePresentMode Function is %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\ncreateSwapchain()->getPhysicalDevicePresentMode Function is Succeded");
	}


	//Step 8 now fill the structure initialize VkCreateInfoSwapchain structure
	VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
	memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));

	vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapchainCreateInfoKHR.pNext = NULL;
	vkSwapchainCreateInfoKHR.flags = 0;
	vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
	vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
	vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
	vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpacrKHR;
	vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_Swapchain.width;
	vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_Swapchain.height;
	vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
	vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
	vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
	vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
	vkSwapchainCreateInfoKHR.clipped = VK_TRUE;


	////STEP 9
	vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSwapchain()->vkCreateSwapchainKHR Function is %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\ncreateSwapchain()->vkCreateSwapchainKHR Function is Succeded");
	}





	return vkResult;



}

VkResult createImagesAndImageViews(void)
{

	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	///code

	//Get swapchain image count
	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateImagesAndImageViews()->vkGetSwapchainImagesKHR Function is Failed %d\n", vkResult);
		return vkResult;
	}
	else if (swapchainImageCount == 0)
	{
		fprintf(gpFile, "\ncreateImagesAndImageViews() failed due to no count not present driver %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\n\ncreateImagesAndImageViews()->vkGetSwapchainImagesKHR gives swapchain (desired) image count = %d\n", swapchainImageCount);
	}


	//Step 2 : allocate the swapchin image array
	swapchainImage_array = (VkImage*)malloc(sizeof(VkImage) * swapchainImageCount);

	// fill this array by swapchain images
	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s() : First call to vkGetSwapchainImagesKHR Second Call failed.\n", __FUNCTION__);
	}
	else
	{
		fprintf(gpFile, "%s() : First call to vkGetSwapchainImagesKHR Second Call succeeded.\n", __FUNCTION__);
	}

	// allocate array of swapchain image views
	swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount);

	//step5:initialize vkimageview createinfo structure

	VkImageViewCreateInfo vkImageViewCreateInfo;
	memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	vkImageViewCreateInfo.format = vkFormat_color;
	vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //WHICH PART OF IMAGE OR WHOLE OF IMAGE IS GOING TO BE AFFECTED BY IMAGE BARRIER
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;		//How much mipmap level so start from 0th index level
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;			//dont know level count so atleast 1 is expected
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;		//if image is composite then any layer is also image then it is called as layered rendering and it is image arrays layer
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;			//minimum layer is 1
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;			//ENUM OF vk_image_view_type_2D

	//Step 6 : Now fill imageview_array using above struct
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkImageViewCreateInfo.image = swapchainImage_array[i];

		vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\ncreateImagesAndImageViews()->vkCreateImageViews failed for iteration %d.%d\n", i, vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\ncreateImagesAndImageViews()->vkCreateImageViews Function is Succeded");
		}

	}


	return vkResult;


}


VkResult createCommandPool(void)
{
	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
	memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

	vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext = NULL;
	vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//fLAG INDICATES THAT CREATE SUCH COMMAND POOLS WHICH CAN BE CAPABALE OF RESETTED AND RESTARTED.THESE COMMAND BUFERS ARE LONG LIVED
	vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;

	vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\createCommandPool()->vkCreateCommandPool failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\createCommandPool()->vkCreateCommandPool Function is Succeded");
	}

	return vkResult;

}



VkResult createCommandBuffers(void)
{

	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code

	//vkcommandbuffer initialization of structure
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));


	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;		//PRIMARY COMMAND BUFFER CAN BE SUBMITTED,  PRIMARY BUFFERS CAN BE CALLED WITHIN PRIMARY COMMAND BUFFERS
	vkCommandBufferAllocateInfo.commandBufferCount = 1;

	//Step2
	vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);


	//Step2 Allocate command buffers
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\createCommandBuffers()->vkAllocateCommandBuffers failed for Function is %d\n", vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\createCommandBuffers()->vkAllocateCommandBuffers Function is Succeded");
		}


	}



	return vkResult;


}

VkResult createVertexBuffer(void)
{

	// Variable declarations
	VkResult vkResult = VK_SUCCESS;

	//Code
	float traingle_position[] =
	{
		0.0f,1.0f,0.0f,
		-1.0f,-1.0f,0.0f,
		1.0f,-1.0f,0.0f
	};

	//2
	memset((void*)&vertexData_Position, 0, sizeof(VertexData));

	//3
	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0;				//valid flags are used in scattered buffer(sparse buffer)
	vkBufferCreateInfo.size = sizeof(traingle_position);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	//MEMBERS: SHARINGMODE,QUEUEFAMILYINDEX_COUNT,QUEUESFAMILYINDICES: 
	//Values of members: zero (exclusive means one queue) , zero,NULL
	//In Vulkan memory management is not done in Bytes but it is done in regions and the max number of regions (vkDevicePhysicalProperties : min 4096), though this number looks smalls, it deliberate because vulkan demands/recommends using small number of large sized allocations and use them repetively for different resources
	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_Position.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkCreateBuffer() failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkCreateBuffer() Function is Succeded\n");
	}


	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));


	vkGetBufferMemoryRequirements(vkDevice, vertexData_Position.vkBuffer, &vkMemoryRequirements);
	//No error checking


	//8
	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0;			//Initial value before entering into the loop

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}


	//9

	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_Position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkAllocateMemory failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkAllocateMemory Function is Succeded\n");
	}


	//10
	vkResult = vkBindBufferMemory(vkDevice, vertexData_Position.vkBuffer, vertexData_Position.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkBindBufferMemory failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkBindBufferMemory Function is Succeded\n");
	}

	//Map memory
	void* data = NULL;
	vkResult = vkMapMemory(vkDevice, vertexData_Position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkMapMemory failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkMapMemory Function is Succeded\n");
	}


	memcpy(data, traingle_position, sizeof(traingle_position));

	//vkunmap memory
	vkUnmapMemory(vkDevice, vertexData_Position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkUnmapMemory failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateVertexBuffer()->vkUnmapMemory Function is Succeded\n");
	}








	return vkResult;


}



VkResult createShaders(void)
{
	VkResult vkResult = VK_SUCCESS;

	//code
	// 
	//For Vertex Shader
	const char* szFileName = "Shader.vert.spv";
	FILE* fp = NULL;
	size_t size;

	fp = fopen(szFileName, "rb");  //read binary format
	if (fp == NULL)
	{
		fprintf(gpFile, "\ncreateShaders()-> Fail to Open Spirv Vertex Shader file\n\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateShaders()-> Succeded to Open Spirv Vertex Shader file\n\n");
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	if (size == 0)
	{
		fprintf(gpFile, "\ncreateShaders()-> gave  Spirv Vertex Shader size 0 \n\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateShaders()-> gave  Spirv Vertex Shader size more than 0 \n\n");

	}
	fseek(fp, 0L, SEEK_SET);

	char* shaderData = (char*)malloc(sizeof(char) * size);
	size_t retVal = fread(shaderData, size, 1, fp);
	if (retVal != 1)
	{
		fprintf(gpFile, "\ncreateShaders()-> Fail to Read the Spirv Vertex Shader file\n\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	fclose(fp);


	VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0;				//reserved hence must be 0 (for future use)
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;


	vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateShaders()->vkCreateShaderModule failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateShaders()->vkCreateShaderModule Function is Succeded");
	}


	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
		fprintf(gpFile, "\ncreateShaders()-> Vertex Shader Module is Succeded");

	}


	///For Framgment Shader

	szFileName = "Shader.frag.spv";
	fp = NULL;
	size = 0;

	fp = fopen(szFileName, "rb");  //read binary format
	if (fp == NULL)
	{
		fprintf(gpFile, "\ncreateShaders()-> Fail to Open Spirv Fragment Shader file\n\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateShaders()-> Succeded to Open Spirv Fragment Shader file\n\n");
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	if (size == 0)
	{
		fprintf(gpFile, "\ncreateShaders()-> gave  Spirv Fragment Shader size 0 \n\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateShaders()-> gave  Spirv Fragment Shader size more than 0 \n\n");

	}
	fseek(fp, 0L, SEEK_SET);

	shaderData = (char*)malloc(sizeof(char) * size);
	retVal = fread(shaderData, size, 1, fp);
	if (retVal != 1)
	{
		fprintf(gpFile, "\ncreateShaders()-> Fail to Read the Spirv Fragment Shader file\n\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	fclose(fp);


	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0;				//reserved hence must be 0 (for future use)
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;


	vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateShaders()->vkCreateShaderModule failed for fragment Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateShaders()->vkCreateShaderModule Function for fragment is Succeded");
	}


	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
		fprintf(gpFile, "\ncreateShaders()-> Fragment Shader Module is Succeded");

	}




	return vkResult;

}




VkResult createDescriptorSetLayout(void)
{
	VkResult vkResult = VK_SUCCESS;


	//struct
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
	memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0;
	vkDescriptorSetLayoutCreateInfo.bindingCount = 0;
	vkDescriptorSetLayoutCreateInfo.pBindings = NULL;				//OPENGL VAO, VBO first binding points is here pBindings

	//unit32_t binding :  an integer value where you want to bind the descriptor or the descriptor set

		//vkDescriptorSetType vkDescriptorSetType


	vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateDescriptorSetLayout()->vkCreateDescriptorSetLayout() failed for fragment Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateDescriptorSetLayout()->vkCreateDescriptorSetLayout() Function for fragment is Succeded");
	}





	return vkResult;

}




VkResult createPipelineLayout(void)
{
	VkResult vkResult = VK_SUCCESS;


	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));


	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.flags = 0;		//RESERVED
	vkPipelineLayoutCreateInfo.setLayoutCount = 1;		//WE HAVE DESCRIPTOR SET LAYOUT SO 1
	vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreatePipelineLayout()->vkCreatePipelineLayout() failed for fragment Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreatePipelineLayout()->vkCreatePipelineLayout() Function for fragment is Succeded");
	}





	return vkResult;


}




















VkResult createRenderPass(void)
{

	VkResult vkResult = VK_SUCCESS;

	VkAttachmentDescription vkAttachmentDescription_array[1];

	memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));
	vkAttachmentDescription_array[0].flags = 0;
	vkAttachmentDescription_array[0].format = vkFormat_color;
	vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;   //No multi sampling so 1 bit is enough
	vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;				//color attachment related
	vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			//This is for both Depth and Stencil although it is for stencil
	vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;													//Image data when in and when out
	vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;




	//Declare and initialize vkAttachmentReference structure
	VkAttachmentReference vkAttachmentRederence;
	memset((void*)&vkAttachmentRederence, 0, sizeof(VkAttachmentReference));
	vkAttachmentRederence.attachment = 0;			//This means above given array 0th Ataachment reference, O means it is the index number
	vkAttachmentRederence.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		//Tis means this attachment i can use it color attachment so keep it optimal


	//Step 3 : Declare and Initialize vkSubpassDescription

	VkSubpassDescription vkSubpassDesciption;
	memset((void*)&vkSubpassDesciption, 0, sizeof(VkSubpassDescription));
	vkSubpassDesciption.flags = 0;
	vkSubpassDesciption.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkSubpassDesciption.inputAttachmentCount = 0;
	vkSubpassDesciption.pInputAttachments = NULL;
	vkSubpassDesciption.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
	vkSubpassDesciption.pColorAttachments = &vkAttachmentRederence;
	vkSubpassDesciption.pResolveAttachments = NULL;
	vkSubpassDesciption.pDepthStencilAttachment = NULL;
	vkSubpassDesciption.preserveAttachmentCount = 0;
	vkSubpassDesciption.pPreserveAttachments = NULL;

	//Step 4: Declare and initialize vkrenderpass create info structure
	VkRenderPassCreateInfo vkRenderPassCreateInfo;
	memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
	vkRenderPassCreateInfo.flags = 0;
	vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderPassCreateInfo.pNext = NULL;
	vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
	vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
	vkRenderPassCreateInfo.subpassCount = 1;
	vkRenderPassCreateInfo.pSubpasses = &vkSubpassDesciption;
	vkRenderPassCreateInfo.pDependencies = NULL;

	//Create Renderpass
	vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\createRenderPass()->vkCreateRenderPass failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\createRenderPass()->vkCreateRenderPass Function is Succeded");
	}


	return vkResult;



}












VkResult createPipeline(void)
{


	VkResult vkResult = VK_SUCCESS;

	//Now structure is equivalent to vbo and attribute in OpenGL
	////1.vertexInputstate

	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];

	memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

	vkVertexInputBindingDescription_array[0].binding = 0; //GL_ARRAY_BUFFER chya buffer chya array madhe 0 the index
	vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; 
	////
	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
	memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

	
	vkVertexInputAttributeDescription_array[0].location = 0; //layout location = 0 in shader (should be same)
	vkVertexInputAttributeDescription_array[0].binding = 0;
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vkVertexInputAttributeDescription_array[0].offset = 0; //interleave chya veli vaprat yeil
	fprintf(gpFile, "\nvkVertexInputAttributeDescription_array\n", vkResult);
	fflush(gpFile);


	VkPipelineVertexInputStateCreateInfo  vkPipelineVertexInputStateCreateInfo;
	memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

	vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkPipelineVertexInputStateCreateInfo.flags = 0;
	vkPipelineVertexInputStateCreateInfo.pNext = NULL;
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;
	fprintf(gpFile, "\nvkPipelineVertexInputStateCreateInfo\n", vkResult);
	fflush(gpFile);

	//2. Input Assembly State
	VkPipelineInputAssemblyStateCreateInfo  vkPipelineInputAssemblyStateCreateInfo;
	memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

	vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
	vkPipelineInputAssemblyStateCreateInfo.flags = 0;
	vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	///primitiveRestartEnable : dont restart primitive, used mainly in geometry shader 
	fprintf(gpFile, "\nvkPipelineInputAssemblyStateCreateInfo\n", vkResult);
	fflush(gpFile);


	//3. Rasterization/rasterizer State
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
	memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

	vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vkPipelineRasterizationStateCreateInfo.pNext = NULL;
	vkPipelineRasterizationStateCreateInfo.flags = 0;
	vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;			//WINDING/DRAWING ORDER OF TRIANGLE VERTICES , so we are giving clockwise
	vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0;			///Line width is implementation dependant, 0 means Nullify our drawing, atlast one line widht we need to give

	fprintf(gpFile, "\nvkPipelineRasterizationStateCreateInfo\n", vkResult);
	fflush(gpFile);

	//4. Color Blend State
	//No blending required here, off it
	VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
	memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

	vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_G_BIT;
	//vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0Xf;

	fprintf(gpFile, "\nvkPipelineColorBlendAttachmentState_array\n", vkResult);
	fflush(gpFile);

	VkPipelineColorBlendStateCreateInfo  vkPipelineColorBlendStateCreateInfo;
	memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

	vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkPipelineColorBlendStateCreateInfo.flags = 0;
	vkPipelineColorBlendStateCreateInfo.pNext = NULL;
	vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

	fprintf(gpFile, "\nvkPipelineColorBlendStateCreateInfo\n", vkResult);
	fflush(gpFile);

	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
	memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));


	vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkPipelineViewportStateCreateInfo.flags = 0;
	vkPipelineViewportStateCreateInfo.pNext = NULL;
	vkPipelineViewportStateCreateInfo.viewportCount = 1;


	fprintf(gpFile, "\nvkPipelineViewportStateCreateInfo\n", vkResult);
	fflush(gpFile);
	memset((void*)&vkViewport, 0, sizeof(VkViewport));
	vkViewport.x = 0;
	vkViewport.y = 0;
	vkViewport.width = (float)vkExtent2D_Swapchain.width;
	vkViewport.height = (float)vkExtent2D_Swapchain.height;
	vkViewport.maxDepth = 1.0;
	vkViewport.minDepth = 0.0;

	vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
	vkPipelineViewportStateCreateInfo.scissorCount = 1;

	memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));

	vkRect2D_scissor.offset.x = 0;
	vkRect2D_scissor.offset.y = 0;
	vkRect2D_scissor.extent.width = vkExtent2D_Swapchain.width;
	vkRect2D_scissor.extent.height = vkExtent2D_Swapchain.height;

	vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor;
	fprintf(gpFile, "\nvkPipelineViewportStateCreateInfo\n", vkResult);
	fflush(gpFile);
	//5.Depth Stencil State : As we dont have depth yet, we can omit this state
	//Dymanically create dynamic state : viewport, scissors, blend constants, stencil mask, line width, depth bias etc 
	//Dynamic state : we dont have any dynamic state

	//6. Multi-sampling state 
	VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
	memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

	vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vkPipelineMultisampleStateCreateInfo.pNext = NULL;
	vkPipelineMultisampleStateCreateInfo.flags = 0;
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	fprintf(gpFile, "\nvvkPipelineMultisampleStateCreateInfo\n", vkResult);
	fflush(gpFile);

	//7. Shader Stage
	//Need to creat array because we have VS and FS, either it is equals to 2 or upto 5

	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
	memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

	//VERTEX SHADER
	vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[0].flags = 0;
	vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;
	vkPipelineShaderStageCreateInfo_array[0].pName = "main";
	vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;


	//FRAGMENT SHADER
	vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[1].flags = 0;
	vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;
	vkPipelineShaderStageCreateInfo_array[1].pName = "main";
	vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;

	//Tessellation stage 
	//We dont have tessellation shaders so we can omit this stage
	fprintf(gpFile, "\nvkPipelineShaderStageCreateInfo_array\n", vkResult);
	fflush(gpFile);

	//As pipeline are created from pipeline cache, now we will create pipeline create object

	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));


	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0;
	fprintf(gpFile, "\nvkPipelineCacheCreateInfo\n", vkResult);
	fflush(gpFile);
	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
	vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreatePipeline()->vkCreatePipelineCache failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreatePipeline()->vkCreatePipelineCache Function is Succeded\n");
	}



	/////////////////////////////////////////////////////////////////////////////
	//Create the Actual Graphics Pipeline

	VkGraphicsPipelineCreateInfo 	vkGraphicsPipelineCreateInfo;
	memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

	vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = NULL;
	vkGraphicsPipelineCreateInfo.flags = 0;
	vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL;
	vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
	vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
	vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
	vkGraphicsPipelineCreateInfo.pStages = &vkPipelineShaderStageCreateInfo_array;
	vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
	vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
	vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;
	vkGraphicsPipelineCreateInfo.subpass = 0;
	vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;




	////Now Create the pipeline

	vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreatePipeline()->vkCreateGraphicsPipelines failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreatePipeline()->vkCreateGraphicsPipelines Function is Succeded\n");
	}




	//WE HAVE DONE WITH PIPELINE CACHE SO DESTROY

	vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
	vkPipelineCache = VK_NULL_HANDLE;





	return vkResult;

}

















VkResult createFrameBuffers(void)
{

	VkResult vkResult = VK_SUCCESS;


	//Step 1:Array of vkImageview
	VkImageView vkImageView_attachments_array[1];
	memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

	//Step 2:declare and initialize vkFraebuffer
	VkFramebufferCreateInfo vkFramebufferCreateInfo;
	memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

	vkFramebufferCreateInfo.flags = 0;
	vkFramebufferCreateInfo.pNext = NULL;
	vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	vkFramebufferCreateInfo.renderPass = vkRenderPass;
	vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
	vkFramebufferCreateInfo.pAttachments = vkImageView_attachments_array;
	vkFramebufferCreateInfo.width = vkExtent2D_Swapchain.width;
	vkFramebufferCreateInfo.height = vkExtent2D_Swapchain.height;
	vkFramebufferCreateInfo.layers = 1;		//layered Rendering : 256 Layers is Guaranteed, 2048 layers for composite


	vkFrameBuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount);
	//Loop

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkImageView_attachments_array[0] = swapchainImageView_array[i];
		vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFrameBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\ncreateFrameBuffers()->vkCreateFramebuffer failed for Function is %d\n", vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\ncreateFrameBuffers()->vkCreateFramebuffer Function is Succeded");
		}


	}

	return vkResult;


}


VkResult createSemaphores(void)
{
	VkResult vkResult = VK_SUCCESS;

	//Code
	VkSemaphoreCreateInfo vkSempahoreCreateInfo;
	memset((void*)&vkSempahoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

	vkSempahoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSempahoreCreateInfo.pNext = NULL;
	vkSempahoreCreateInfo.flags = 0;			//Must be 0 because its reserved


	vkResult = vkCreateSemaphore(vkDevice, &vkSempahoreCreateInfo, NULL, &vkSemaphore_BackBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSemaphores()->vkCreateSemaphore() first failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateSemaphores()->vkCreateSemaphore() first Function is Succeded\n");
	}


	vkResult = vkCreateSemaphore(vkDevice, &vkSempahoreCreateInfo, NULL, &vkSemaphore_RenderComplete);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSemaphores()->vkCreateSemaphore() second failed for Function is %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\ncreateSemaphores()->vkCreateSemaphore() second Function is Succeded\n");
	}




	return vkResult;

}








VkResult createFences(void)
{
	VkResult vkResult = VK_SUCCESS;

	//code
	//Declare memset and initialize
	VkFenceCreateInfo vkFenceCreateInfo;
	memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

	vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceCreateInfo.pNext = NULL;
	vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;		//It is green signaled to go ahead(0 means unsignalled and means wait)



	vkFence_array = (VkFence*)malloc(sizeof(VkFence) * swapchainImageCount);
	//Loop
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\ncreateFences()->vkCreateFence() failed for Function is %d\n", vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\ncreateFences()->vkCreateFence() Function is Succeded\n");
		}
	}




	return vkResult;

}



VkResult buildComaandBuffers(void)
{

	VkResult vkResult = VK_SUCCESS;

	//code
	//loop per swapchainimage

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		//reset command buffers

		vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);		//0 means created command buffers from command pool, dont release the resources created by command pool for this command buffers
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\nbuildComaandBuffers()->vkResetCommandBuffer() failed for Function is %d\n", vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nbuildComaandBuffers()->vkResetCommandBuffer() Function is Succeded\n");
		}


		//Fill the structure

		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
		memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = 0;
		vkCommandBufferBeginInfo.flags = NULL;


		//call function
		vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\nbuildComaandBuffers()->vkBeginCommandBuffer() failed for Function is %d\n", vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nbuildComaandBuffers()->vkBeginCommandBuffer() Function is Succeded\n");
		}


		//set clear values
		VkClearValue vkClearValue_array[1];
		memset((void*)vkClearValue_array, 0, sizeof(VkClearColorValue) * _ARRAYSIZE(vkClearValue_array));

		vkClearValue_array[0].color = vkClearColorValue;


		//Strcture fill of renderpass begin
		VkRenderPassBeginInfo vkRenderPassBegininfo;
		memset((void*)&vkRenderPassBegininfo, 0, sizeof(VkRenderPassBeginInfo));

		vkRenderPassBegininfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkRenderPassBegininfo.pNext = NULL;
		vkRenderPassBegininfo.renderPass = vkRenderPass;
		vkRenderPassBegininfo.renderArea.offset.x = 0;
		vkRenderPassBegininfo.renderArea.offset.y = 0;
		vkRenderPassBegininfo.renderArea.extent.height = vkExtent2D_Swapchain.height;
		vkRenderPassBegininfo.renderArea.extent.width = vkExtent2D_Swapchain.width;


		vkRenderPassBegininfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
		vkRenderPassBegininfo.pClearValues = vkClearValue_array;
		vkRenderPassBegininfo.framebuffer = vkFrameBuffer_array[i];

		//This fun is void so no error checking
		vkCmdBeginRenderPass(vkCommandBuffer_array[i], &vkRenderPassBegininfo, VK_SUBPASS_CONTENTS_INLINE);


		//Here we should call vulkan drawing functions
		vkCmdBindPipeline(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

		//bind with Vertex Buffer
		/*// Provided by VK_VERSION_1_0
typedef uint64_t VkDeviceSize;*/
		VkDeviceSize vkDeviceSize_offset_array[1];
		memset((void*)vkDeviceSize_offset_array, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkCommandBuffer_array));

		/*// Provided by VK_VERSION_1_0
void vkCmdBindVertexBuffers(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    firstBinding,
	uint32_t                                    bindingCount,
	const VkBuffer*                             pBuffers,
	const VkDeviceSize*                         pOffsets);*/ //offset is if multiple buffer che binding point astil, 
		vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 0, 1, &vertexData_Position.vkBuffer, vkDeviceSize_offset_array);

		//**************Here we should call Vulkan Drawing functions******************
		/*// Provided by VK_VERSION_1_0
void vkCmdDraw(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    vertexCount,
	uint32_t                                    instanceCount,
	uint32_t                                    firstVertex,
	uint32_t                                    firstInstance);*/
		vkCmdDraw(vkCommandBuffer_array[i], 3, 1, 0, 0);

		//End Renderpass
		vkCmdEndRenderPass(vkCommandBuffer_array[i]);


		//End command buffer recording
		vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\nbuildComaandBuffers()->vkEndCommandBuffer() failed for Function is %d\n", vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nbuildComaandBuffers()->vkEndCommandBuffer() Function is Succeded\n");
		}
	}	//End the command loop







	return vkResult;

}








VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{

	//Code

	fprintf(gpFile, "\n\nSAB_Validation : debugReportCallback()->%s  (%d)  =  %s  \n\n", pLayerPrefix, messageCode, pMessage);


	return VK_FALSE;

}





















































































































































































































