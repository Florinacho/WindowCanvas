#include "WindowCanvas.h"

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const uint32_t WINDOW_WIDTH  = 1024; // TODO: calculate this
	const uint32_t WINDOW_HEIGHT = 720;

	HDC hDCMem = (HDC)GetWindowLongPtr(hwnd, GWL_USERDATA);

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_MOUSEMOVE :
		WMOnCursorEvent(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_LBUTTONDOWN :
		WMOnButtonEvent(GUI_BUTTON_LEFT, GUI_BUTTON_PRESSED);
		return 0;
	case WM_LBUTTONUP :
		WMOnButtonEvent(GUI_BUTTON_LEFT, GUI_BUTTON_RELEASED);
		return 0;
	case WM_MOUSEWHEEL :
		GUIOnMouseWheelEvent(GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -1 : 1);
		return 0;
	case WM_KEYDOWN :
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		GUIOnKeyEvent(wParam, true);
		return 0;
	case WM_CHAR :
		GUIOnCharEvent(wParam);
		return 0;
	case WM_PAINT:
		HDC hdc = GetDC(hwnd);
		ExtFloodFill(hdc, 0, 0, RGB(255, 255, 255), FLOODFILLSURFACE);
		BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hDCMem, 0, 0, SRCCOPY);
		ReleaseDC(hwnd, hdc);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif // _WIN32

int WindowCanvas::initialize(uint32_t width, uint32_t height, uint8_t depth, const char* title) {
#if defined(_WIN32)
#else // __linux__
	static const uint32_t DEFAULT_MARGIN = 5;
	display = XOpenDisplay(NULL);
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
#else // __linux__
	if (display != nullptr) {
		XFreeGC(display, gc);
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = nullptr;
	}
	
	if (xImage != nullptr) {
		XDestroyImage(xImage);
	}
	
	if (pixelBuffer != nullptr) {
//		delete [] pixelBuffer;
//		pixelBuffer = nullptr;
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

void WindowCanvas::setTitle(const char* title) {
}

const char* WindowCanvas::getTitle() const {
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
#else // __linux__
	XEvent xEvent;
	KeySym key;
	char text[1];
	if (XPending(display) > 0) {
		XNextEvent(display, &xEvent);
		switch (xEvent.type) {
		case KeyPress :
			if (XLookupString(&xEvent.xkey, text, 1, &key, 0) == 1) {
				switch (text[0]) {
				case 0x7F : // delete
				case 0x8 : // backspace
					event.type = WindowEvent::KeyPressed;
					event.keyCode = xEvent.xkey.keycode;
					break;
				default :
					event.type = WindowEvent::Character;
					event.chr = text[0];
					break;
				}
			} else {
				event.type = WindowEvent::KeyPressed;
				event.keyCode = xEvent.xkey.keycode;
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
				event.type = WindowEvent::Wheel;
				event.value = -1;
				break;
			case Button5 :
				event.type = WindowEvent::Wheel;
				event.value = 1;
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
			case Button1 :
				event.type = WindowEvent::ButtonReleased;
				event.button = xEvent.xbutton.button;
				break;
			}
			ans = true;
			break;
		}
	}
#endif
	return ans;
}

void WindowCanvas::clear() {
	memset(pixelBuffer, 0, pixelBufferLength);
}

void WindowCanvas::blit() {
#if defined(_WIN32)
#else // __linux__
	XPutImage(display, window, gc, xImage, 0, 0, 0, 0, width, height);
#endif
}