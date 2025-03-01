
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


// Entry point fi=unction
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
	VkResult fillInstanceExtensionNames();

	VkResult  createVulkanInstance(void);


	// Variable declarations
	VkResult vkResult = VK_SUCCESS;
	//VkResult vkResult = VK_SUCCESS;

	// Code
	vkResult = fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "Initialize()->fillInstanceExtensionNames Function is Failed");
		return vkResult;
	}
	else {
		fprintf(gpFile, "Initialize()->fillInstanceExtensionNames Function is Succeded");
	}


	

	//Variable declarations
	

	vkResult = createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "Initialize()->createVulkanInstance Function is Failed");
		return vkResult;
	}
	else {
		fprintf(gpFile, "Initialize()->createVulkanInstance Function is Succeded");
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
	//uninitialize/destroy vulkan instance
	if(vkInstance)
	{
		vkDestroyInstance(vkInstance,NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "VkDestroyInstance Succeded\n");
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
	memset((void*)&vkApplicationInfo, 0,sizeof(VkApplicationInfo));

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
	vkResult = vkCreateInstance(&vkInstanceCreateInfo,NULL,&vkInstance);
	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpFile, "vkCreateInstance() failed due to incompatible driver %d\n",vkResult);
		return vkResult;
	}
	else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpFile, "vkCreateInstance() failed due to extension not present driver %d\n", vkResult);
		return vkResult;
	}
	else if(vkResult != VK_SUCCESS)
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

	//First vulkan API
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
	VkExtensionProperties * vkExtensionProperties_array = NULL;
	vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties)*instanceExtensionCount);			//Allocate memory for VkExxtension
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
	char ** instanceExtensionNames_array = NULL;
	instanceExtensionNames_array = (char**)malloc(sizeof(char*)*instanceExtensionCount);
	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		instanceExtensionNames_array[i] = (char*)malloc(sizeof(char)*strlen(vkExtensionProperties_array[i].extensionName) + 1);
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
		fprintf(gpFile, "fillInstanceExtensionNames()->vulkanExtensionName = %s\n", instanceExtensionNames_array[i]);
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



















