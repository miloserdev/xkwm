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






#define KeyMask		Mod4Mask

#define USE_FUCKING_MOUSE			// to use fucking mouse;

Window xtask;

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


uint32_t _RGB(r, g, b)	{ return b + (g << 8) + (r << 16); }

#define	BORDER_WIDTH		3
#define	BORDER_COLOR		_RGB(20, 20, 20)
#define	BORDER_COLOR_FOCUSED	_RGB(255, 100, 100)

typedef struct app_t
{
	struct app_t *prev;
	Window window;
	size_t monitor;
	Atom type;
	struct app_t *next;
} app_t;


#define MONITORS_COUNT	8
typedef struct monitor_t
{
	uint32_t number;
	app_t *first;
	app_t *last;
	Window last_focused;
	size_t win_count;
} monitor_t;
monitor_t *mons[MONITORS_COUNT];
int32_t current_mon = 0;





Window get_focused();
void focus_window(monitor_t* mon, Window win);
void focus_prev(monitor_t *mon, Window win);
void focus_next(monitor_t *mon, Window win);

void change_mon(int32_t mon);

void update_borders(Window win);

bool monitor_init();

/*
bool app_init()
{
	last = (app_t*) malloc(sizeof(app_t));
	last->window = 0;
	last->monitor = 99;
	last->prev = last;
	last->next = last;

	first = last;

//	win_count++;
	LOG("apps init | first window %d | first`s next %d", first->window, first->next->window, first->prev->window);
}
*/

bool app_list();


bool app_add(Window win);

//#define app_iterate(code)	{ app_t *ptr = first; for (;;) { ptr = ptr->next; code if (ptr == first) return; } }

bool app_remove(Window win);


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
void hide_windows(int32_t mon);
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

	monitor_init();

//	app_init();

	event_loop();

	XCloseDisplay(display);
	exit(0);
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

	kz(0);

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

/**/	font = XLoadFont(display, "-*-*-*-R-Normal---*-18-100-100-*-*");
//	if (font == NULL) exit(-1);


	XSetWindowAttributes at;
	at.event_mask = SubstructureRedirectMask |
			SubstructureNotifyMask |
			StructureNotifyMask |
			ExposureMask |
			KeyPressMask |
			KeyReleaseMask |

		#ifdef USE_FUCKING_MOUSE
			PointerMotionMask |
			ButtonPressMask |
			ButtonReleaseMask |
		#endif
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

	XSync(display, False);
	XFlush(display);
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
	monitor_t *mon = mons[current_mon];

	LOG("key_release | window %lu | key %d | mask %d", e->window, e->keycode, _mask);

	switch(key)
	{
		case XK_0: { if (_mask == KeyMask) run_prog("dmenu_run"); break; }

		case XK_Down:	{ focus_prev(mon, fcs); break; }
		case XK_Up:	{ focus_next(mon, fcs); break; }

		case XK_Left:	{ change_mon(current_mon - 1); break; }
		case XK_Right:	{ change_mon(current_mon + 1); break; }

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
	monitor_t *mon = mons[current_mon];
	LOG("window %lu | monitor %d", e->window, mon->number);

	//XReparentWindow(display, e->window, root, 0, 0);
	app_add(e->window);

	XSetWMProtocols(display, e->window, &wmdeletewindow, 1);

//	XReparentWindow(display, e->window, root, 0, 0);	//revert to pointer root

//	setup_window(e->window);
	map_windows();

	XMapWindow(display, e->window);
	focus_window(mon, e->window);
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
	monitor_t *mon = mons[current_mon];
	LOG("window %lu | monitor %d", e->window, mon->number);

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

	grab_key(XK_Down, KeyMask);
	grab_key(XK_Up, KeyMask);

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


void hide_windows(int32_t monitor)
{
	monitor_t *mon = mons[monitor];
	app_t *first = mon->first;
	app_t *last = mon->last;
	size_t win_count = mon->win_count;

	for (app_t *ptr = first->next; ptr != first; ptr = ptr->next)
	{
		XMoveWindow(display, ptr->window, WIDTH + 100, HEIGHT + 100);
	}
}


#define XTOP		0
#define XLEFT		0
#define XBOTTOM		0
#define XRIGHT		0
#define GAP		10

void map_windows()
{
	size_t wid = WIDTH;
	size_t hei = HEIGHT;

	size_t BW = BORDER_WIDTH + GAP;

	size_t x = 0;// - BW + GAP;
	size_t y = 0;// - BW + GAP;

	size_t i = 1;

	monitor_t *mon = mons[current_mon];
	app_t *first = mon->first;
	app_t *last = mon->last;
	size_t win_count = mon->win_count;

	app_t *ptr = first->next;
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

		XMoveResizeWindow(display, ptr->window, x + XLEFT + GAP, y + XTOP + GAP, (wid - BW*2) - XLEFT - XRIGHT, (hei - BW*2) - XTOP - XBOTTOM);

		i++;
	}
	focus_window(mon, mon->last_focused);
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

void focus_window(monitor_t *mon, Window win)
{
	LOG("window %d", win);
	if (win == 0)
	{
		LOG("cannot focus zero %d", win);
		return;
	}
	mon->last_focused = win < root ? root : win;
	XRaiseWindow(display, mon->last_focused);
	XSetInputFocus(display, mon->last_focused, RevertToPointerRoot, CurrentTime);
}

/*
void focus_return(monitor_t *mon)
{
//	return;
	for (app_t *ptr = mon->last; ptr != mon->first; ptr = ptr->prev)
	{
		if (ptr->window == mon->last_focused)
		{
			focus_window(mon, ptr->prev);
			mon->last_focused = ptr->window;
		}
	}
}
*/

void focus_next(monitor_t *mon, Window win)
{
	LOG("window %d", win);

	for (app_t *ptr = mon->first->next; ptr != mon->first; ptr = ptr->next)
	{
		if (ptr->window == win && ptr->next->window != 0) { focus_window(mon, ptr->next->window); return; }
	}
}

void focus_prev(monitor_t *mon, Window win)
{
	LOG("window %d", win);

	for (app_t *ptr = mon->first->next; ptr != mon->first; ptr = ptr->next)
	{
		if (ptr->window == win && ptr->prev->window != 0) { focus_window(mon, ptr->prev->window); return; }
	}
}


void change_mon(int32_t mon)
{
	LOG("trying %d", mon);

	if (mon >= 0 && mon < MONITORS_COUNT)
	{
		LOG("setup monitor %d", mon);
		hide_windows(current_mon);
		current_mon = mon;
		map_windows();
	} else
	{
		LOG("buffer fucked up %d", mon);
	}
}


void update_borders(Window win)
{
	Window fcs = get_focused();

	XSetWindowBorderWidth(display, win, BORDER_WIDTH);
	XSetWindowBorder(display, win, fcs == win ? BORDER_COLOR_FOCUSED : BORDER_COLOR);
}

bool monitor_init()
{
	for (size_t i = 0; i < MONITORS_COUNT; i++)
	{
		mons[i] = (monitor_t*) malloc(sizeof(monitor_t));
		mons[i]->number = i;

		mons[i]->last = (app_t*) malloc(sizeof(app_t));
		mons[i]->last->window = 0;
		mons[i]->last->monitor = i;
		mons[i]->last->prev = mons[i]->last;
		mons[i]->last->next = mons[i]->last;
		mons[i]->first = mons[i]->last;

		mons[i]->last_focused = root;
		mons[i]->win_count = 0;
		LOG("monitor %d init | first window %d | first`s next %d", mons[i]->number, mons[i]->first->window, mons[i]->first->next->window, mons[i]->first->prev->window);
	}
}

bool app_list()
{
	LOG("", NULL);

	monitor_t *mon = mons[current_mon];
	app_t *first = mon->first;
	app_t *last = mon->last;

	for (app_t *ptr = first->next; ptr != first; ptr = ptr->next)
	{
		LOG("app iterator | prev %d | window %d | next %d ", ptr->prev->window, ptr->window, ptr->next->window);
	}
	LOG("", NULL);
}

bool app_add(Window win)
{
	monitor_t *mon = mons[current_mon];

	app_t *first = mon->first;
	app_t *last = mon->last;

	app_t* tmp = (app_t*) malloc(sizeof(app_t));
	tmp->window = win;
	tmp->monitor = current_mon;
	tmp->next = first;
	tmp->prev = last;
	tmp->type = get_atom(win, netwmtype);

	mon->last->next = tmp;
	mon->last = tmp;

	LOG("v chem raznitsa | window %d | monitor %d | last window & %d | last window %d | tmp window %d", win, mon->number, &mon->last->window, mon->last->window, tmp->window);

	mon->win_count++;
	LOG("prog %d | type %d | added as last | last`s prev %d | last is %d | last`s next %d", tmp->window, tmp->type, last->prev->window, last->window, last->next->window);
	app_list();
}

bool app_remove(Window win)
{
	LOG("", NULL);
	for (size_t i = 0; i < MONITORS_COUNT; i++)
	{
		monitor_t *mon = mons[i];
		size_t win_count = mon->win_count;

		LOG("monitor %d | first %d | last %d | win_count %d", mon->number, mon->first->window, mon->last->window, win_count);

		app_t *ptr = mon->first->next;
		for (; ptr != mon->first; ptr = ptr->next)
		{
			LOG("iterator prev %d | current %d | next %d ", ptr->prev->window, ptr->window, ptr->next->window);
			if (ptr->window == win)
			{
				LOG("removing %d | at monitor %d", ptr->window, ptr->monitor);

	//			monitor_t *mon = mons[ptr->monitor];

				ptr->prev->next = ptr->next;
				ptr->next->prev = ptr->prev;

				mon->last = (ptr->next != mon->first) ? ptr->next : ptr->prev;
				mon->last_focused = mon->last->window;		///	FIX | very good hack :)		;;;

				if (mon->win_count > 0) mon->win_count--;

				app_list();
				return;
			}
		}
	}
}
