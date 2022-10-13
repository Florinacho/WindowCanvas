#include "WindowCanvas.h"
#include <stdio.h>

#if defined(_WIN32)
#define KEY_ESCAPE VK_ESCAPE
#else
#define KEY_ESCAPE 0x09
#endif

int main() {
	WCanvas canvas(800, 600, 32, "Window canvas demo");
	uint32_t* pixelBuffer = (uint32_t*)canvas.getPixelBuffer();

	WEvent event;
	bool running = true;
	char c;
	
	while(running) {
		while (canvas.getEvent(event)) {
			switch (event.type) {
			case WEvent::WindowClose :
				printf("WindowClose\n");
				running = false;
				break;
			case WEvent::KeyPressed :
				printf("KeyPressed 0x%X\n", event.keyCode);
				if (event.getAscii(c)) {
					if (c == 'q') {
						running = false;
					} else {
						printf("Character '%c'\n", c);
					}
				}
				break;
			case WEvent::KeyReleased :
				printf("KeyReleased 0x%X\n", event.keyCode);
				if (event.keyCode == KEY_ESCAPE) {
					running = false;
				}
				break;
			case WEvent::CursorMove :
				printf("CursorMove %d, %d\n", event.x, event.y);
				break;
			case WEvent::ButtonPressed :
				printf("ButtonDown 0x%X\n", event.button);
				break;
			case WEvent::ButtonReleased :
				printf("ButtonUp 0x%X\n", event.button);
				break;
			case WEvent::WheelDown :
				printf("WheelDown\n");
				break;
			case WEvent::WheelUp :
				printf("WheelUp\n");
				break;
			}
		}
		
		canvas.clear();
		for (int y = 100; y < 200; ++y) {
			for (int x = 100; x < 200; ++x) {
				pixelBuffer[y * 800 + x] = 0x00FFFFFF;
			}
		}
		canvas.blit();
	}

	return 0;
}