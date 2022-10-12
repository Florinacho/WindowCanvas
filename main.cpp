#include "WindowCanvas.h"
#include <stdio.h>

int main() {
	WCanvas canvas(800, 600, 32, "Window canvas demo");
	uint32_t* pixelBuffer = (uint32_t*)canvas.getPixelBuffer();
	bool running = true;
	WEvent event;
	
	while(running) {
		while (canvas.getEvent(event)) {
			switch (event.type) {
			case WEvent::WindowClose :
				running = false;
				break;
			case WEvent::KeyReleased :
				if (event.keyCode == 0x09) {
					running = false;
				}
				break;
			case WEvent::Character :
				if (event.chr == 'q') {
					running = false;
				}
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