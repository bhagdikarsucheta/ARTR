
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

//VK_KHR_SURFACE_EXTENSION_NAME and VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[2];

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
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_Swapchain;


//Swapchain Image and Swapchain ImageView related variables
uint32_t swapchainImageCount = UINT32_MAX;
VkImage* swapchainImage_array = NULL;
VkImageView* swapchainImageView_array = NULL;


// Entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function declarations
	VkResult Initialize(void);
	void Display(void);
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
		fprintf(gpFile, "WinMain()->Initilize Function is Failed");
		DestroyWindow(hwnd);
		hwnd = NULL;;
	}
	else {
		fprintf(gpFile, "WinMain()->Initilize Function is Succeded");
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
				Display();

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
	void Resize(int, int);
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
	VkResult  createVulkanInstance(void);

	VkResult getSupportedSurface(void);

	VkResult getPhysicalDevice(void);

	VkResult printVKInfo(void);

	VkResult createVulkanDevice(void);

	void getDeviceQueue(void);

	VkResult createSwapchain(VkBool32);

	VkResult createImagesAndImageViews(void);




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



	return vkResult;
}


void Resize(int width, int height)
{
	// Code
	if (height == 0)
		height = 1;
}


void Display(void)
{
	// Code
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
			vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
			fprintf(gpFile, "\nFree swapchainImage_array images freed\n");
		}

		if (swapchainImageView_array)
		{
			free(swapchainImageView_array);
			swapchainImageView_array = NULL;
		}


		/*for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
			fprintf(gpFile, "\nFree swapchainImage_array images freed\n");
		}*/

		if (swapchainImage_array)
		{
			free(swapchainImage_array);
			swapchainImage_array = NULL;
		}

		//destroy 	swapchain
		if (vkSwapchainKHR)
		{
			vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
			vkSwapchainKHR = VK_NULL_HANDLE;		//Bhanda swachha
			fprintf(gpFile, "\n vkSwapchainKHR is Done\n");

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
		fprintf(gpFile, "vkDestroySurfaceKHR Succeded\n");
	}

	//uninitialize/destroy vulkan instance
	if (vkInstance)
	{
		vkDestroyInstance(vkInstance, NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "vkDestroyInstance Succeded\n");
	}


	if (gpFile)
	{
		fprintf(gpFile, "Uninitialize->Program Terminated Successfully.\n");
		fclose(gpFile);
		gpFile = NULL;
	}




}


//////////////////////////////////////////////////////////////////////////////////// Definition of Vulkan related Functions ///////////////////////////////////////////////////////////////////////////////

VkResult createVulkanInstance(void)
{

	// Function declarations
	VkResult fillInstanceExtensionNames();

	VkResult vkResult = VK_SUCCESS;

	// Code
	vkResult = fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVulkanInstance()->fillInstanceExtensionNames Function is Failed");
		return vkResult;
	}
	else {
		fprintf(gpFile, "createVulkanInstance()->fillInstanceExtensionNames Function is Succeded");
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
		fprintf(gpFile, "fillInstanceExtensionNames()->VK_KHR_SURFACE_EXTENSION_NAME not found");
		return (vkResult);
	}
	else {
		fprintf(gpFile, "fillInstanceExtensionNames()->VK_KHR_SURFACE_EXTENSION_NAME  is found");
	}


	if (win32SurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;			//Return hardcoded failure
		fprintf(gpFile, "fillInstanceExtensionNames()->VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found");
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "fillInstanceExtensionNames()->VK_KHR_WIN32_SURFACE_EXTENSION_NAME  is found");
	}

	//8. Print only enabled extension names
	for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
	{
		fprintf(gpFile, "fillInstanceExtensionNames()->EnabledVulkanExtensionName = %s\n", enabledInstanceExtensionNames_array[i]);
		return (vkResult);
	}

	return (vkResult);
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
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
	memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.pNext = NULL;
	vkDeviceQueueCreateInfo.flags = 0;
	vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;
	vkDeviceQueueCreateInfo.queueCount = 1;
	vkDeviceQueueCreateInfo.pQueuePriorities = &queuePriority;



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
	swapchainImage_array = (VkImage*)malloc(swapchainImageCount * sizeof(VkImage));

	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateImagesAndImageViews()->vkGetSwapchainImagesKHR Function is %d\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\ncreateImagesAndImageViews()->vkGetSwapchainImagesKHR Function is Succeded");
	}


	//step 3 : allocate array of swapchain image view
	swapchainImageView_array = (VkImageView*)malloc(swapchainImageCount * sizeof(VkImageView));

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





















































































































































