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
	int cx = 0, cy = 0;
	int px = 350, py = 250;

	WEvent event;
	bool running = true;
	
	while(running) {
		while (canvas.getEvent(event)) {
			switch (event.type) {
			case WEvent::Unknown :
				break;
			case WEvent::WindowClose :
				printf("WindowClose\n");
				running = false;
				break;
			case WEvent::KeyPressed :
				printf("KeyPressed 0x%X\n", event.keyCode);
				if (event.ascii != '\0') {
					printf("Character '%c'\n", event.ascii);
					if (event.ascii == 'q') {
						running = false;
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
				cx = event.x;
				cy = event.y;
				break;
			case WEvent::ButtonPressed :
				printf("ButtonDown 0x%X\n", event.button);
				px = cx;
				py = cy;
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
		for (int y = 0; y < 32; ++y) {
			for (int x = 0; x < 32; ++x) {
				pixelBuffer[(py + y) * 800 + (px + x)] = 0x00FFFFFF;
			}
		}
		canvas.blit();
	}

	return 0;
}