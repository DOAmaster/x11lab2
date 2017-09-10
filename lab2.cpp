//modified by: Derrick Alden
//
//program: lab2.cpp
//author:  Gordon Griesel
//purpose: Framework for graphics
//         Using X11 (Xlib) for drawing
//
//drawing straight lines
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

const int MAX_POINTS = 10;

struct Point {
 // int x,y;
 double x,y;
};

class Global {
public:
	int xres, yres;
	Point point[MAX_POINTS];
	int npoints;
	int grabbed_point;
	bool rigid;
	int  anchor;
	Global() {
		srand((unsigned)time(NULL));
		xres = 800;
		yres = 600;
		npoints = 0;
 		grabbed_point = -1;
		rigid = false;
		anchor = 1;
	}
} g;

class X11 {
private:
	Display *dpy;
	Window win;
	GC gc;
public:
	X11() {
		//constructor
		if (!(dpy=XOpenDisplay(NULL))) {
			fprintf(stderr, "ERROR: could not open display\n"); fflush(stderr);
			exit(EXIT_FAILURE);
		}
		int scr = DefaultScreen(dpy);
		win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), 1, 1,
			g.xres, g.yres, 0, 0x00ffffff, 0x00000000);
		XStoreName(dpy, win, "x11lab2");
		gc = XCreateGC(dpy, win, 0, NULL);
		XMapWindow(dpy, win);
		XSelectInput(dpy, win, ExposureMask | StructureNotifyMask |
			PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
			KeyPressMask);
	}
	~X11() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void check_resize(XEvent *e) {
		//ConfigureNotify is sent when window size changes.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		g.xres = xce.width;
		g.yres = xce.height;
	}
	void clear_screen() {
		XClearWindow(dpy, win);
	}
	void setColor3i(int r, int g, int b) {
		unsigned long cref = 0L;
		cref += r;
		cref <<= 8;
		cref += g;
		cref <<= 8;
		cref += b;
		XSetForeground(dpy, gc, cref);
	}
	bool getXPending() {
		return (XPending(dpy));
	}
	void getXNextEvent(XEvent *e) {
		XNextEvent(dpy, e);
	}
	void drawPixel(int x, int y) {
		XDrawPoint(dpy, win, gc, x, y);
	}
	void fillRectangle(int x, int y, int w, int h) {
		XFillRectangle(dpy, win, gc, x, y, w, h);
	}
	void drawLine(int x0, int y0, int x1, int y1) {
		XDrawLine(dpy, win, gc, x0, y0, x1, y1);
	}
	void drawText(int x, int y, const char *text) {
		XDrawString(dpy, win, gc, x, y, text, strlen(text));
	}
} x11;

//function prototypes
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();


int main(void)
{
	int done = 0;
	while (!done) {
		//Check the event queue
		while (x11.getXPending()) {
			//Handle them one-by-one
			XEvent e;
			x11.getXNextEvent(&e);
			x11.check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
			//Render when any event happens
			render();
		}
	}
	return 0;
}

void deletePoint() 
{
	if(g.npoints > 0){
		g.npoints--;
	}
}


void makePerpendicular(double v1[1], double v2[1])
{
	v2[0] = v1[1];
	v2[1] = -v1[0];
	double temp = v2[1];
	printf("v2 [2]: %lf", temp);

}

void perp()
{
	//checks for max points before prevent overflow
	if(g.npoints < MAX_POINTS) {

		double v[2];
		v[0] = g.point[1].x - g.point[0].x;
		v[1] = g.point[1].y - g.point[0].y;
		double vlen = sqrt(v[0]*v[0] + v[1]*v[1]);

		double v1[2];
		v1[0] = g.point[g.grabbed_point + 1].x - g.point[g.grabbed_point].x;
		v1[1] = g.point[g.grabbed_point + 1].y - g.point[g.grabbed_point].y;
		double len = sqrt(v1[0]*v1[0] + v1[1]*v1[1]);
	
       	 	//normalize vector size
		v1[0] /= len;
		v1[1] /= len;
		v1[0] *= vlen;
		v1[1] *= vlen;

		printf("v1[1]: %lf \n", v1[1]);
		
		double temp = v1[1]; 
		printf("temp: %lf \n", temp);
		v1[0] = v[1];
		v1[1] = -v[0];

		printf("v1[1]: %lf \n", v1[1]);
		//makePerpendicular(&v[1], &v1[1]);

		g.point[g.grabbed_point].x =  v1[0]+len;
		g.point[g.grabbed_point].y =  v1[1]+len;

		//creates a new point at location add to count
		g.point[g.npoints].x = g.point[g.grabbed_point].x;
		g.point[g.npoints].y = g.point[g.grabbed_point].y;
		++g.npoints;
		

	}

}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;
	int mx = e->xbutton.x;
	int my = e->xbutton.y;
	//release of the button
	if (e->type == ButtonRelease) {
    g.grabbed_point = -1;
		return;

  }
	if (e->type == ButtonPress) {
		//A mouse button was pressed.
		if (e->xbutton.button==1) {
	//Left button pressed
      //drag a point
      //find closest point to mouse
      double close_dist = 9e9;
      int close_idx;
      if(g.npoints > 0) {
      for (int i=0; i <g.npoints; i++) {
        double d0 = mx - g.point[i].x;
        double d1 = my - g.point[i].y;
        double dist = sqrt(d0*d0 + d1*d1);
        if (dist < close_dist) {
          close_dist = dist;
          close_idx = i;
        }
     


      }
      //we have closest point to mouse.
      g.grabbed_point = close_idx;

      }


		}
		if (e->xbutton.button==3) {
	//Right button pressed
	//checks if npoints has room then add points by cursor
      if ( g.npoints < MAX_POINTS) {
          g.point[g.npoints].x = mx;
          g.point[g.npoints].y = my;
          ++g.npoints;
      }
		}
	}
	if (savex != mx || savey != my) {
		//mouse moved
		savex = mx;
		savey = my;
    if (g.grabbed_point >=0 ) {
      if (g.rigid) {
        //put in loop to talk to each point
        //make a vector **Issue with rounding**
        //move 2 points at same time if rigid is on
	for(int i = 0; i < g.npoints; i++) {
		double v[2];
		v[0] = g.point[1].x - g.point[0].x;
		v[1] = g.point[1].y - g.point[0].y;
		double vlen = sqrt(v[0]*v[0] + v[1]*v[1]);
		g.point[g.grabbed_point].x = mx;
		g.point[g.grabbed_point].y = my;
		double v1[2];
		v1[0] = g.point[g.grabbed_point+1].x - mx;
		v1[1] = g.point[g.grabbed_point+1].y - my;
		double len = sqrt(v1[0]*v1[0] + v1[1]*v1[1]);
       		 //normalize vector size
		v1[0] /= len;
		v1[1] /= len;
		v1[0] *= vlen;
		v1[1] *= vlen;
		g.point[g.grabbed_point+1].x = mx + v1[0];
		g.point[g.grabbed_point+1].y = my + v1[1];
	}


       // g.point[g.grabbed_point+1].y = my + v[1];

      } else {

	g.point[g.grabbed_point].x = mx;
	g.point[g.grabbed_point].y = my;
      }
    }
	}

//end of check mouse  
}

int check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	switch (key) {	

	case XK_r:
      //toggle rigid
      if (g.rigid == true)
        g.rigid = false;
      else
        g.rigid = true;
      break;
	case XK_a:
	 g.anchor ^= 1;
      break;
     	case XK_p:
     	 perp();
	 break;
	case XK_d:
	deletePoint();
      break;
	case XK_Escape:
		//program is ending
	return 1;
	}
	return 0;
}

void physics(void)
{
	//no physics yet.
}

void drawAllPoints(int size) {
  x11.setColor3i(255, 255, 255);
  //size is doubled to fit in square
  if (g.anchor == 1) {
  	for (int i = 0; i < g.npoints; i++) {
     		//x11.drawPixel(g.point[i].x, g.point[i].y);
      		x11.fillRectangle(g.point[i].x-size, g.point[i].y-size, size*2+1, size*2+1);

  	}
  }

}

void drawAllLines() {
  drawAllPoints(4);
  //loops through the npoints using drawLine on them
  for (int i=0; i<g.npoints -1; i++) {

    x11.drawLine(g.point[i].x, g.point[i].y, g.point[i+1].x, g.point[i+1].y);
  }
}


void render(void)
{
  x11.clear_screen();
	//render function is always at the bottom of program.
	x11.setColor3i(255, 255, 255);
	x11.drawText(4, 8,  "Left-click to drag point.");
	x11.drawText(4, 20, "Right-click to place a point.");
	x11.drawText(4, 32, "L toggle lines");
	x11.drawText(4, 44, "A toggle anchors");
	x11.drawText(4, 56, "R rigid line");
	x11.drawText(4, 68, (g.rigid) ? "rigid ON" : "rigid OFF");
	x11.drawText(4, 80, (g.anchor) ? "anchor ON" : "anchor OFF");
	x11.drawText(4, 92, "P add perpendicular segment");
	x11.drawText(4, 104, "D remove point");
	x11.setColor3i(255, 0, 0);
//	x11.drawLine(100, 100, 200, 240);
	drawAllLines();
}














