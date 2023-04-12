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


Atom wmdeletewindow;
Atom netwmtype	;//	= NULL;
Atom netdesktop	;//	= NULL;
Atom netdock	;//	= NULL;
Atom nettoolbar	;//	= NULL;
Atom netmenu	;//	= NULL;
Atom netutility	;//	= NULL;
Atom netsplash	;//	= NULL;
Atom netdialog	;//	= NULL;
Atom netnormal	;//	= NULL;

Atom get_atom(Window window, Atom at);
void print_types(Window window);

Window get_focused();
void focus_window(Window win);
void focus_prev(Window win);
void focus_next(Window win);

void update_borders(Window win);


#define KeyMask		Mod4Mask

#define USE_FUCKING_MOUSE			// to use fucking mouse;


uint32_t _RGB(r, g, b)	{ return b + (g << 8) + (r << 16); }

#define	BORDER_WIDTH		3
#define	BORDER_COLOR		_RGB(20, 20, 20)
#define	BORDER_COLOR_FOCUSED	_RGB(255, 100, 100)


typedef struct monitor_t;
typedef struct app_t {
	struct app_t *prev;
	Window window;
	Atom type;
	struct app_t *next;
} app_t;

app_t *first = NULL;
app_t *last = NULL;
size_t win_count = 0;
//Window focused;

bool app_init()
{
	last = (app_t*) malloc(sizeof(app_t));
	last->window = 0;
	last->prev = last;
	last->next = last;

	first = last;

//	win_count++;
	LOG("apps init | first window %d | first`s next %d", first->window, first->next->window, first->prev->window);
}


bool app_list()
{
	LOG("", NULL);
	for (app_t *ptr = first->next; ptr != first; ptr = ptr->next)
	{
		LOG("app iterator | prev %d | window %d | next %d ", ptr->prev->window, ptr->window, ptr->next->window);
	}
	LOG("", NULL);
}


bool app_add(Window win)
{
	app_t* tmp = (app_t*) malloc(sizeof(app_t));
	tmp->window = win;
	tmp->next = first;
	tmp->prev = last;

	tmp->type = get_atom(win, netwmtype);

	last->next = tmp;
	last = tmp;

	win_count++;
	LOG("prog %d | type %d | added as last | last`s prev %d | last is %d | last`s next %d", tmp->window, tmp->type, last->prev->window, last->window, last->next->window);
	app_list();
}

//#define app_iterate(code)	{ app_t *ptr = first; for (;;) { ptr = ptr->next; code if (ptr == first) return; } }

bool app_remove(Window win)
{
	app_t *ptr = first;
	for (;;)
	{
		ptr = ptr->next;
		LOG("iterator prev %d | current %d | next %d ", ptr->prev->window, ptr->window, ptr->next->window);
		if (ptr->window == win)
		{
			LOG("removing %d", ptr->window);

			ptr->prev->next = ptr->next;
			ptr->next->prev = ptr->prev;

			last = (ptr->next != first) ? ptr->next : ptr->prev;

			if (win_count > 0) win_count--;

			app_list();
			return;
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
	Cursor cursor_move;
	Cursor cursor_size;
#endif




void kz(int x)
{
	if ( signal(SIGCHLD, kz) == SIG_ERR ) { exit(-1); };
	while(0 < waitpid(-1, NULL, WNOHANG) );
}


void event_loop();
void init_atoms();
void init_x();
void init_keys();

bool run_prog(char* prog);
void map_windows();
bool grab_key(int key, int mask);

void focus_io_handler(XFocusChangeEvent *e);
void enter_notify_handler(XEvent *e);
void leave_notify_handler(XEvent *e);
void expose_handler(XExposeEvent *e);
void create_notify_handler(XCreateWindowEvent *e);
void property_notify_handler(XPropertyEvent *e);
void key_press_handler(XKeyEvent *e);
void key_release_handler(XKeyEvent *e);

void configure_request_handler(XConfigureRequestEvent *e);
void configure_notify_handler(XConfigureEvent *e);
void map_request_handler(XMapRequestEvent *e);
void map_notify_handler(XMapEvent *e);
void unmap_notify_handler(XUnmapEvent *e);

#ifdef USE_FUCKING_MOUSE
void button_press_handler(XButtonEvent *e);
void button_release_handler(XButtonEvent *e);
void motion_notify_handler(XMotionEvent *e);
#endif

void destroy_notify_handler(XEvent *ev);
void reparent_notify_handler(XReparentEvent *e);




void setup_window(Window win);


int main()
{
	clear_log();
	LOG("XKWM Begin", NULL);

	init_x();

	init_atoms();

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
			PointerMotionMask |
			ButtonPressMask |
			ButtonReleaseMask |
			//
			PropertyChangeMask;
	at.do_not_propagate_mask = 0;


	#ifdef USE_FUCKING_MOUSE
	cursor = XCreateFontCursor(display, XC_left_ptr);
	cursor_move = XCreateFontCursor(display, XC_fleur);
	cursor_size = XCreateFontCursor(display, XC_sizing);
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


void init_atoms()
{
	LOG("init atoms", NULL);
	wmdeletewindow	=	XInternAtom(display, "WM_DELETE_WINDOW", 0);
	netwmtype	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	netdesktop	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
	netdock		=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
	nettoolbar	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
	netmenu		=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_MENU", False);
	netutility	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
	netsplash	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
	netdialog	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netnormal	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
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
			case CreateNotify: { create_notify_handler(&e.xcreatewindow); break; }
			case PropertyNotify: { property_notify_handler(&e.xproperty); break; }
			case KeyPress: { key_press_handler(&e.xkey); break; }
			case KeyRelease: { key_release_handler(&e.xkey); break; }
			case ConfigureRequest: { configure_request_handler(&e.xconfigurerequest); break; }
			case ConfigureNotify: { configure_notify_handler(&e.xconfigure); break; }

			case MapRequest: { map_request_handler(&e.xmaprequest); break; }
			case MapNotify: { map_notify_handler(&e.xmap); break; }
			case UnmapNotify: { unmap_notify_handler(&e.xunmap); break; }
			case ButtonPress: { button_press_handler(&e.xbutton); break; }
			case ButtonRelease: { button_release_handler(&e.xbutton); break; }
			case MotionNotify: { motion_notify_handler(&e.xmotion); break; }
			case DestroyNotify: { destroy_notify_handler(&e); break; }
			case ReparentNotify: { reparent_notify_handler(&e.xreparent); break; }

//			default: { break; }
		}
	}
}


void focus_io_handler(XFocusChangeEvent *e)
{
	LOG("focus_io | window %lu", e->window);
	update_borders(e->window);
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
	LOG(" expose | window %lu", e->window);
	XSync(display, False);
}

void create_notify_handler(XCreateWindowEvent *e)
{
	LOG("create window %d", e->window);

	XSelectInput(display, e->window,
					SubstructureRedirectMask |
					SubstructureNotifyMask |
					EnterWindowMask |
					LeaveWindowMask |
					FocusChangeMask
	);
}

void property_notify_handler(XPropertyEvent *e)
{
	LOG("window %d", e->window);
}


void key_press_handler(XKeyEvent *e)
{
	LOG("key_press | window %lu", e->window);
}


void key_release_handler(XKeyEvent *e)
{
	int key = XKeycodeToKeysym(display, e->keycode, 0);
	int _mask = (e->state & (ShiftMask | ControlMask | Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask));

	Window fcs = get_focused();

	LOG("key_release | window %lu | key %d | mask %d", e->window, e->keycode, _mask);

	switch(key)
	{
		case XK_0: { if (_mask == KeyMask) run_prog("dmenu_run"); break; }

		case XK_Left: { focus_prev(fcs); break; }
		case XK_Right: { focus_next(fcs); break; }

		default: break;
	}
}


void configure_request_handler(XConfigureRequestEvent *e)
{
	LOG("window %lu", e->window);
	print_types(e->window);

	long msize;
	XSizeHints size;
	if (!XGetWMNormalHints(display, e->window, &size, &msize)) size.flags = PSize;

	XWindowChanges xwc;
	xwc.x 			= e->x;
	xwc.y			= e->y;
	xwc.width		= e->width;
	xwc.height		= e->height;
	xwc.border_width	= e->border_width;
	xwc.sibling		= e->above;
	xwc.stack_mode		= e->detail;
	XConfigureWindow(display, e->window, e->value_mask, &xwc);

	XSync(display, False);
}


void configure_notify_handler(XConfigureEvent *e)
{
	LOG("window %lu", e->window);

	print_types(e->window);

	XSync(display, False);
}


void map_request_handler(XMapRequestEvent *e)
{
	LOG("window %lu", e->window);
	//Window frame = containerize_window(e->window);


/*
	XSetWindowAttributes at;
	at.event_mask = SubstructureRedirectMask |
			SubstructureNotifyMask |
			StructureNotifyMask |
			ExposureMask |
			KeyPressMask |
			KeyReleaseMask |
			// fucking mouse;
			PointerMotionMask |
			ButtonPressMask |
			ButtonReleaseMask |

			EnterWindowMask |
			LeaveWindowMask |
			FocusChangeMask |
			PropertyChangeMask;

	XChangeWindowAttributes(display, e->window, CWCursor | CWEventMask | NoEventMask, &at);
*/

/*
	XSelectInput(display, e->window,
				//	SubstructureRedirectMask |
				//	SubstructureNotifyMask |
					StructureNotifyMask |
					PropertyChangeMask |
					EnterWindowMask |
					LeaveWindowMask |
					FocusChangeMask
	);


	XSetWindowBorderWidth(display, e->window, BORDER_WIDTH);
	XSetWindowBorder(display, e->window, BORDER_COLOR);
*/

	//XReparentWindow(display, e->window, root, 0, 0);
	app_add(e->window);


	XReparentWindow(display, e->window, root, 0, 0);	//revert to pointer root


//	setup_window(e->window);
	map_windows();


	XSetWMProtocols(display, e->window, &wmdeletewindow, 1);


	XMapWindow(display, e->window);
	focus_window(e->window);
}


void map_notify_handler(XMapEvent *e)
{
	LOG("window %lu", e->window);
	// can setup everything;
//	XSetWindowBorderWidth(display, e->window, BORDER_WIDTH);
//	XSetWindowBorder(display, e->window, BORDER_COLOR);
}


void unmap_notify_handler(XUnmapEvent *e)
{
	LOG("window %lu", e->window);
	XUnmapWindow(display, e->window);
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
	// ?????? stuck
}


void destroy_notify_handler(XEvent *ev)
{
	XDestroyWindowEvent *e = &ev->xclient;
	LOG("window %lu", e->window);

	app_remove(e->window);

	map_windows();

	XDestroyWindow(display, e->window);
}

void reparent_notify_handler(XReparentEvent *e)
{
	LOG("reparent window %d", e->window);
}




void init_keys()
{
	XUngrabKey(display, AnyKey, AnyModifier, root);

	grab_key(XK_0, KeyMask);
	grab_key(XK_Left, KeyMask);
	grab_key(XK_Right, KeyMask);

	XGrabButton(display, Button1, KeyMask, root, False, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, KeyMask, root, False, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
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


#define XTOP		0
#define XLEFT		0
#define XBOTTOM		0
#define XRIGHT		0
#define GAP		0

void map_windows()
{
	size_t wid = WIDTH;
	size_t hei = HEIGHT;

	size_t BW = BORDER_WIDTH;

	size_t x = 0 - BW + GAP;
	size_t y = 0 - BW + GAP;

	size_t i = 1;

	app_t* ptr = first->next;
	for (; ptr != first; ptr = ptr->next)
	{
		LOG("mapping %d", ptr->window);

		/*if (i != 0)
		{
			if (i % 2 == 0) wid = wid / 2;
			else hei = hei / 2;
		}*/

		x = WIDTH - wid;
		y = HEIGHT - hei;

		if (i != win_count)
		{
			wid = (i % 2 != 0) ? wid / 2: wid;
			hei = (i % 2 == 0) ? hei / 2: hei;
		}


		LOG("resizing window %d | i %d | win_count %d", ptr->window, i, win_count);

		XMoveResizeWindow(display, ptr->window, x + BW + XLEFT, y + BW + XTOP, (wid - BW*2) - XLEFT - XRIGHT, (hei - BW*2) - XTOP - XBOTTOM);

		i++;
	}
}

void setup_window(Window win)
{

	XWindowAttributes xwa;
	XGetWindowAttributes(display, win, &xwa);

	LOG("window %d setup | X %d | Y %d | Width %d | Height %d", win, xwa.x, xwa.y, xwa.width, xwa.height);

//	XResizeWindow(display, win, WIDTH, HEIGHT);

//	XSelectInput(display, win, SubstructureRedirectMask | SubstructureNotifyMask);
//	XReparentWindow(display, win, frame, 0, 0);
}











Atom get_atom(Window window, Atom at) {
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;
	if (XGetWindowProperty(display, window, at, 0L, sizeof atom, False, XA_ATOM, &da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}

void print_types(Window window) {
	Atom type = get_atom(window, netwmtype);
	if (type == netdesktop) LOG("_NET_WM_WINDOW_TYPE_DESKTOP %d", type);
	if (type == netdock) LOG("_NET_WM_WINDOW_TYPE_DOCK %d", type);
	if (type == nettoolbar) LOG("_NET_WM_WINDOW_TYPE_TOOLBAR %d", type);
	if (type == netmenu) LOG("_NET_WM_WINDOW_TYPE_MENU %d", type);
	if (type == netutility) LOG("_NET_WM_WINDOW_TYPE_UTILITY %d", type);
	if (type == netsplash) LOG("_NET_WM_WINDOW_TYPE_SPLASH %d", type);
	if (type == netdialog) LOG("_NET_WM_WINDOW_TYPE_DIALOG %d", type);
	if (type == netnormal) LOG("_NET_WM_WINDOW_TYPE_NORMAL %d", type);
}

Window get_focused()
{

	Window winfoc;
	int rev_foc;
	XGetInputFocus(display, &winfoc, &rev_foc);
	return winfoc;
}

void focus_window(Window win)
{
	LOG("window %d", win);
	XRaiseWindow(display, win);
	XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime);
}

void focus_next(Window win)
{
	LOG("window %d", win);
	for (app_t *ptr = first->next; ptr != first; ptr = ptr->next)
	{
		if (ptr->window == win && ptr->next->window != 0) focus_window(ptr->next->window);
	}
}

void focus_prev(Window win)
{
	LOG("window %d", win);
	for (app_t *ptr = first->next; ptr != first; ptr = ptr->next)
	{
		if (ptr->window == win && ptr->prev->window != 0) focus_window(ptr->prev->window);
	}
}

void update_borders(Window win)
{
	Window fcs = get_focused();

	XSetWindowBorderWidth(display, win, BORDER_WIDTH);
	XSetWindowBorder(display, win, fcs == win ? BORDER_COLOR_FOCUSED : BORDER_COLOR);
}









/*
Window containerize_window(Window win)
{
	XWindowAttributes xwa;
	XGetWindowAttributes(display, win, &xwa);

	Window frame = XCreateSimpleWindow(display, root, xwa.x, xwa.y, xwa.width, xwa.height, BORDER_WIDTH, BORDER_COLOR, BACKGROUND_COLOR);
	XSelectInput(display, frame, SubstructureRedirectMask | SubstructureNotifyMask);
	XReparentWindow(display, win, frame, 0, 0);
	return frame;
}
*/

