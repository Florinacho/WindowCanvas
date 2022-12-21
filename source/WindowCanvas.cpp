#include "WindowCanvas.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>

#if defined(_DEBUG)
#define ERROR(...) fprintf(stderr, __VA_ARGS__)
#define INFO(...) fprintf(stdout, __VA_ARGS__)
#else
#define ERROR(...)
#define INFO(...)
#endif

/******************************************************************************/
/** Platform specific code                                                    */
/******************************************************************************/
#if defined(__linux__)
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#ifdef XDestroyImage
#undef XDestroyImage
#endif

#define X11_PROC_LIST \
	X11_PROC(XOpenDisplay) \
	X11_PROC(XCloseDisplay) \
	X11_PROC(XCreateGC) \
	X11_PROC(XFreeGC) \
	X11_PROC(XCreateSimpleWindow) \
	X11_PROC(XDestroyWindow) \
	X11_PROC(XSelectInput) \
	X11_PROC(XGetWMNormalHints) \
	X11_PROC(XSetWMNormalHints) \
	X11_PROC(XMapRaised) \
	X11_PROC(XPending) \
	X11_PROC(XNextEvent) \
	X11_PROC(XLookupString) \
	X11_PROC(XCreateImage) \
	X11_PROC(XPutImage) \
	X11_PROC(XDestroyImage) \
	/* EMPTY_LINE */

struct X11 {
	// X11 function pointers
	typedef Display* (*PFN_XOpenDisplay)(char*);
	typedef void     (*PFN_XCloseDisplay)(Display*);
	typedef Window   (*PFN_XCreateSimpleWindow)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, unsigned long, unsigned long);
	typedef int      (*PFN_XDestroyWindow)(Display*, Window);
	typedef int      (*PFN_XSelectInput)(Display*, Window, long);
	typedef int      (*PFN_XMapRaised)(Display*, Window);
	typedef int      (*PFN_XPending)(Display*);
	typedef int      (*PFN_XNextEvent)(Display*, XEvent*); 
	typedef int      (*PFN_XLookupString)(XKeyEvent*, char*, int, KeySym*, XComposeStatus*);
	typedef Status   (*PFN_XGetWMNormalHints)(Display*, Window, XSizeHints*, long*);
	typedef void     (*PFN_XSetWMNormalHints)(Display*, Window, XSizeHints*);
	typedef GC       (*PFN_XCreateGC)(Display*, Drawable, unsigned long, XGCValues*);
	typedef int      (*PFN_XFreeGC)(Display*, GC);
	typedef XImage*  (*PFN_XCreateImage)(Display*, Visual*, unsigned int, int, int, char*, unsigned int, unsigned int, int, int);
	typedef int      (*PFN_XPutImage)(Display*, Drawable, GC, XImage*, int, int, int, int, unsigned int, unsigned int); 
	typedef int      (*PFN_XDestroyImage)(XImage*);

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

	int init(const char* filename = "libX11.so") {
		if (handle != nullptr) {
			return 0;
		}
		int count = 0;

		if ((handle = dlopen(filename, RTLD_LAZY)) == nullptr) {
			ERROR("Cannot open library '%s'.\n", filename);
			return 1;
		}
		INFO("Opened dynamic library '%s', at %p.\n", filename, handle);

		#define X11_PROC(name) \
		if ((name = (PFN_##name)dlsym(handle, #name)) == nullptr) {\
			ERROR("Failed to load " #name "\n"); \
			uninit(); \
			return 1;\
		} else {\
			INFO("Loaded function '%s', at %p.\n", #name, name); \
			++count; \
		}
		X11_PROC_LIST
		#undef X11_PROC

		INFO("Successfully loaded %u functions.\n", count);
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
#include <windows.h>
#error Not implemented

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
		if (ToAscii(keyCode, MapVirtualKey(keyCode, MAPVK_VK_TO_VSC), keyState, (LPWORD)text, 0) == 1) {
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
	#error "Unknown platform"
#endif

/******************************************************************************/
/** Window specific code                                                      */
/******************************************************************************/
int WindowCanvas::initialize(uint32_t width, uint32_t height, uint8_t depth, const char* title) {
#if defined(_WIN32)
	// Register the window class.
	const DWORD dwstyle = WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX | WS_VISIBLE;

	WNDCLASS wc = { };
	wc.lpfnWndProc   = WindowProc;
	wc.hInstance     = GetModuleHandle(nullptr);
	wc.lpszClassName = "Sample Window Class";
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);

	RECT r = {0, 0, (LONG)width, (LONG)height};
	AdjustWindowRect(&r, dwstyle, false);

	// Create the window.
	hwnd = CreateWindow(wc.lpszClassName, 
	                    "Win32 imGUI example", 
	                    dwstyle, 
	                    0, 0, r.right - r.left, r.bottom - r.top,
	                    nullptr,
	                    nullptr,
	                    wc.hInstance,
	                    nullptr);
	if (hwnd == nullptr) {
		return 1;
	}
	SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)this);

	// Create bitmap
	hdc = GetDC(hwnd);
	if (hdc == 0) {
		return 2;
	}
	hDCMem = CreateCompatibleDC(hdc);

	BITMAPINFO bitmapinfo = {0};
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biWidth = width;
	bitmapinfo.bmiHeader.biHeight = -height;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = depth;

	pixelBufferLength = width * height * depth / 8;
	bitmap = ::CreateDIBSection(hDCMem, &bitmapinfo, DIB_RGB_COLORS, (VOID**)&pixelBuffer, nullptr, 0);
	oldBitmap = ::SelectObject(hDCMem, bitmap);
#else // __linux__
	static const uint32_t DEFAULT_MARGIN = 5;
	display = x11.XOpenDisplay(nullptr);
	Visual *visual = DefaultVisual(display, 0);
	window = x11.XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, width, height, DEFAULT_MARGIN, 0, 0);
	x11.XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyReleaseMask | KeyPressMask | PointerMotionMask);

	XSizeHints sizeHints;
	memset(&sizeHints, 0, sizeof(sizeHints));
	x11.XGetWMNormalHints(display, window, &sizeHints, nullptr);
	sizeHints.min_width = width;
	sizeHints.max_width = width;
	sizeHints.min_height = height;
	sizeHints.max_height = height;
	x11.XSetWMNormalHints(display, window, &sizeHints);

	//XClearWindow(display, window);
	x11.XMapRaised(display, window);

	gc = x11.XCreateGC(display, window, 0, 0);

	pixelBufferLength = width * height * depth / 8;
    pixelBuffer = (uint8_t*)malloc(pixelBufferLength);
	xImage = x11.XCreateImage(display, visual, 24, ZPixmap, 0, (char*)pixelBuffer, width, height, depth, 0);
#endif
	return 0;
}

int WindowCanvas::uninitialize() {
#if defined(_WIN32)
	::SelectObject(hDCMem, oldBitmap);
	DeleteObject(bitmap);
	DeleteObject(hDCMem);
	delete [] pixelBuffer;
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);
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
	assert(!"Not implemented");
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
	//ExtFloodFill(hdc, 0, 0, RGB(0, 0, 0), FLOODFILLSURFACE);
#endif
	memset(pixelBuffer, 0, pixelBufferLength);
}

void WindowCanvas::blit() {
#if defined(_WIN32)
	BitBlt(hdc, 0, 0, width, height, hDCMem, 0, 0, SRCCOPY);
#else // __linux__
	x11.XPutImage(display, window, gc, xImage, 0, 0, 0, 0, width, height);
#endif
}