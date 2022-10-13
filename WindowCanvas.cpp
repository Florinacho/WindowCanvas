#include "WindowCanvas.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	WindowCanvas* window = (WindowCanvas*)GetWindowLongPtr(hwnd, GWL_USERDATA);
	if (window == nullptr) {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	WindowEvent& event = *window->eventPtr;
#if defined(_WIN32)
    static BYTE keyState[256];
	char text[8];
#endif // _WIN32
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
#endif // _WIN32

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
	display = XOpenDisplay(nullptr);
	Visual *visual = DefaultVisual(display, 0);
	window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, width, height, DEFAULT_MARGIN, 0, 0);
	XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyReleaseMask | KeyPressMask | PointerMotionMask);
	
	XSizeHints sizeHints;
	memset(&sizeHints, 0, sizeof(sizeHints));
	XGetWMNormalHints(display, window, &sizeHints, nullptr);
	sizeHints.min_width = width;
	sizeHints.max_width = width;
	sizeHints.min_height = height;
	sizeHints.max_height = height;
	XSetWMNormalHints(display, window, &sizeHints);
	
	XClearWindow(display, window);
	XMapRaised(display, window);

	gc = XCreateGC(display, window, 0, 0);

	pixelBufferLength = width * height * depth / 8;
    pixelBuffer = (uint8_t*)malloc(pixelBufferLength);
	xImage = XCreateImage(display, visual, 24, ZPixmap, 0, (char*)pixelBuffer, width, height, depth, 0);
	return 0;
#endif
	return 1;
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
		// XImage owns the pixelBuffer
		XDestroyImage(xImage);
	}
	
	if (display != nullptr) {
		XFreeGC(display, gc);
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = nullptr;
	}
#endif
	return 0;
}

WindowCanvas::WindowCanvas(uint32_t width, uint32_t height, uint8_t depth, const char* title) 
	: width(width), height(height), depth(depth), pixelBufferLength(0), pixelBuffer(nullptr) {
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
	static char title[128];
#if defined(_WIN32)
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
	if (XPending(display) > 0) {
		XNextEvent(display, &xEvent);
		switch (xEvent.type) {
		case KeyPress :
			event.type = WindowEvent::KeyPressed;
			event.keyCode = xEvent.xkey.keycode;
			if (XLookupString(&xEvent.xkey, text, sizeof(text), &key, 0) == 1) {
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
	XPutImage(display, window, gc, xImage, 0, 0, 0, 0, width, height);
#endif
}