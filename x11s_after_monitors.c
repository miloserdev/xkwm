/*
 ============================================================================
 Name        : x11s.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

//#include <bits/signum-arch.h>
//#include <bits/signum-generic.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
//#include <X11/Xft/Xft.h>

#include <stdbool.h>

#define try bool __HadError = false;
#define catch(x) ExitJmp:if(__HadError)
#define throw(x) {__HadError=true; goto ExitJump;}


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


static int (*xerrorxlib)(Display*, XErrorEvent*);

static int xerrorstart(Display *display, XErrorEvent *e) {
	printf("another instance\n", -1);
	return -1;
}

static int xerror(Display *display, XErrorEvent *e) {
	printf("request code: %d, error code: %d\n", e->request_code,
			e->error_code);
	return xerrorxlib(display, e);
}
static int xerrordummy(Display *display, XErrorEvent *e) {
	printf("request code: %d, error code: %d...idle...   \n", e->request_code,
			e->error_code);
}



static Display *display;
static Window root;
int rootw;

static Window checkwin;


int min_width = 200, min_height = 100;
int border_width = 3;
bool fs_border = false;
int border_color;

unsigned long black, white, red, green, blue;

unsigned long _RGB(int r, int g, int b) {
	return b + (g << 8) + (r << 16);
}

Font font;
Cursor cursor;
Cursor cursor_move;
Cursor cursor_size;

static int screen_def;
unsigned int sw;
unsigned int sh;

bool alt_pressed = false;
bool shift_pressed = false;
bool ctrl_pressed = false;

int num_monitors = 3;
int curr_monitor = 1;






typedef struct monitor {
	int id;
	Window last_focused;

	struct monitor *next;
	struct monitor *prev;
} monitor;

monitor *last_mon, *first_mon;

monitor * add_mon(int id) {
	struct monitor *obj;

	obj = (struct monitor*) malloc( sizeof ( struct monitor ) );

	obj->id = id;
	obj->last_focused = root;
	obj->next = first_mon;
	obj->prev = last_mon;

	last_mon->next = obj;
	last_mon = last_mon->next;

	printf("________FUNC_%s________________PROG_ADDED____ID_%d____WINDOW_%d______\n", __FUNCTION__, last_mon->id, last_mon->last_focused);

//	founds(__FUNCTION__);

	return obj;
}



monitor * get_mon(int id) {
	printf("________FUNC_%s_____________________________________________MONITOR_REQUEST______________MONITOR_NUMBER_%d______\n", __FUNCTION__, id);

	monitor *mon = first_mon;
	printf("________FUNC_%s_____________________________________________MONITOR_REQUEST______________MONITOR_ID_%d______\n", __FUNCTION__, mon->id);

	for(int i = 0; i < num_monitors; i++) {
		printf("________FUNC_%s_____________________________________________MONITOR_REQUEST__ITERATION___MONITOR_ID_%d______\n", __FUNCTION__, mon->id);
		if( (int) mon->id == (int) id + 1) return &(*mon);
		mon = mon->next;
	}
	return last_mon;
}



typedef struct program {
	Window window;
	XWindowAttributes xwa;
	int id;
	int monitor;
	bool raised;
	bool fullscreen;

	int x, y;
	int x_fs, y_fs, wid_fs, hei_fs;
	int x_rs, y_rs, wid_rs, hei_rs;

	int width, height;

	struct program *next;
	struct program *prev;
} program;

struct program *last, *first;

int prog_count = 0;

void init_progs() {
	last = (struct program*) malloc( sizeof ( struct program ) );
	last->next = last;
	last->prev = last;
	first = last;
}



program * get_prog(Window win) {
	program *obj = first;
	for(int i = 0; i < prog_count; i++) {
		if ( (int) obj->window == (int) win) return &(*obj);
		obj = obj->next;
	}
	return NULL;
}




void founds(char *fn) {
	printf("________FUNC_%s______________________________________________________________________________________________________FOUNDS_BEGIN________\n", fn);

	program *prog = first;
	for(int i = 0; i < prog_count; i++) {
		printf("________FUNC_%s_______________FOUND__ID_%d_WINDOW_%d__X_%d_Y_%d_____Xrs_%d_Yrs_%d____Xfs_%d_Yfs_%d____MONITOR_%d___\n",
				fn, prog->id, prog->window, prog->x, prog->y, prog->x_fs, prog->y_fs, prog->x_rs, prog->y_rs, prog->monitor);
		//if (prog->next != NULL) 
		prog = prog->next;
	}
	printf("________FUNC_%s_________________________________________________________________________________ROOT_%d_________ROOTW_%d_______________PROG_COUNT_%d\n", fn, root, rootw, prog_count);
}



program * add_prog(Window win) {
	struct program *obj;

	obj = (struct program*) malloc( sizeof ( struct program ) );

	obj->id = prog_count;
	obj->window = win;
	obj->next = first;
	obj->prev = last;

	last->next = obj;
	last = last->next;

	printf("________FUNC_%s________________PROG_ADDED____ID_%d____WINDOW_%d______\n", __FUNCTION__, last->id, last->window);

	founds(__FUNCTION__);

	prog_count++;

	return obj;
}



void *rem_prog(Window win) {

	printf("________FUNC_%s________________________________________TRYING_TO_REMOVE_WINDOW_%d______\n", __FUNCTION__, win);


	program *obj = first;
	for(int i = 0; i <= prog_count; i++) {
		if ( (int) obj->window == (int) win) {
		//	if(obj->window == root) return;
			obj->next->prev = obj->prev;
			obj->prev->next = obj->next;
			last = obj->next;
			prog_count--;
			printf("________FUNC_%s________________________________________REMOVED_WINDOW_%d______WINDOW_%d_________________________\n", __FUNCTION__, win, obj->window);
			break;
		}
		obj = obj->next;
	}
	//free(obj);
}











void do_not_go_fucking_out(Window win) {
	int fucked_up = 0;
	program *prog = get_prog(win);
	if(prog->monitor == curr_monitor) {
		XWindowAttributes xwa;
		XGetWindowAttributes(display, win, &xwa);

		if (xwa.width < min_width || xwa.height < min_height) {
			XResizeWindow(display, win, min_width, min_height);
			fucked_up = 1;
		}

		if(prog->raised && (xwa.x >= sw || xwa.x < 0   ||   xwa.y >= sh || xwa.y < 0) ) {
			int x = (sw / 2) - (xwa.width / 2);
			int y = (sh / 2) - (xwa.height / 2);
			XMoveWindow(display, win, x, y);
			fucked_up = 1;
		}
		if (fucked_up) printf("________FUNC_%s________________________________________WINDOW_%d______IS_FUCKED_UP_________________________\n", __FUNCTION__, win, prog->window);

	}
}


program * add_prog_with_check(Window win) {

	bool exist = false;

	program *obj = first;
	for(int i = 0; i < prog_count; i++) {
		if ( (int) obj->window ==  (int) win) {
			exist = true;
			break;
		} else {
		}
		obj = obj->next;
	}

	printf("________FUNC_%s___________________________________________________BOOL_EXIST___%b\n", __FUNCTION__, exist);

	if (!exist) {

		struct program *prog;

		prog = (struct program*) malloc( sizeof ( struct program ) );

		prog->id = prog_count;
		prog->window = win;
		prog->next = first;
		prog->prev = last;

		last->next = prog;
		last = last->next;

		printf("________FUNC_%s________________PROG_ADDED____ID_%d____WINDOW_%d______\n", __FUNCTION__, last->id, last->window);

		XWindowAttributes xwa;
		XGetWindowAttributes(display, win, &xwa);

		prog->xwa = xwa;
		prog->monitor = curr_monitor;
		prog->raised = true;

		prog->x = xwa.x;
		prog->y = xwa.y;

		prog->x_rs = 0;
		prog->y_rs = 0;

		prog->x_fs = 0;
		prog->y_fs = 0;

		prog->width = xwa.width;
		prog->height = xwa.height;
		XSync(display, False);

		founds(__FUNCTION__);

		prog_count++;

		return prog;
	}
	return obj;
}


Atom netwmstate, netwmfs, netwmtype, netwmdialog;


Atom AtomUTF8, AtomNetSupported, AtomNetWMName, AtomNetWMState, AtomNetWMCheck, AtomNetWMFullscreen, AtomNetActiveWindow, AtomNetWMWindowType, AtomNetWMWindowTypeDialog, AtomNetClientList;


Atom AtomWMProtocols, AtomWMDelete, AtomWMState, AtomWMTakeFocus;

Atom get_atom(Window window, Atom at) {
	printf("________FUNC_%s__________\n", __FUNCTION__);

	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;
	if (XGetWindowProperty(display, window, at, 0L, sizeof atom, False, XA_ATOM,
			&da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom*) p;
		XFree(p);
	}
	return atom;
}



void change_monitor(int numb) {
	printf("________FUNC_%s_____PROG_COUNT_____\n", __FUNCTION__, prog_count);

	curr_monitor = numb;
	program *prog = first;
//	monitor *mon = first_mon;


//	XSetInputFocus(display, root, 0, CurrentTime);

	for (int i = 0; i < prog_count; i++) {

		printf("______________________CHANGINGMON______ITERATION_____WIN_%d", prog->window);

//		if (prog->next != NULL) prog = prog->next;
		if ( (int) prog->window != root || (int) prog->window != rootw) {
			if (prog->monitor != curr_monitor) {

				if (prog->raised) {
					XWindowAttributes xwa;
					XGetWindowAttributes(display, prog->window, &xwa);
					prog->x_rs = xwa.x;
					prog->y_rs = xwa.y;
					prog->width = xwa.width;
					prog->height = xwa.height;
				}

				XMoveResizeWindow(display, prog->window, prog->x,
						(1080 * 2) + 100, prog->width, prog->height);
				prog->raised = false;
			} else {
				if (!prog->raised) {
					XMoveResizeWindow(display, prog->window, prog->x_rs, prog->y_rs,
							prog->width, prog->height);
					prog->raised = true;
//					printf("________________TRYMONITOR______%d________%d", mon->id, mon->last_focused);
					XSetInputFocus(display, prog->window, 0, CurrentTime);
				}
//				mon = get_mon(prog->monitor);
//				XSetInputFocus(display, mon->last_focused, 0, CurrentTime);
			}
		}
		prog = prog->next;
	}
}

// #fullscreen

void fullscreen(Window w) {

	if (w == root) return;

	printf("________FUNC_%s__________PROG%d\n", __FUNCTION__, w);

	program *prog = get_prog(w);

	if(prog->window == first) return;

	if ( (int) prog->window != 0 && (int) prog->window != rootw) {

		XWindowAttributes xwa;
		XGetWindowAttributes(display, prog->window, &xwa);

		if (prog->monitor == curr_monitor) {
			if (!prog->fullscreen) {

				if (!prog->fullscreen) {
					prog->x_fs = xwa.x;
					prog->y_fs = xwa.y;
					prog->wid_fs = xwa.width;
					prog->hei_fs = xwa.height;
				}

				int wid = fs_border ? sw - border_width : sw;
				int hei = fs_border ? sh - border_width : sh;

//				Atom netwmfs = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
//				XChangeProperty(display, prog->window, netwmfs, XA_ATOM, 32, PropModeReplace, (unsigned char*) &netwmfs, 1);

				XSetWindowBorderWidth(display, prog->window,
						(fs_border ? border_width : 0));
				XMoveResizeWindow(display, prog->window, 0, 0, wid, hei);
				prog->fullscreen = true;
			} else {
//				Atom netwmstate = XInternAtom(display, "_NET_WM_STATE", False);
//				XChangeProperty(display, prog->window, netwmstate, XA_ATOM, 32, PropModeReplace, (unsigned char*) &netwmstate, 1);

				XSetWindowBorderWidth(display, prog->window, border_width);
				XMoveResizeWindow(display, prog->window, prog->x_fs, prog->y_fs,
						prog->wid_fs, prog->hei_fs);
				prog->fullscreen = false;
			}
		}
	}
}

Window under_pointer() {
	printf("________FUNC_%s__________\n", __FUNCTION__);

	int ret, pos;
	unsigned int mask;
	Window child, contnr;
	ret = XQueryPointer(display, root, &contnr, &child, &pos, &pos, &pos, &pos,
			&mask);
	if (!ret)
		return 0;
	return ret;
}

void mkill(Window window) {
	printf("DEBUG X11 func: %s window: %d\n", __FUNCTION__, window);

	XEvent event;
	event.xclient.type = ClientMessage;
	event.xclient.window = window;
	event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
	event.xclient.format = 32;
	event.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
	event.xclient.data.l[1] = CurrentTime;
	int result = XSendEvent(display, window, False, NoEventMask, &event);
	XKillClient(display, window);
}

void handle_expose(XExposeEvent *e) {
	printf("\nDEBUG X11 func: %s window: %d\n", __FUNCTION__, e->window);

	founds(__FUNCTION__);

//	add_prog_with_check(e->window);

	do_not_go_fucking_out(e->window);

	XSetInputFocus(display, e->window, 0, CurrentTime);
	XSync(display, False);
}

#define CLEANMASK(mask) (mask & ~(LockMask) & (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))


/*void handle_key_press_release(XEvent e) {
	XKeyEvent *event = e->xkey;
}*/


void handle_key_press(XKeyEvent *e) {

	printf("________FUNC_%s__________SWITCH\n", __FUNCTION__);

	int _mask = (e->state & (ShiftMask | ControlMask | Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask));

	switch (XKeycodeToKeysym(display, e->keycode, 0)) {



	Window winfocus;
	int revert_focus;

	case XK_7:

		founds(__FUNCTION__);

//		XSetWindowAttributes attrs;
//		attrs.event_mask = ExposureMask | FocusChangeMask | KeyPressMask
//				| StructureNotifyMask | VisibilityChangeMask | ExposureMask;
//		XChangeWindowAttributes(display, e->window, 0, &attrs);
//		XMapWindow(display, e->window);
		break;
	case XK_Alt_L:
		alt_pressed = true;
		break;
	case XK_Shift_L:
		shift_pressed = true;
		break;
	case XK_Control_L:
		ctrl_pressed = true;
		break;
	case XK_Return:
		if (e->state & Mod1Mask)
			fullscreen(e->window);
		break;
	case XK_5:
		if (e->state & Mod1Mask) {

			int gap = 10;
			program *prog = first;

			int _sw = sw, _sh = sh;
			int _bw = border_width + gap;
			int _x = 0 - _bw + gap, _y = 0 - _bw + gap;

			for (int i = 0; i <= prog_count; i++) {

			//	if (prog->next == NULL) return;
//				printf("prog = prog->next;");
				if(prog->window !=root && prog->monitor == curr_monitor) {
				//	#stack


				/*	XWindowAttributes xwa;
					XGetWindowAttributes(display, prog->window, &xwa);
					prog->x_rs = xwa.x;
					prog->y_rs = xwa.y;
					prog->width = xwa.width;
					prog->height = xwa.height;
				*/

						if (i % 2 != 0) {
						//	y = sh * 2;
							_sw = _sw / 2;
							if (i >= prog_count-1) _sw = _sw * 2;
						} else {
						//	x = sw * 2;
							_sh = _sh / 2;
							if (i >= prog_count-1) _sh = _sh * 2;
						}

						XMoveResizeWindow(display, prog->window, _x + _bw, _y + _bw, _sw - _bw*2, _sh - _bw*2);
//						XMoveResizeWindow(display, prog->window, _x + _bw, _y + _bw, _sw - _bw*2, _sh - _bw*2);

						if (i % 2 != 0) {
							_x = _x + _sw;
						} else {
							_y = _y + _sh;
						}


				}
				prog = prog->next;
			}
		}
		break;
	case XK_1:
		if (e->state & Mod1Mask) {
			if ( _mask == (Mod1Mask | ControlMask) ) {
				XGetInputFocus(display, &winfocus, &revert_focus);
				if (winfocus == root || winfocus == rootw) break;
				program *prog = get_prog(winfocus);
				prog->monitor = 1;
			//	monitor *mon = get_mon(prog->monitor);
			//	mon->last_focused = winfocus;
				change_monitor(curr_monitor);
			} else {
				change_monitor(1);
			}
		}		break;
	case XK_2:
		if (e->state & Mod1Mask) {
			if ( _mask == (Mod1Mask | ControlMask) ) {
				XGetInputFocus(display, &winfocus, &revert_focus);
				if (winfocus == root || winfocus == rootw) break;
				program *prog = get_prog(winfocus);
				prog->monitor = 2;
			//	monitor *mon = get_mon(prog->monitor);
			//	mon->last_focused = winfocus;
				change_monitor(curr_monitor);
			} else {
				change_monitor(2);
			}
		}		break;
	case XK_3:
		if (e->state & Mod1Mask) {
			if ( _mask == (Mod1Mask | ControlMask) ) {
				XGetInputFocus(display, &winfocus, &revert_focus);
				if (winfocus == root || winfocus == rootw) break;
				program *prog = get_prog(winfocus);
				prog->monitor = 3;
			//	monitor *mon = get_mon(prog->monitor);
			//	mon->last_focused = winfocus;
				change_monitor(curr_monitor);
			} else {
				change_monitor(3);
			}
		}
		break;
	case XK_0:
		if (e->state & Mod1Mask) {
			if (fork() == 0) {
				if (display)
					close(ConnectionNumber(display));
				char *cmd[] = { "dmenu_run", NULL };
				setsid();
				execvp(cmd[0], cmd);
				perror("failed");
				exit(EXIT_SUCCESS);
			}
		}
		break;

	case XK_F4:
		if (&e->state && Mod1Mask) {
			XEvent event;
			event.xclient.type = ClientMessage;
			event.xclient.window = e->window;
			event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS",
					True);
			event.xclient.format = 32;
			event.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW",
					False);
			event.xclient.data.l[1] = CurrentTime;
			int result = XSendEvent(display, e->window, False, NoEventMask,
					&event);
			XKillClient(display, e->window);
		}
		break;
	}

	printf("________FUNC_%s__________SWITCH__________SHIFT_%b_________CONTROL_%b________ALT_%b_______\n", __FUNCTION__, shift_pressed, ctrl_pressed, alt_pressed);

}

void handle_key_release(XKeyEvent *e) {
	printf("________FUNC_%s__________\n", __FUNCTION__);

	//XkbKeycodeToKeysym(display, e->keycode, 0, 0)
	switch (XKeycodeToKeysym(display, e->keycode, 0)) {
	case XK_Alt_L:
		alt_pressed = false;
		break;
	case XK_Shift_L:
		shift_pressed = false;
		break;
	case XK_Control_L:
		ctrl_pressed = false;
		break;
	}
}

void handle_configure_request(XConfigureRequestEvent *e) {

	printf("DEBUG X11 func: %s window: %d\n", __FUNCTION__, e->window);


/////////////////////Every time updates////////////////////////

//	monitor *mons = last_mon;
//	for (int i = 1; i < num_monitors; i++) {
//		printf("________FUNC_%s________________MONITOR_ID_%d____LAST_FOCUSED_%d______\n", __FUNCTION__, mons->id, mons->last_focused);
//		mons = mons->next;
//	}


	XSetWindowAttributes attrs;

//	XWindowAttributes xwa;
//	XGetWindowAttributes(display, e->window, &xwa);
//
//	Program *prog = get_program(e->window);

	XWindowAttributes xwa;
	XGetWindowAttributes(display, e->window, &xwa);
//	XMoveResizeWindow(display, e->window, xwa.x, xwa.y, xwa.width, xwa.height);

	long msize;
	XSizeHints size;
	if (!XGetWMNormalHints(display, e->window, &size, &msize)) {
		size.flags = PSize;
	}

	Atom netwmstate = XInternAtom(display, "_NET_WM_STATE", False);
	Atom netwmfs = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
	Atom netwmtype = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	Atom netwmdialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG",
			False);

	Atom state = get_atom(e->window, netwmstate);
	Atom type = get_atom(e->window, netwmtype);

	if (state == netwmfs) {
//		fullscreen(e->window);
	}
	if (type != netwmdialog) {
	}


	XSync(display, False);

}

void handle_configure_notify(XConfigureEvent *e) {

	printf("DEBUG X11 func: %s window: %d\n", __FUNCTION__, e->window);


	XWindowAttributes xwa;
	XGetWindowAttributes(display, e->window, &xwa);

	Atom netwmstate = XInternAtom(display, "_NET_WM_STATE", False);
	Atom netwmfs = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
	Atom netwmtype = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	Atom netwmdialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG",
			False);

	Atom state = get_atom(e->window, netwmstate);
	Atom type = get_atom(e->window, netwmtype);

	if (state == netwmfs) {
//		fullscreen(e->window);
	}
	if (type == netwmdialog) {
//		XMoveResizeWindow(display, e->window, 1200, 200, 500, 500);
	}

//	XReparentWindow(display, e->window, e->window, 0, 0);
//	XSetWindowBorderWidth(display, e->window, ( ( (xwa.x == 0 && xwa.y == 0) && (xwa.width >= 1920 && xwa.height >= 1080 ) ) ? 0 : border_width ) );
	XSync(display, False);
}

void handle_map_request(XMapRequestEvent *e) {
	printf("DEBUG X11 func: %s window: %d\n", __FUNCTION__, e->window);

	founds(__FUNCTION__);

	Pixmap color = 0x00fefe;


//	XGrabKey(display, XKeysymToKeycode(display, XK_Alt_L), AnyModifier, e->window, False, GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_Shift_L), Mod1Mask, e->window, False, GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_1), Mod1Mask, e->window, False, GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_2), Mod1Mask | ControlMask, e->window, False, GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_3), Mod1Mask, e->window, False, GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_Control_L), Mod1Mask, e->window, False, GrabModeAsync, GrabModeAsync);




//	XGrabKey(display, XKeysymToKeycode(display, XK_1), Mod4Mask, e->window, False, GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_2), Mod4Mask, e->window, False, GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_3), Mod4Mask, e->window, False, GrabModeAsync, GrabModeAsync);

	/*	XGrabKey(display, XKeysymToKeycode(display, XK_1),
	 XKeysymToKeycode(display, XK_Alt_L), e->window, False,
	 GrabModeAsync, GrabModeAsync);
	 XGrabKey(display, XKeysymToKeycode(display, XK_2),
	 XKeysymToKeycode(display, XK_Alt_L), e->window, False,
	 GrabModeAsync, GrabModeAsync);
	 XGrabKey(display, XKeysymToKeycode(display, XK_3),
	 XKeysymToKeycode(display, XK_Alt_L), e->window, False,
	 GrabModeAsync, GrabModeAsync);
	 */
	XGrabButton(display, Button1, Mod1Mask, e->window, False,
	ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
	GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, Mod1Mask, e->window, False,
	ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
	GrabModeAsync, GrabModeAsync, None, None);

	XWindowAttributes xwa;
	XGetWindowAttributes(display, e->window, &xwa);

	XSetWindowAttributes attrs;
	attrs.event_mask = ExposureMask |
			   FocusChangeMask |
			   KeyPressMask |
			   EnterWindowMask |
			   LeaveWindowMask |
			   StructureNotifyMask |
			   VisibilityChangeMask;

//	attrs.border_pixel = _RGB(255,66,66);
//	attrs.background_pixel = white;
	int x, y, width, height;
//	XGetWindowAttributes(display, e->window, &xwa);
//	XGetGeometry(display, drawable, e->window, &x, &y, &width, &height, &border_width, &depth);

//	width = xwa.width < min_width ? min_width : xwa.width;
//	height = xwa.height < min_height ? min_height : xwa.height;

	XSetWindowBorderWidth(display, e->window, border_width);
	XSetWindowBorder(display, e->window, border_color);

	XChangeProperty(display, root, AtomNetClientList, XA_WINDOW, 32, PropModeAppend,
			(unsigned char*) &(e->window), 1);

//	XMoveResizeWindow(display, e->window, xwa.x, xwa.y, xwa.width, xwa.height);

	XChangeWindowAttributes(display, e->window, 0, &attrs);

	Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(display, e->window, &delWindow, 1);


	long msize;
	XSizeHints size;
	if (!XGetWMNormalHints(display, e->window, &size, &msize)) {
		size.flags = PSize;
	}

	x = xwa.x < 1 ? (sw / 2) - (xwa.width / 2) : xwa.x;
	y = xwa.y < 1 ? (sh / 2) - (xwa.height / 2) : xwa.y;

	width = MAX(size.base_width, xwa.width);
	height = MAX(size.base_height, xwa.height);

//	width = MAX(size.min_width, width);
//	height = MAX(size.min_height, height);

	printf("________FUNC_%s____________________________________WINDOW_%d___xwa.x_%d___xwa.y_%d___xwa.width_%d___xwa.height_%d____\n", __FUNCTION__, e->window, xwa.x, xwa.y, xwa.width, xwa.height);

	XMoveResizeWindow(display, e->window, x, y, width, height);

	XSetTransientForHint(display, root, e->window);

	Atom netwmstate = XInternAtom(display, "_NET_WM_STATE", False);
	Atom netwmfs = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
	Atom netwmtype = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	Atom netwmdialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG",
			False);

	Atom state = get_atom(e->window, netwmstate);
	Atom type = get_atom(e->window, netwmtype);

	if (state == netwmfs) {
		printf("________FUNC_%s________________CHANGE_STATE_FULLSCREEN____WINDOW_%d______\n", __FUNCTION__, e->window);
	}
	if (type == netwmdialog) {
		printf("________FUNC_%s________________DIALOG_____WINDOW_%d______\n", __FUNCTION__, e->window);
	}


	XGrabServer(display);

	program *prog = add_prog_with_check(e->window);

	printf("_________SYNC_________SYNC_________SYNC_________SYNC_________SYNC_______\n");

	XRaiseWindow(display, e->window);
	XMapWindow(display, e->window);

	if(e->window != root || e->window != rootw) {
		//XSetInputFocus(display, e->window, 0, CurrentTime);
		monitor *mon = get_mon(prog->monitor);
		mon->last_focused = (int) e->window;
	}

	XSync(display, False);
	XUngrabServer(display);
	XSync(display, False);

//	monitor *mon = get_mon(curr_monitor);
//	mon->last_focused = e->window;

}

void handle_button_press(XButtonEvent *e) {

	Window winfocus;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);

	printf("________FUNC_%s_______WINDOW_%d_________WINDOW_FOCUS_%d________REVERT_FOCUS_%d______\n", __FUNCTION__, e->window, winfocus, revert_focus);


//	e->window = winfocus;

//	program *prog_ = get_prog(winfocus);
//	monitor *mon = get_mon(prog_->monitor);
//	mon->last_focused = (int) winfocus;


	XSetInputFocus(display, winfocus, 0, CurrentTime);

	if (alt_pressed) {

		if(e->window == root || e->window == rootw) return;

		program *prog = get_prog(e->window);

		int cx, cy, rx, ry = 0;
		unsigned int maskq = 0;

		XWindowAttributes xwa;
		XGetWindowAttributes(display, e->window, &xwa);

		Window c, r;
		XQueryPointer(display, e->window, &c, &r, &rx, &ry, &cx, &cy, &maskq);

		XRaiseWindow(display, e->window);
		XSetInputFocus(display, e->window, 0, CurrentTime);
		Cursor cur = (e->button == Button1) ? cursor_move : cursor_size;
		XGrabPointer(display, root, True,
				ButtonPressMask | ButtonReleaseMask | ButtonMotionMask
						| PointerMotionMask, GrabModeAsync, GrabModeAsync, None,
				cur, CurrentTime);
		XEvent ev;

		int wid, hei;

		do {

			XMaskEvent(display,
					ButtonPressMask | ButtonReleaseMask | ButtonMotionMask
							| PointerMotionMask | ExposureMask
							| SubstructureRedirectMask, &ev);
			switch (ev.type) {
			case ConfigureRequest:
			case Expose:
			case MapRequest:
				// if (prog->fullscreen) fullscreen(e->window);
				//handle_map_request(&ev);
				break;
			case MotionNotify:
				XSetTransientForHint(display, root, e->window);

				unsigned int mask;

				if (e->button == Button1) {
					XMoveWindow(display, e->window, xwa.x + (ev.xmotion.x - rx),
							xwa.y + (ev.xmotion.y - ry));
				} else {
					wid = ev.xmotion.x - (xwa.x);
					hei = ev.xmotion.y - (xwa.y);
					XResizeWindow(display, e->window,
							(wid < min_width ? min_width : wid),
							(hei < min_height ? min_height : hei));
				}

				break;
			}
		} while (ev.type != ButtonRelease);
		XUngrabPointer(display, CurrentTime);
	}
}

void handle_button_release(XButtonEvent *e) {
	printf("________FUNC_%s__________\n", __FUNCTION__);

}

void handle_motion_notify(XMotionEvent *e) {
//	printf("_FUNC_%s_", __FUNCTION__);

//	XSetInputFocus(display, e->window, 0, CurrentTime);
	XSync(display, False);
}

void handle_unmap_notify(XUnmapEvent *e) {

	printf("DEBUG X11 func: %s | window: %d \n", __FUNCTION__, e->window);


	XGrabServer(display);

//	rem_prog(e->window);

	founds(__FUNCTION__);

	XUnmapWindow(display, e->window);
	XSync(display, False);
	XUngrabServer(display);
	XSync(display, False);

}

void handle_destroy_notify(XDestroyWindowEvent *e) {

	printf("________FUNC_%s__________\n", __FUNCTION__);

	XGrabServer(display);

	rem_prog(e->window);

	mkill(e->window);

	XSync(display, False);
	XUngrabServer(display);
	XSync(display, False);
}


void handle_focus_event(XEvent *e) {
	XFocusChangeEvent *event = &e->xfocus;
	printf("________FUNC_%s____________________________________________________FOCUS_IN_WINDOW_%d____SUBWINDOW_%d_____\n", __FUNCTION__, event->window, 999);
}

void handle_unfocus_event(XEvent *e) {
	XFocusChangeEvent *event = &e->xfocus;
	printf("________FUNC_%s____________________________________________________FOCUS_OUT_WINDOW_%d____SUBWINDOW_%d_____\n", __FUNCTION__, event->window, 999);
}



/*
	Window winfocus;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);

	program *prog_ = get_prog(winfocus);
	monitor *mon = get_mon(prog_->monitor);
	mon->last_focused = (int) winfocus;
*/


void handle_enter_notify(XEvent *e) {
	XCrossingEvent *event = &e->xcrossing;

	Window winfocus;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);

	printf("________FUNC_%s____________________________________________________ENTER_WINDOW_%d____SUBWINDOW_%d___________________WINDOW_FOCUS_%d__________REVERT_FOCUS_%d____\n",
		__FUNCTION__, event->window, event->subwindow, winfocus, revert_focus);

	if (winfocus == root || winfocus == rootw) return;

	program *prog = get_prog(event->window);
	XSetWindowBorder(display, event->window, _RGB(0, 255, 00));

	if (prog->monitor == curr_monitor) {
		monitor *mon = get_mon(prog->monitor);
		mon->last_focused = (int) winfocus;
	}
}

void handle_leave_notify(XEvent *e) {
	XCrossingEvent *event = &e->xcrossing;

	Window winfocus;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);

	printf("________FUNC_%s____________________________________________________LEAVE_WINDOW_%d____SUBWINDOW_%d___________________WINDOW_FOCUS_%d__________REVERT_FOCUS_%d____\n",
		__FUNCTION__, event->window, event->subwindow, winfocus, revert_focus);

	if (event->window == root || event->window == rootw) return;

	program *prog = get_prog(event->window);
	XSetWindowBorder(display, event->window, _RGB(0, 255, 00));

	if (prog->monitor == curr_monitor) {
		monitor *mon = get_mon(prog->monitor);
		// mon->last_focused = (int) winfocus; NE NADO BLYAT
	}

}


void handle_property_notify(XPropertyEvent *e) {

//	founds(__FUNCTION__);
	printf("________FUNC_%s__________\n", __FUNCTION__);
}

int kz(int x) {
	printf("________FUNC_%s__________\n", __FUNCTION__);

	if (signal(SIGCHLD, kz) == SIG_ERR)
		exit(-1);
	while (0 < waitpid(-1, NULL, WNOHANG))
		;
}

void handle_client_message(XClientMessageEvent *e) {
	printf("________FUNC_%s____________________WINDOW_%d_____________TYPE_%d_____________MESSAGE_TYPE_%d______\n", __FUNCTION__, e->window, e->type, e->message_type);

	if ((Atom) e->data.l[0] == AtomWMDelete) {
		printf("________FUNC_%s_____DELETE_ATOM_____\n", __FUNCTION__);

		//mkill(&e.xclient);
	}


	if (e->message_type == netwmstate) {
		if (e->data.l[1] == netwmfs || e->data.l[2] == netwmfs) {
			printf("________FUNC_%s_____DELETE_ATOM_____\n", __FUNCTION__);

//			fullscreen(e->window);
		}
	}
}


void init_atoms() {
	printf("________FUNC_%s__________\n", __FUNCTION__);

	AtomUTF8 = XInternAtom(display, "UTF8_STRING", False);

	AtomWMProtocols = XInternAtom(display, "WM_PROTOCOLS", 0);
	AtomWMDelete = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	AtomWMState = XInternAtom(display, "WM_STATE", 0);
	AtomWMTakeFocus = XInternAtom(display, "WM_TAKE_FOCUS", 0);

	AtomNetSupported = XInternAtom(display, "_NET_SUPPORTED", 0);
	AtomNetWMName = XInternAtom(display, "_NET_WM_NAME", 0);
	AtomNetWMState = XInternAtom(display, "_NET_WM_STATE", 0);
	AtomNetWMCheck = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", 0);
	AtomNetWMFullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", 0);
	AtomNetActiveWindow = XInternAtom(display, "_NET_ACTIVE_WINDOW", 0);
	AtomNetWMWindowType = XInternAtom(display, "_NET_WM_WINDOW_TYPE", 0);
	AtomNetWMWindowTypeDialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", 0);
	AtomNetClientList = XInternAtom(display, "_NET_CLIENT_LIST", 0);
}





typedef struct Shortcut {
	int mask;
	KeySym ks;
} Shortcut;






int main(void) {

	printf("\n==========================================================\n");
	printf("            BEGIN            %s\n", __FUNCTION__);
	printf("==========================================================\n");


	kz(0);

	xerrorxlib = XSetErrorHandler(xerrorstart);

	display = XOpenDisplay(NULL);
	screen_def = DefaultScreen(display);

	sw = DisplayWidth(display, screen_def);
	sh = DisplayHeight(display, screen_def);
	root = RootWindow(display, screen_def);
	rootw = XDefaultRootWindow(display);

	black = BlackPixel(display, screen_def);
	white = WhitePixel(display, screen_def);



	init_atoms();


	last_mon = (struct monitor*) malloc( sizeof( struct monitor) );
	last_mon->id = 0;
	last_mon->last_focused = (int) root;
	last_mon->next = last_mon;
	last_mon->prev = last_mon;

	first_mon = last_mon;

	printf("________FUNC_%s________________MONITOR_ADDED____ID_%d____WINDOW_%d______\n", __FUNCTION__, last_mon->id, last_mon->last_focused);


	for(int i = 0; i < num_monitors; i++) {
		add_mon(i + 1);
	}


//	init_progs();

	last = (struct program*) malloc( sizeof( struct program ) );
	last->id = 0;
	last->window = (int) root;
	last->next = last;
	last->prev = last;

	first = last;

//	last->next = last;
//	last = last->next;

	prog_count++;








	printf("________FUNC_%s________________ROOT_ADDED____ID_%d____WINDOW_%d______\n", __FUNCTION__, last->id, last->window);
	founds(__FUNCTION__);


	XSetErrorHandler(xerrordummy);
	XSetIOErrorHandler(xerrordummy);


//	XSync(display, False);
//	XUngrabServer(display);
//	XSync(display, False);


	Atom netsupport = XInternAtom(display, "_NET_SUPPORTED", 0);
	Atom clientlist = XInternAtom(display, "_NET_CLIENT_LIST", 0);


	Atom utfstr;

	netwmstate = XInternAtom(display, "_NET_WM_STATE", False);
	netwmfs = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
	netwmtype = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	netwmdialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);


	checkwin = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, 0, 0);


 	XChangeProperty(display, checkwin, AtomNetWMCheck, XA_WINDOW, 32, PropModeReplace,
			(unsigned char*) &checkwin, 1);
	XChangeProperty(display, checkwin, AtomNetWMName, AtomUTF8, 8, PropModeReplace,
			(unsigned char*) "asd", 3);

	XChangeProperty(display, root, AtomNetWMCheck, XA_WINDOW, 32, PropModeReplace,
			(unsigned char*) &checkwin, 1);
	XChangeProperty(display, root, AtomNetSupported, XA_WINDOW, 32, PropModeReplace,
			(unsigned char*) &checkwin, 1);
	XDeleteProperty(display, root, AtomNetClientList);



	XSync(display, False);
	XFlush(display);

	font = XLoadFont(display, "-*-*-*-R-Normal--*-180-100-100-*-*");
	cursor = XCreateFontCursor(display, XC_left_ptr);
	cursor_move = XCreateFontCursor(display, XC_fleur);
	cursor_size = XCreateFontCursor(display, XC_sizing);

//	border_color = _RGB(255, 66, 66);
	border_color = _RGB(255, 255, 255);

//	sigset_t sigmask, oldsigmask;
//	sigprocmask(2, &sigmask, &oldsigmask);

	XSetWindowAttributes attrs;
	attrs.event_mask = SubstructureRedirectMask |
						SubstructureRedirectMask |
						SubstructureNotifyMask |
						StructureNotifyMask |
						ExposureMask |
						KeyPressMask |
						KeyReleaseMask |
						PointerMotionMask |
						ButtonPressMask |
						ButtonReleaseMask |
						EnterWindowMask |
						LeaveWindowMask |
						FocusChangeMask |
						PropertyChangeMask;

	attrs.do_not_propagate_mask = 0;

	attrs.cursor = cursor;




	unsigned long attrs_mask = CWEventMask | NoEventMask | CWBackPixel;

//	XSelectInput(display, root, attrs.event_mask);
	XSync(display, False);

	XChangeWindowAttributes(display, root, CWEventMask | CWCursor, &attrs);

/*	XGrabKey(display, XKeysymToKeycode(display, XK_1), XKeysymToKeycode(display, XK_Alt_L), root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_2), XKeysymToKeycode(display, XK_Alt_L), root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_3), XKeysymToKeycode(display, XK_Alt_L), root, False, GrabModeAsync, GrabModeAsync);
*/

// mainsgrab
	XUngrabKey(display, AnyKey, AnyModifier, root);

	XGrabKey(display, XKeysymToKeycode(display, XK_0), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_5), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_7), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Return), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_1), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_2), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_3), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_1), Mod1Mask | ControlMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_2), Mod1Mask | ControlMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_3), Mod1Mask | ControlMask, root, True, GrabModeAsync, GrabModeAsync);

//	XGrabKey(display, AnyKey, Mod1Mask, root, False, GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, AnyKey, Mod1Mask | ControlMask, root, False, GrabModeAsync, GrabModeAsync);




	XSync(display, False);
	XFlush(display);



	XSync(display, False);


	XEvent e;
	XSync(display, False);
	while (1) {

		XNextEvent(display, &e);

		switch (e.type) {
		case ClientMessage:
			handle_client_message(&e.xclient);
			break;
		case Expose:
			handle_expose(&e.xexpose);
			break;
		case KeyPress:
			handle_key_press(&e.xkey);
			break;
		case KeyRelease:
			handle_key_release(&e.xkey);
//			handle_key_press_release(&c);
			break;
		case ConfigureRequest:
			handle_configure_request(&e.xconfigurerequest);
			break;
		case ConfigureNotify:
			handle_configure_notify(&e.xconfigure);
			break;
		case MapRequest:
			handle_map_request(&e.xmaprequest);
			break;
		case ButtonPress:
			handle_button_press(&e.xbutton);
			break;
		case ButtonRelease:
			handle_button_release(&e.xbutton);
			break;
		case MotionNotify:
			handle_motion_notify(&e.xmotion);
			break;
		case UnmapNotify:
			handle_unmap_notify(&e.xunmap);
			break;
		case DestroyNotify:
			handle_destroy_notify(&e.xdestroywindow);
			break;
		case FocusIn:
			handle_focus_event(&e.xfocus);
			break;
		case FocusOut:
			handle_unfocus_event(&e.xfocus);
			break;
		case EnterNotify:
			handle_enter_notify(&e);
			break;
		case LeaveNotify:
			handle_leave_notify(&e);
			break;
///		case DestroyNotify:
///			handle_destroy_notify(&e.xdestroywindow);
///			break;
//		case PropertyNotify:
//			handle_property_notify(&e.xproperty);
//			break;
		}

	}
	return EXIT_SUCCESS;
}
