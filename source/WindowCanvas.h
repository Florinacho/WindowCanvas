#ifndef __WINDOW_CANVAS_H__
#define __WINDOW_CANVAS_H__

#include <stdint.h>

#if defined (__linux__) 
#include <X11/Xlib.h>
#elif defined (_WIN32)
#include <windows.h>
#endif

struct WindowEvent {
	enum Type {
		Unknown, 
		WindowClose,
		KeyPressed,
		KeyReleased,
		CursorMove,
		ButtonPressed,
		ButtonReleased,
		WheelDown,
		WheelUp,
	} type;
	union {
		int32_t x;
		uint32_t width, keyCode, button;
	};
	union {
		int32_t y;
		uint32_t height;
		char ascii;
	};

	WindowEvent(Type type = Unknown, int lParam = 0, int wParam = 0) 
		: type(type), x(lParam), y(wParam) {
	}
};

typedef WindowEvent WEvent;

class WindowCanvas {
	uint32_t width;
	uint32_t height;
	uint8_t depth;
	uint8_t* pixelBuffer;
	uint32_t pixelBufferLength;
#if defined (_WIN32)
	friend LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
	HWND hwnd;
	HDC hdc;
	HDC hDCMem;
	HBITMAP bitmap;
	HGDIOBJ oldBitmap;
	WindowEvent* eventPtr;
#else
	Display* display;
	Window window;
	GC gc;
	XImage* xImage;
    Atom wm_delete_window;
#endif
	int initialize(uint32_t width, uint32_t height, uint8_t depth, const char* title);
	int uninitialize();

public:
	// Supported depth values: 24, 32
	WindowCanvas(uint32_t width, uint32_t height, uint8_t depth = 32, const char* title = "");

	~WindowCanvas();

	uint32_t getWidth() const;

	uint32_t getHeight() const;

	uint32_t getDepth() const;

	void setTitle(const char* title = "");

	const char* getTitle() const;

	// Returns the internal pixel buffer that will be displayed in the window.
	uint8_t* getPixelBuffer() const;

	//Returns the internal pixel buffer length. 
	// value = width * height * depth / 8
	uint32_t getPixelBufferLength() const;

	// If any events are available, populate the 'event' and returns true.
	// Returns false otherwise.
	bool getEvent(WindowEvent& event);

	// Clear the internal pixel buffer by filling it with 0.
	void clear();

	//Send the internal pixel buffer to the display.
	void blit();
};

typedef WindowCanvas WCanvas;

#endif // __WINDOW_CANVAS_H__