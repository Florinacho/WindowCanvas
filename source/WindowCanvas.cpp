#include "WindowCanvas.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>

#if defined(_DEBUG)
#define WC_INFO(...)     fprintf(stdout, __VA_ARGS__)
#define WC_WARNING(...)  WC_INFO(__VA_ARGS__)
#define WC_ERROR(...)    fprintf(stderr, __VA_ARGS__)
#else
#define WC_INFO(...)     /* EMPTY */
#define WC_WARNING(...)  /* EMPTY */
#define WC_ERROR(...)    /* EMPTY */
#endif

#if defined(__linux__)
/*****************************************************************************/
/** Linux - X11                                                              */
/*****************************************************************************/
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#ifdef XDestroyImage
#undef XDestroyImage
#endif

#define X11_LIB_NAME "libX11.so"

#define X11_PROC_LIST \
	X11_PROC(XOpenDisplay) \
	X11_PROC(XCloseDisplay) \
	X11_PROC(XCreateGC) \
	X11_PROC(XFreeGC) \
	X11_PROC(XCreateSimpleWindow) \
	X11_PROC(XDestroyWindow) \
	X11_PROC(XStoreName) \
	X11_PROC(XSelectInput) \
	X11_PROC(XGetWMNormalHints) \
	X11_PROC(XSetWMNormalHints) \
	X11_PROC(XMapRaised) \
	X11_PROC(XPending) \
	X11_PROC(XSendEvent) \
	X11_PROC(XNextEvent) \
	X11_PROC(XLookupString) \
	X11_PROC(XCreateImage) \
	X11_PROC(XPutImage) \
	X11_PROC(XDestroyImage) \
	X11_PROC(XInternAtom) \
	X11_PROC(XSetWMProtocols) \
	/* EMPTY_LINE */

struct X11 {
	// X11 function pointers
	typedef Display* (*PFN_XOpenDisplay)(char*);
	typedef void     (*PFN_XCloseDisplay)(Display*);
	typedef Window   (*PFN_XCreateSimpleWindow)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, unsigned long, unsigned long);
	typedef int      (*PFN_XDestroyWindow)(Display*, Window);
	typedef int      (*PFN_XStoreName)(Display *display, Window w, char *window_name); 
	typedef int      (*PFN_XSelectInput)(Display*, Window, long);
	typedef int      (*PFN_XMapRaised)(Display*, Window);
	typedef int      (*PFN_XPending)(Display*);
	typedef Status   (*PFN_XSendEvent)(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send); 
	typedef int      (*PFN_XNextEvent)(Display*, XEvent*); 
	typedef int      (*PFN_XLookupString)(XKeyEvent*, char*, int, KeySym*, XComposeStatus*);
	typedef Status   (*PFN_XGetWMNormalHints)(Display*, Window, XSizeHints*, long*);
	typedef void     (*PFN_XSetWMNormalHints)(Display*, Window, XSizeHints*);
	typedef GC       (*PFN_XCreateGC)(Display*, Drawable, unsigned long, XGCValues*);
	typedef int      (*PFN_XFreeGC)(Display*, GC);
	typedef XImage*  (*PFN_XCreateImage)(Display*, Visual*, unsigned int, int, int, char*, unsigned int, unsigned int, int, int);
	typedef int      (*PFN_XPutImage)(Display*, Drawable, GC, XImage*, int, int, int, int, unsigned int, unsigned int); 
	typedef int      (*PFN_XDestroyImage)(XImage*);
	typedef Atom     (*PFN_XInternAtom)(Display *display, const char *atom_name, Bool only_if_exists); 
	typedef Status   (*PFN_XSetWMProtocols)(Display *display, Window w, Atom *protocols, int count); 

	void* handle;

	// Declare X11 functions
	#define X11_PROC(name) PFN_##name name;
	X11_PROC_LIST
	#undef X11_PROC

	X11() 
		: handle(nullptr) {
		init();
	}

	~X11() {
		uninit();
	}

	int init(const char* filename = X11_LIB_NAME) {
		if (handle != nullptr) {
			return 0;
		}
		int count = 0;

		if ((handle = dlopen(filename, RTLD_LAZY)) == nullptr) {
			WC_ERROR("Cannot open library '%s'.\n", filename);
			return 1;
		}
		WC_INFO("Opened dynamic library '%s', at %p.\n", filename, handle);

		#define X11_PROC(name) \
		if ((name = (PFN_##name)dlsym(handle, #name)) == nullptr) {\
			WC_ERROR("Failed to load " #name "\n"); \
			uninit(); \
			return 1;\
		} else {\
			WC_INFO("Loaded function '%s', at %p.\n", #name, name); \
			++count; \
		}
		X11_PROC_LIST
		#undef X11_PROC

		WC_INFO("Successfully loaded %u functions.\n", count);
		return 0;
	}

	void uninit() {
		if (handle != nullptr) {
			dlclose(handle);
			handle = nullptr;
		}
	}
};
static const X11 x11;

#undef X11_PROC_LIST

#elif defined(_WIN32)
/*****************************************************************************/
/** Windows - GDI                                                            */
/*****************************************************************************/
#include <windows.h>

#define GDI_LIB_NAME "gdi32.dll"
#define GDI_PROC_LIST \
	GDI_PROC(CreateCompatibleDC) \
	GDI_PROC(CreateDIBSection) \
	GDI_PROC(SelectObject) \
	GDI_PROC(DeleteObject) \
	GDI_PROC(ExtFloodFill) \
	GDI_PROC(BitBlt) \
	/* Empty line */
	
/*
	// WIN32 API
	WIN32_PROC(RegisterClass) \
	WIN32_PROC(SetWindowLongPtr) \
	WIN32_PROC(GetWindowLongPtr) \
	WIN32_PROC(DefWindowProc) \
	WIN32_PROC(GetKeyboardState) \
	WIN32_PROC(ToAscii) \
	WIN32_PROC(AdjustWindowRect) \
	WIN32_PROC(CreateWindowEx) \
	WIN32_PROC(GetDC) \
	
	typedef ATOM     (*PFN_RegisterClass)(const WNDCLASSA *lpWndClass);
	typedef LONG_PTR (*PFN_SetWindowLongPtr)(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
	typedef LONG_PTR (*PFN_GetWindowLongPtr)(HWND hWnd, int nIndex);
	typedef LRESULT  (*PFN_DefWindowProc)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	typedef BOOL     (*PFN_GetKeyboardState)(PBYTE lpKeyState);
	typedef int      (*PFN_ToAscii)(UINT uVirtKey, UINT uScanCode, const BYTE *lpKeyState, LPWORD lpChar, UINT uFlags);
	typedef BOOL     (*PFN_AdjustWindowRect)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu);
	typedef HWND     (*PFN_CreateWindowEx)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	typedef HDC      (*PFN_GetDC)(HWND hWnd);
*/

struct GDI {
	typedef HDC      (__stdcall *PFN_CreateCompatibleDC)(HDC hdc);
	typedef HBITMAP  (__stdcall *PFN_CreateDIBSection)(HDC hdc, const BITMAPINFO *pbmi, UINT usage, VOID **ppvBits, HANDLE hSection, DWORD offset);
	typedef HGDIOBJ  (__stdcall *PFN_SelectObject)(HDC hdc, HGDIOBJ h);
	typedef BOOL     (__stdcall *PFN_DeleteObject)(HGDIOBJ ho);
	typedef BOOL     (__stdcall *PFN_ExtFloodFill)(HDC hdc, int x, int y, COLORREF color, UINT type);
	typedef BOOL     (__stdcall *PFN_BitBlt)(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);

	HMODULE handle;
	
	// Declare GDI functions
	#define GDI_PROC(name) PFN_##name name;
	GDI_PROC_LIST
	#undef GDI_PROC

	GDI() : handle(nullptr) {
		init();
	}

	~GDI() {
		uninit();
	}

	int init(const char* filename = GDI_LIB_NAME) {
		if (handle != nullptr) {
			return 0;
		}
		int count = 0;

		if ((handle = LoadLibrary(filename)) == nullptr) {
			WC_ERROR("Cannot open library '%s'.\n", filename);
			return 1;
		}
		WC_INFO("Opened dynamic library '%s', at %p.\n", filename, handle);

		#define GDI_PROC(name) \
		if ((name = (PFN_##name)GetProcAddress(handle, #name)) == nullptr) {\
			WC_ERROR("Failed to load " #name "\n"); \
			uninit(); \
			return 1;\
		} else {\
			WC_INFO("Loaded function '%s', at %p.\n", #name, name); \
			++count; \
		}
		GDI_PROC_LIST
		#undef GDI_PROC

		WC_INFO("Successfully loaded %u functions.\n", count);
		return 0;
	}
	
	void uninit() {
		if (handle != nullptr) {
			FreeLibrary(handle);
			handle = nullptr;
		}
	}
};

static const GDI gdi;

#undef GDI_PROC_LIST

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	WindowCanvas* window = (WindowCanvas*)GetWindowLongPtr(hwnd, GWL_USERDATA);
	if (window == nullptr) {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	WindowEvent& event = *window->eventPtr;
    static BYTE keyState[256];
	char text[8];
	switch (uMsg) {
	case WM_CLOSE:
		event.type = WindowEvent::WindowClose;
		return 0;
	case WM_MOUSEMOVE :
		event.type = WindowEvent::CursorMove;
		event.x = LOWORD(lParam);
		event.y = HIWORD(lParam);
		return 0;
	case WM_LBUTTONDOWN :
		event.type = WindowEvent::ButtonPressed;
		event.button = 1;
		return 0;
	case WM_LBUTTONUP :
		event.type = WindowEvent::ButtonReleased;
		event.button = 1;
		return 0;
	case WM_MBUTTONDOWN :
		event.type = WindowEvent::ButtonPressed;
		event.button = 2;
		return 0;
	case WM_MBUTTONUP :
		event.type = WindowEvent::ButtonReleased;
		event.button = 2;
		return 0;
	case WM_RBUTTONDOWN :
		event.type = WindowEvent::ButtonPressed;
		event.button = 3;
		return 0;
	case WM_RBUTTONUP :
		event.type = WindowEvent::ButtonReleased;
		event.button = 3;
		return 0;
	case WM_MOUSEWHEEL :
		event.type = (GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? WindowEvent::WheelUp : WindowEvent::WheelDown);
		return 0;
	case WM_KEYDOWN :
		event.type = WindowEvent::KeyPressed;
		event.keyCode = wParam;
		GetKeyboardState(keyState);
		if (ToAscii(event.keyCode, MapVirtualKey(event.keyCode, MAPVK_VK_TO_VSC), keyState, (LPWORD)text, 0) == 1) {
			switch (text[0]) {
			case 0x1B : // escape
			case 0x08 : // backspace
			case 0x7F : // delete
				event.ascii = '\0';
				break;
			default :
				event.ascii = text[0];
				break;
			}
		} else {
			event.ascii = '\0';
		}
		return 0;
	case WM_KEYUP :
		event.type = WindowEvent::KeyReleased;
		event.keyCode = wParam;
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#else
/*****************************************************************************/
/** Unknown platform                                                         */
/*****************************************************************************/
	#error "Unknown platform"
#endif

/******************************************************************************/
/** Window specific code                                                      */
/******************************************************************************/
int WindowCanvas::initialize(uint32_t width, uint32_t height, uint8_t depth, const char* title) {
#if defined(_WIN32)
	const DWORD dwstyle = WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX | WS_VISIBLE;

	WNDCLASS wc = {};
	wc.lpfnWndProc   = WindowProc;
	wc.hInstance     = GetModuleHandle(nullptr);
	wc.lpszClassName = "Sample Window Class";
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	if (RegisterClass(&wc) == 0) {
		WC_ERROR("Failed to register class.\n");
		return 1;
	}

	RECT r = {0, 0, (LONG)width, (LONG)height};
	if (AdjustWindowRect(&r, dwstyle, false) == 0) {
		WC_WARNING("Failed to adjust window bounds.\n");
	}

	hwnd = CreateWindow(wc.lpszClassName, 
	                    title,
	                    dwstyle, 
	                    0, 0, r.right - r.left, r.bottom - r.top,
	                    nullptr,
	                    nullptr,
	                    wc.hInstance,
	                    (LPVOID)nullptr);
	if (hwnd == nullptr) {
		WC_ERROR("Failed to create window.\n");
		return 2;
	}

	SetLastError(0);
	if (SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)this) == 0) {
		if (GetLastError() != 0) {
			WC_ERROR("Failed to set window long pointer.\n");
			return 3;
		}
	}

	if ((hdc = GetDC(hwnd)) == 0) {
		WC_ERROR("Failed to retrieve the device context.\n");
		return 4;
	}

	if ((hDCMem = gdi.CreateCompatibleDC(hdc)) == nullptr) {
		WC_ERROR("Failed to create compatible device context.\n");
		return 5;
	}

	BITMAPINFO bitmapinfo = {};
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biWidth = width;
	bitmapinfo.bmiHeader.biHeight = -height;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = depth;

	pixelBufferLength = width * height * depth / 8;
	if ((bitmap = gdi.CreateDIBSection(hDCMem, &bitmapinfo, DIB_RGB_COLORS, (VOID**)&pixelBuffer, nullptr, 0)) == nullptr) {
		WC_ERROR("Failed to create bitmap.\n");
		return 6;
	}

	oldBitmap = gdi.SelectObject(hDCMem, bitmap);
	
	ShowWindow(hwnd, SW_SHOW);
	WC_INFO("Successfully created WIN32 window %ux%u.\n", width, height);
#else // __linux__
	static const uint32_t DEFAULT_MARGIN = 5;
	if ((display = x11.XOpenDisplay(nullptr)) == nullptr) {
		WC_ERROR("Failed to connect X server.\n");
		return 1;
	}
	if ((window = x11.XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, width, height, DEFAULT_MARGIN, 0, 0)) == None) {
		WC_ERROR("Failed to create simple window.\n");
		return 2;
	}
	x11.XStoreName(display, window, (char*)title);
	x11.XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyReleaseMask | KeyPressMask | PointerMotionMask);

	XSizeHints sizeHints;
	memset(&sizeHints, 0, sizeof(sizeHints));
	if (x11.XGetWMNormalHints(display, window, &sizeHints, nullptr) == 0) {
		WC_WARNING("Failed to get normal hints.\n");
	}
	sizeHints.min_width = width;
	sizeHints.max_width = width;
	sizeHints.min_height = height;
	sizeHints.max_height = height;
	x11.XSetWMNormalHints(display, window, &sizeHints);

    wm_delete_window = x11.XInternAtom(display, "WM_DELETE_WINDOW", False);
    x11.XSetWMProtocols(display, window, &wm_delete_window, 1);
	x11.XMapRaised(display, window);

	gc = x11.XCreateGC(display, window, 0, 0);

	pixelBufferLength = width * height * depth / 8;
    pixelBuffer = (uint8_t*)malloc(pixelBufferLength);
	if ((xImage = x11.XCreateImage(display, DefaultVisual(display, 0), 24, ZPixmap, 0, (char*)pixelBuffer, width, height, depth, 0)) == nullptr) {
		WC_ERROR("Failed to create xImage.\n");
		return 3;
	}
	WC_INFO("Successfully created X11 window %ux%u.\n", width, height);
#endif
	return 0;
}

int WindowCanvas::uninitialize() {
#if defined(_WIN32)
	if (hDCMem) {
		if (oldBitmap) {
			gdi.SelectObject(hDCMem, oldBitmap);
		}
		if (bitmap) {
			gdi.DeleteObject(bitmap);
		}
		if (pixelBuffer) {
			delete [] pixelBuffer;
		}
		gdi.DeleteObject(hDCMem);
	}
	if (hwnd) {
		ReleaseDC(hwnd, hdc);
		DestroyWindow(hwnd);
	}
#else // __linux__
	if (xImage != nullptr) {
		x11.XDestroyImage(xImage);
		xImage = nullptr;
	}
	if (display != nullptr) {
		x11.XFreeGC(display, gc);
		x11.XDestroyWindow(display, window);
		x11.XCloseDisplay(display);
		display = nullptr;
	}
#endif
	return 0;
}

WindowCanvas::WindowCanvas(uint32_t width, uint32_t height, uint8_t depth, const char* title) 
	: width(width), height(height), depth(depth), pixelBuffer(nullptr), pixelBufferLength(0)
#if defined(__linux__)
	, display(nullptr), window(0), gc(0), xImage(nullptr)
#elif defined (_WIN32)
	, hwnd(0), hdc(0), hDCMem(0), bitmap(0), oldBitmap(0), eventPtr(nullptr)
#endif
{
	initialize(width, height, depth, title);
}

WindowCanvas::~WindowCanvas() {
	uninitialize();
}

uint32_t WindowCanvas::getWidth() const {
	return width;
}

uint32_t WindowCanvas::getHeight() const {
	return height;
}

uint32_t WindowCanvas::getDepth() const {
	return depth;
}

void WindowCanvas::setTitle(const char* title) {
#if defined(_WIN32)
	SetWindowText(hwnd, title);
#else // __linux__
	x11.XStoreName(display, window, (char*)title);
#endif
}

const char* WindowCanvas::getTitle() const {
#if defined(_WIN32)
	static char title[128];
	return (GetWindowText(hwnd, title, sizeof(title)) > 0) ? title : "";
#else // __linux__
	assert(!"Not implemented");
#endif
	return "";
}

uint8_t* WindowCanvas::getPixelBuffer() const {
	return pixelBuffer;
}

uint32_t WindowCanvas::getPixelBufferLength() const {
	return pixelBufferLength;
}
#include <stdio.h>
bool WindowCanvas::getEvent(WindowEvent& event) {
	bool ans = false;
#if defined(_WIN32)
    MSG msg;
	if (PeekMessage(&msg, hwnd, 0, 0, 0)) {
		GetMessage(&msg, hwnd, 0, 0);
		eventPtr = &event;
		eventPtr->type = WindowEvent::Unknown;
		
        TranslateMessage(&msg);
        DispatchMessage(&msg);
		
		ans = (eventPtr->type != WindowEvent::Unknown);
    }
#else // __linux__
	XEvent xEvent;
	KeySym key;
	char text[32];
	if (x11.XPending(display) > 0) {
		x11.XNextEvent(display, &xEvent);
		switch (xEvent.type) {
		case ClientMessage:
			if((Atom)xEvent.xclient.data.l[0] == wm_delete_window) {
				event.type = WindowEvent::WindowClose;
				ans = true;
			}
			break;
		case KeyPress :
			event.type = WindowEvent::KeyPressed;
			event.keyCode = xEvent.xkey.keycode;
			if (x11.XLookupString(&xEvent.xkey, text, sizeof(text), &key, 0) == 1) {
				switch (text[0]) {
				case 0x1B : // escape
				case 0x08 : // backspace
				case 0x7F : // delete
					event.ascii = '\0';
					break;
				default :
					event.ascii = text[0];
					break;
				}
			} else {
				event.ascii = '\0';
			}
			ans = true;
			break;
		case KeyRelease :
			event.type = WindowEvent::KeyReleased;
			event.keyCode = xEvent.xkey.keycode;
			ans = true;
			break;
		case MotionNotify :
			event.type = WindowEvent::CursorMove;
			event.x = xEvent.xmotion.x;
			event.y = xEvent.xmotion.y;
			ans = true;
			break;
		case ButtonPress:
			switch (xEvent.xbutton.button) {
			case Button4 :
				event.type = WindowEvent::WheelUp;
				break;
			case Button5 :
				event.type = WindowEvent::WheelDown;
				break;
			default :
				event.type = WindowEvent::ButtonPressed;
				event.button = xEvent.xbutton.button;
				break;
			}
			ans = true;
			break;
		case ButtonRelease:
			switch (xEvent.xbutton.button) {
			case Button4 :
			case Button5 :
				break;
			default :
				event.type = WindowEvent::ButtonReleased;
				event.button = xEvent.xbutton.button;
				ans = true;
				break;
			}
			break;
		}
	}
#endif
	return ans;
}

void WindowCanvas::clear() {
#if defined(_WIN32)
	// gdi.ExtFloodFill(hdc, 0, 0, RGB(0, 0, 0), FLOODFILLSURFACE);
#endif
	memset(pixelBuffer, 0, pixelBufferLength);
}

void WindowCanvas::blit() {
#if defined(_WIN32)
	gdi.BitBlt(hdc, 0, 0, width, height, hDCMem, 0, 0, SRCCOPY);
#else // __linux__
	x11.XPutImage(display, window, gc, xImage, 0, 0, 0, 0, width, height);
#endif
}