#ifndef _FB_H
#define _FB_H

#include "core.h"

#define FB_GRID_WIDTH	60
#define FB_GRID_HEIGHT	60

#define FB_DESIRED_DEPTH		32
#define FB_DESIRED_WIDTH		1920
#define FB_DESIRED_HEIGHT		1080
#define FB_DESIRED_ALIGNMENT	7680

struct fb_info
{
	uint32_t	width;
	uint32_t	height;
	uint32_t	depth;
	uint32_t	pitch;
	uint32_t	alignment;
	void		*buffer;
	uint32_t	size;  
	int			is_rgb;	
};

int fb_init();

void fb_draw_splash_image();

#endif