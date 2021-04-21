#include <SDL/SDL.h>
#include <math.h>
#include <string.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

SDL_Surface* screen = NULL;

SDL_Event event;

int mouse_x=SCREEN_WIDTH/2,mouse_y=SCREEN_HEIGHT/2;

int mouserightdown=0;
int mouseleftdown=0;

int color_r=255,color_g=255,color_b=255;

int num_colors = 9;
int colors[] = {
	  0,   0,   0,
	255,   0,   0,
    255, 165,   0,
	255, 255,   0,
	  0, 128,   0,
	  0,   0, 255,
	 75,   0, 130,
	238, 130, 238,
	255, 255, 255,
};

int hold=0;

int inrect(int x0,int y0,int x,int y,int w,int h) {
	return x0>=x && x0<=x+w && y0>=y && y0<=y+h;
}

double d2r(double deg) {
	return deg*(M_PI/180.0);
}

double sgn(double x) {
	if(x<0) return -1;
	if(x>0) return  1;
	return 0;
}

Uint32 get_pixel32( SDL_Surface *surface, int x, int y )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;

    //Get the requested pixel
    return pixels[ ( y * surface->w ) + x ];
}

void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel )
{

	if(x<0 || x>SCREEN_WIDTH || y<0 || y>SCREEN_HEIGHT) return;

    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;

    //Set the pixel
    pixels[ ( y * surface->w ) + x ] = pixel;
}

void line(int x0,int y0,int x1,int y1,int r,int g,int b) {

  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    put_pixel32(screen,x0,y0,SDL_MapRGB(screen->format,r,g,b));
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void box(int x,int y,int w,int h,int r,int g,int b) {
	for(int i=x;i<=x+w;i++) {
		put_pixel32(screen,i,y+0,SDL_MapRGB(screen->format,r,g,b));
		put_pixel32(screen,i,y+h,SDL_MapRGB(screen->format,r,g,b));
	}
	for(int j=y;j<=y+h;j++) {
		put_pixel32(screen,x+0,j,SDL_MapRGB(screen->format,r,g,b));
		put_pixel32(screen,x+w,j,SDL_MapRGB(screen->format,r,g,b));
	}
}

void fill_box(int x,int y,int w,int h,int r,int g,int b) {
	for(int j=y;j<y+h;j++) {
		for(int i=x;i<x+w;i++) {
			put_pixel32(screen,i,j,SDL_MapRGB(screen->format,r,g,b));
		}
	}
}

void bcircle(int xc, int yc, int x, int y, int r,int g,int b) {
    put_pixel32(screen, xc+x, yc+y, SDL_MapRGB(screen->format,r,g,b));
    put_pixel32(screen, xc-x, yc+y, SDL_MapRGB(screen->format,r,g,b));
    put_pixel32(screen, xc+x, yc-y, SDL_MapRGB(screen->format,r,g,b));
    put_pixel32(screen, xc-x, yc-y, SDL_MapRGB(screen->format,r,g,b));
    put_pixel32(screen, xc+y, yc+x, SDL_MapRGB(screen->format,r,g,b));
    put_pixel32(screen, xc-y, yc+x, SDL_MapRGB(screen->format,r,g,b));
    put_pixel32(screen, xc+y, yc-x, SDL_MapRGB(screen->format,r,g,b));
    put_pixel32(screen, xc-y, yc-x, SDL_MapRGB(screen->format,r,g,b));
}

void circle(int xc, int yc, int rc, int r,int g, int b) {
    int x = 0, y = rc;
    int d = 3 - 2 * rc;
    bcircle(xc,yc, x,y, r,g,b);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
		}
        bcircle(xc,yc, x,y, r,g,b);
    }
}

void bfcircle(int xc, int yc, int x, int y, int r,int g,int b) {
    line(xc+x,yc+y, xc-x,yc+y, r,g,b);
    line(xc-x,yc+y, xc+x,yc+y, r,g,b);
    line(xc+x,yc-y, xc-x,yc-y, r,g,b);
    line(xc-x,yc-y, xc+x,yc-y, r,g,b);
    line(xc+y,yc+x, xc-y,yc+x, r,g,b);
    line(xc-y,yc+x, xc+y,yc+x, r,g,b);
    line(xc+y,yc-x, xc-y,yc-x, r,g,b);
    line(xc-y,yc-x, xc+y,yc-x, r,g,b);
}

void fill_circle(int xc, int yc, int rc, int r,int g, int b) {
    int x = 0, y = rc;
    int d = 3 - 2 * rc;
    bfcircle(xc,yc, x,y, r,g,b);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
		}
        bfcircle(xc,yc, x,y, r,g,b);
    }
}

int pallete_gui() {
	int sz=16;
	int w=16,h=16,g=4;
	int b=num_colors-1;
	int brv=0;

	for(int i=0;i<num_colors;i++) {
		box(i*(sz+g*2)+g-2,g-2,w+4,h+4,colors[b*3+0],colors[b*3+1],colors[b*3+2]);
		fill_box(i*(sz+g*2)+g,g,w,h,colors[i*3+0],colors[i*3+1],colors[i*3+2]);
		box(i*(sz+g*2)+g,g,w,h,colors[b*3+0],colors[b*3+1],colors[b*3+2]);
	}

	for(int i=0;i<num_colors;i++) {
		if(inrect(mouse_x,mouse_y,i*(sz+g*2)+g,g,w,h)) {
			box(i*(sz+g*2)+g,g,w,h,colors[brv*3+0],colors[brv*3+1],colors[brv*3+2]);
			return i;
		}
	}

	return -1;
}

int main( int argc, char* args[] ) {

	int quit=0;

    SDL_Init( SDL_INIT_EVERYTHING );

    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE );

	fill_circle(SCREEN_WIDTH/2,SCREEN_HEIGHT/2,100,255,255,255);

	while(!quit) {

		SDL_GetMouseState(&mouse_x,&mouse_y);

	    if( SDL_PollEvent( &event ) ) {
			switch(event.type) {
		        case SDL_KEYDOWN:
	                switch( event.key.keysym.sym ) {
	                    case SDLK_ESCAPE: quit=1; break;
						default: break;
					}
				break;
				case SDL_MOUSEBUTTONDOWN:
	                switch( event.button.button ) {
	                    case SDL_BUTTON_LEFT: mouseleftdown=1; break;
	                    case SDL_BUTTON_RIGHT: mouserightdown=1; break;
						default: break;
					}
				break;
				case SDL_MOUSEBUTTONUP:
	                switch( event.button.button ) {
	                    case SDL_BUTTON_LEFT: mouseleftdown=0; break;
	                    case SDL_BUTTON_RIGHT: mouserightdown=0; break;
						default: break;
					}
				break;
				default: break;
			}
		}

		int ci=pallete_gui();
		if(ci!=-1 && mouseleftdown) {
			color_r=colors[ci*3+0];
			color_g=colors[ci*3+1];
			color_b=colors[ci*3+2];
			hold=1;
		}

		if(!hold) {
			if(mouseleftdown) fill_circle(mouse_x,mouse_y,10,color_r,color_g,color_b);
			if(mouserightdown) {
				Uint32 rgb = get_pixel32(screen,mouse_x,mouse_y);
				color_r=rgb >> 16 | 8;
				color_g=rgb >>  8 | 8;
				color_b=rgb       | 8;
			}
		}

		if(!mouseleftdown && !mouserightdown) hold=0;

		SDL_Flip( screen );
	}

    SDL_Quit();

	return 0;
}

