#ifndef __WINDOW_CANVAST_H__
#define __WINDOW_CANVAST_H__

#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined (__linux__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#else
#error "Platform not supported"
#endif

struct WindowEvent {
	enum Type {
		Unknown, 
		
		// Window events
		WindowMove,
		WindowSize,
		WindowClose,
		
		// Keyboard events
		KeyPressed,
		KeyReleased,
		
		// Mouse events
		CursorMove,
		ButtonPressed,
		ButtonReleased,
		Wheel,
		
		// character key event
		Character,
	};
	
	Type type;
	union {
		int32_t x;
		uint32_t width;
		uint32_t keyCode;
		uint32_t button;
		char chr;
	};
	union {
		int32_t y;
		uint32_t height;
		int32_t value;
	};
	
	WindowEvent(Type type = Unknown, int lParam = 0, int wParam = 0) 
		: type(type), x(lParam), y(wParam) {
	}
};

typedef WindowEvent WEvent;

class WindowCanvas {
#if defined (_WIN32)
	WindowEvent* eventPtr;
#else
	Display* display;
	Window window;
	GC gc;
	XImage* xImage;
#endif
	uint32_t width;
	uint32_t height;
	uint8_t depth;
	
	uint8_t* pixelBuffer;
	uint32_t pixelBufferLength;
	
	int initialize(uint32_t width, uint32_t height, uint8_t depth, const char* title);
	int uninitialize();

public:
	/*
		Supported depth values: 24, 32
	*/
	WindowCanvas(uint32_t width, uint32_t height, uint8_t depth = 32, const char* title = "");
	
	~WindowCanvas();
	
	void setTitle(const char* title = "");
	
	const char* getTitle() const;
	
	/*
		Returns the internal pixel buffer that will be displayed in the window.
	*/
	uint8_t* getPixelBuffer() const;
	
	/*
		Returns the internal pixel buffer length. 
		value = width * height * depth / 8
	*/
	uint32_t getPixelBufferLength() const;
	
	/*
		If any events are available, populate the 'event' and returns true.
		Returns false otherwise.
	*/
	bool getEvent(WindowEvent& event);
	
	/*
		Clear the internal pixel buffer by filling it with 0.
		This will result in a black screen.
	*/
	void clear();
	
	/*
		Send the internal pixel buffer to the display.
	*/
	void blit();
};

typedef WindowCanvas WCanvas;

#endif // __WINDOW_CANVAST_H__