#include "fb.h"
#include "mailbox.h"
#include "string.h"


static struct fb_info fb_info = {
	.width =		FB_DESIRED_WIDTH,
	.height =		FB_DESIRED_HEIGHT,
	.depth =		FB_DESIRED_DEPTH,
	.pitch =		0,
	.alignment =	FB_DESIRED_ALIGNMENT,
	.buffer =		NULL,
	.is_rgb =		1,
};

int fb_init()
{
	if (mailbox_init_fb_info(&fb_info) != 0) {
		printf("\rFailed to init fb.\n");
		return -1;
	}

	printf("\rfb info: w:%u, h:%u, depth:%u, pitch:%u, is_rgb:%u, buffer:%llu, size:%u\n",
			fb_info.width, fb_info.height,
			fb_info.depth, fb_info.pitch,
			fb_info.is_rgb, fb_info.buffer,
			fb_info.size);
	
	if(fb_info.width != FB_DESIRED_WIDTH || 
		fb_info.height != FB_DESIRED_HEIGHT ||
		fb_info.depth != FB_DESIRED_DEPTH ||
		fb_info.pitch != FB_DESIRED_ALIGNMENT) {
		printf("Failed to alloc %ux%ux%u frame buffer.",
			FB_DESIRED_WIDTH, FB_DESIRED_HEIGHT, FB_DESIRED_DEPTH);
		return -1;
	}

	return 0;
}

void fb_draw_splash_image()
{
	if(fb_info.buffer == NULL)
		return;

	//Black and white
	unsigned int required_color[] = {0xffffffff,0x00000000};
	unsigned int size = sizeof(required_color) / sizeof(uint32_t);
	int idx = 0;

	for (uint32_t h = 0; h < fb_info.height; h += FB_GRID_HEIGHT) {
		int start_idx = idx;
		
		for(uint32_t w = 0; w < fb_info.width; w += FB_GRID_WIDTH) {
			unsigned int* start_fb = (uint32_t *)(fb_info.buffer) + h * fb_info.width + w;
			
			int color_idx = start_idx % size;	
			
			int grid_width = (w + FB_GRID_WIDTH) <=  fb_info.width ?
				FB_GRID_WIDTH : fb_info.width - w;

			int grid_height = (h + FB_GRID_HEIGHT) <= fb_info.height ?
				FB_GRID_HEIGHT :  fb_info.height - h;


			for(uint32_t g_w = 0; g_w < grid_width; g_w++) {
				for(uint32_t g_h = 0; g_h < grid_height; g_h++) {
					uint32_t *pixel = start_fb + g_h * fb_info.width + g_w;
					(*pixel) = required_color[color_idx];
				}
			}

			start_idx++;
		}

		idx++;
	}
}