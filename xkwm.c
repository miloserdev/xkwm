#include <signal.h>

#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>




FILE *logfd;
void clear_log() { logfd = fopen("/home/max/fuck.log", "w"); fprintf(logfd, "   "); fclose(logfd); }
#define LOG(__format, __VA_ARGS__...)		{ logfd = fopen("/home/max/fuck.log", "a"); fprintf(logfd, "[%s] "__format" \n", __FUNCTION__, __VA_ARGS__); fclose(logfd); }
//#define LOG(__format)	{ LOGF(__format, 0) }



//#define USE_FUCKING_MOUSE			// to use fucking mouse;




typedef struct monitor_t;
typedef struct app_t {
	Window window;
	struct app_t *next;
} app_t;

struct app_t *last, *first;


bool app_init()
{
	first = (struct app_t*) malloc(sizeof(struct app_t));
	first->window = 0;
	first->next = first;
	//last->next = last;
	last = first;
	LOG("apps init", NULL);
}

bool app_add(Window win)
{
	app_t* tmp = (struct app_t*) malloc(sizeof(struct app_t));
	tmp->window = win;
	tmp->next = first;
	last->next = tmp;
	last = tmp;
	LOG("prog %d added", win);
}

//#define app_iterate(code)	{ app_t *ptr = first; for (;;) { ptr = ptr->next; code if (ptr == first) return; } }

bool app_remove(Window win)
{
	app_t *ptr = first;
	app_t *tmp = NULL;
	for (;;)
	{
		ptr = ptr->next;
		tmp = ptr->next;
		LOG("iterator at %d , next %d ", ptr->window, tmp->window);
		if (tmp->window == win)
		{
			LOG("removing %d", tmp->window);
			ptr->next = tmp->next;
		}
		if (ptr == first) return;
	}
}



Display *display;
Window root;
int def_screen;

uint32_t WIDTH = 0;
uint32_t HEIGHT = 0;
uint32_t black = 0;
uint32_t white = 0;

Font font;
GC gc;

#ifdef USE_FUCKING_MOUSE
	#include <X11/cursorfont.h>
	Cursor cursor;
#endif




void kz(int x)
{
	if ( signal(SIGCHLD, kz) == SIG_ERR ) { exit(-1); };
	while(0 < waitpid(-1, NULL, WNOHANG) );
}


void event_loop();
void init_x();
void init_keys();

bool run_prog(char* prog);
bool grab_key(int key, int mask);

void focus_io_handler(XFocusChangeEvent *e);
void enter_notify_handler(XEvent *e);
void leave_notify_handler(XEvent *e);
void expose_handler(XExposeEvent *e);
void key_press_handler(XKeyEvent *e);
void key_release_handler(XKeyEvent *e);

void configure_request_handler(XConfigureRequestEvent *e);
void configure_notify_handler(XConfigureEvent *e);
void map_request_handler(XMapRequestEvent *e);

#ifdef USE_FUCKING_MOUSE
void button_press_handler(XButtonEvent *e);
void button_release_handler(XButtonEvent *e);
#endif

void motion_notify_handler(XMotionEvent *e);
void destroy_notify_handler(XEvent *ev);

#define BORDER_WIDTH		3
#define BORDER_COLOR		0x0000FF
#define BACKGROUND_COLOR	0x000000
Window containerize_window(Window win)
{
	XWindowAttributes xwa;
	XGetWindowAttributes(display, win, &xwa);

	Window frame = XCreateSimpleWindow(display, root, xwa.x, xwa.y, xwa.width, xwa.height, BORDER_WIDTH, BORDER_COLOR, BACKGROUND_COLOR);
	XSelectInput(display, frame, SubstructureRedirectMask | SubstructureNotifyMask);
	XReparentWindow(display, win, frame, 0, 0);
	return frame;
}


int main()
{
	clear_log();
	LOG("XKWM Begin", NULL);

	init_x();

	init_keys();
	LOG("XKWM init done", NULL);

	app_init();


	event_loop();

	XCloseDisplay(display);
//	exit(0);
}



int onerror(Display *dpy, XErrorEvent *e)
{
	char error_out[1024];
	XGetErrorText(display, e->error_code, error_out, sizeof(error_out));
	printf("\n====================\nXORG ERROR | %s \n====================\n", error_out);
	return 0;
}



void init_x()
{
//	struct sigaction action;
//	memset(&action 0, sizeof(struct sigaction));
//	action.sa_flags = SA_SIGINFO;

//	kz(0);

	display 	= XOpenDisplay( /*(const char*)*/ display);
	def_screen	= DefaultScreen(display);

	LOG("display got, default screen %d", def_screen);

	WIDTH		= DisplayWidth(display, def_screen);
	HEIGHT		= DisplayHeight(display, def_screen);

	root		= RootWindow(display, def_screen);

	black		= BlackPixel(display, def_screen);
	white		= WhitePixel(display, def_screen);

	XSync(display, False);
	XFlush(display);
	LOG("flush", NULL);


/**/	XSetErrorHandler(onerror);

/**/	font = XLoadFont(display, "-*-*-*-R-Normal---*-180-100-100-*-*");


	XSetWindowAttributes at;
	at.event_mask = SubstructureRedirectMask |
			SubstructureNotifyMask |
			StructureNotifyMask |
			ExposureMask |
			KeyPressMask |
			KeyReleaseMask |

			// fucking mouse;
		//	PointerMotionMask |
		//	ButtonPressMask |
		//	ButtonReleaseMask |
			//

			PropertyChangeMask;
	at.do_not_propagate_mask = 0;


	#ifdef USE_FUCKING_MOUSE
	cursor = XCreateFontCursor(display, XC_left_ptr);
	at.cursor = cursor;
	#endif

	unsigned long at_mask = CWEventMask
			#ifdef USE_FUCKING_MOUSE
				| CWCursor
			#endif
				| NoEventMask;

	XSelectInput(display, root, at.event_mask);
	XSync(display, False);
	XChangeWindowAttributes(display, root, at_mask, &at);

	XUngrabKey(display, AnyKey, AnyModifier, root);

	XGCValues xgc;
	xgc.foreground = white;
	xgc.background = black;
	gc = XCreateGC(display, root, GCForeground | GCBackground, &xgc);
	XFillRectangle(display, root, gc, 100, 100, 200, 200);

//	XSync(display, False);
//	XFlush(display);

}


void event_loop()
{
	XEvent e;

	while (1)
	{
		XDrawString(display, root, gc, 100, 100, "fuck", 4);

		XNextEvent(display, &e);
		switch (e.type)
		{
			case FocusIn: { focus_io_handler(&e.xfocus); break; }
			case FocusOut: { focus_io_handler(&e.xfocus); break; }
			case EnterNotify: { enter_notify_handler(&e); break; }
			case LeaveNotify: { leave_notify_handler(&e); break; }
			case Expose: { expose_handler(&e.xexpose); break; }
			case KeyPress: { key_press_handler(&e.xkey); break; }
			case KeyRelease: { key_release_handler(&e.xkey); break; }

			case ConfigureRequest: { configure_request_handler(&e.xconfigurerequest); break; }
			case ConfigureNotify: { configure_notify_handler(&e.xconfigure); break; }
			case MapRequest: { map_request_handler(&e.xmaprequest); break; }
			case ButtonPress: { button_press_handler(&e.xbutton); break; }
			case ButtonRelease: { button_release_handler(&e.xbutton); break; }
			case MotionNotify: { motion_notify_handler(&e.xmotion); break; }
			case DestroyNotify: { destroy_notify_handler(&e); break; }

			default: { break; }
		}
	}
}


void focus_io_handler(XFocusChangeEvent *e)
{
	LOG("focus_io | window %lu", e->window);
}


void enter_notify_handler(XEvent *ev)
{
	XCrossingEvent *e = &ev->xcrossing;
	LOG("enter | window %lu", e->window);
}


void leave_notify_handler(XEvent *ev)
{
	XCrossingEvent *e = &ev->xcrossing;
	LOG("leave | window %lu", e->window);
}


void expose_handler(XExposeEvent *e)
{
	LOG(" expose | window %lu", e->window); XSync(display, False);
}


void key_press_handler(XKeyEvent *e)
{
	LOG("key_press | window %lu", e->window);
}


void key_release_handler(XKeyEvent *e)
{
	int key = XKeycodeToKeysym(display, e->keycode, 0);
	int _mask = (e->state & (ShiftMask | ControlMask | Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask));
	LOG("key_release | window %lu | key %d | mask %d", e->window, e->keycode, _mask);

	switch(key)
	{
		case XK_0:
		{
		run_prog("dmenu_run");
		break;
		}
		default: break;
	}
}


void configure_request_handler(XConfigureRequestEvent *e)
{
	LOG("window %lu", e->window);
}


void configure_notify_handler(XConfigureEvent *e)
{
	LOG("window %lu", e->window);
}


void map_request_handler(XMapRequestEvent *e)
{
	LOG("window %lu", e->window);
	//Window frame = containerize_window(e->window);
	//XSelectInput(display, e->window, SubstructureRedirectMask | SubstructureNotifyMask);
	//XReparentWindow(display, e->window, root, 0, 0);
	app_add(e->window);
	XMapWindow(display, e->window);
}


void button_press_handler(XButtonEvent *e)
{
	LOG("window %lu", e->window);
}


void button_release_handler(XButtonEvent *e)
{
	LOG("window %lu", e->window);
}


void motion_notify_handler(XMotionEvent *e)
{
	LOG("window %lu", e->window);
}


void destroy_notify_handler(XEvent *ev)
{
	XDestroyWindowEvent *e = &ev->xclient;

	app_remove(e->window);

	XDestroyWindow(display, e->window);
	LOG("window %lu", e->window);
}







#define KeyMask		Mod4Mask
void init_keys()
{
	grab_key(XK_0, KeyMask);
}


bool grab_key(int key, int mask)
{
	XGrabKey(display, XKeysymToKeycode(display, key), mask, root, True, GrabModeAsync, GrabModeAsync);
}


bool run_prog(char* prog)
{
	if (fork() == 0)
	{
		if (display) close(ConnectionNumber(display));
		char *cmd[] = { prog, NULL };
		setsid();
		execvp(cmd[0], cmd);
		exit(0);
	}
}
