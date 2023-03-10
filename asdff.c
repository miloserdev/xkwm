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
	printf("xid: %s, request code: %d, error code: %d...idle...   \n", e->resourceid, e->request_code,
			e->error_code);
}



static Display *display;
static Window root;
int rootw;

static Window checkwin;


int min_width = 200, min_height = 100;
int border_width = 1;
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

int num_monitors = 3;
int curr_monitor = 1;






typedef struct monitor {
	int id;
	int count;
	Window lasts;
	Window prevs;

	struct monitor *next;
	struct monitor *prev;

} monitor;


monitor *lastmon, *firstmon;


monitor * add_mon(int id) {
	struct monitor *obj;
	obj = (struct monitor*) malloc( sizeof( struct monitor ) );

	obj->id = id;
	obj->count = 0;
	obj->lasts = root;
	obj->prevs = root;
	obj->next = firstmon;
	obj->prev = lastmon;

	lastmon->next = obj;
	lastmon = lastmon->next;

	printf("________FUNC_%s________________MONITOR_ADDED____ID_%d____LAST_WINDOW_%d______\n", __FUNCTION__, lastmon->id, lastmon->lasts);

	return obj;
}

monitor * get_mon(int id) {
	monitor *mon = firstmon;
	for (int i = 0; i < num_monitors; i++) {
		if ( (int) mon->id == id ) return &(*mon);
		mon = mon->next;
	}
	return lastmon;
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
	Window winfocus;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);

	printf("________FUNC_%s______________________________________________________________ROOT_%d_________ROOTW_%d________FOCUSED_%d__________PROG_COUNT_%d\n", fn, root, rootw, winfocus, prog_count);
}



program * add_prog(Window win) {
	struct program *obj;

/*	if ( (obj == get_prog(win)) != NULL  ) {
		obj->window = win;
		return obj;
	}
*/
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

	monitor *mon;
	program *obj = first;
	for(int i = 0; i <= prog_count; i++) {
		printf("________FUNC_%s________________________________________TRYING_TO_REMOVE_WINDOW_%d________ITERATION_WINDOW_%d____\n", __FUNCTION__, win, obj->window);
		printf("________FUNC_%s_________________________________INTS___TRYING_TO_REMOVE_WINDOW_%d________ITERATION_WINDOW_%d____\n", __FUNCTION__, (int) win, (int) obj->window);

		if ( (int) obj->window == (int) win) {
			printf("________FUNC_%s________________________________________TRYING_TO_REMOVE_WINDOW_%d________FOUND%d_________\n", __FUNCTION__, win, obj->window);

		//	if(obj->window == root) return;
			obj->next->prev = obj->prev;
			obj->prev->next = obj->next;
			last = obj->next;
			prog_count--;

			if (mon = get_mon(obj->monitor) ) {
				mon->lasts = root;
				mon->count = mon->count - 1;
			}

			printf("________FUNC_%s________________________________________REMOVED_WINDOW_%d______WINDOW_%d_________________________\n", __FUNCTION__, win, obj->window);
			break;
		}
		obj = obj->next;
	}
	//free(obj);
}


















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





void everything(Window w) {

	Atom netwmstate = XInternAtom(display, "_NET_WM_STATE", False);

	Atom netwmmodal		= XInternAtom(display, "_NET_WM_STATE_MODAL", True);
	Atom netwmsticky 	= XInternAtom(display, "_NET_WM_STATE_STICKY", True);
	Atom netwmmaxvert	= XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", True);
	Atom netwmmaxhor 	= XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", True);
	Atom netwmshaded 	= XInternAtom(display, "_NET_WM_STATE_SHADED", True);
	Atom netwmskiptask 	= XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", True);
	Atom netwmskippager 	= XInternAtom(display, "_NET_WM_STATE_SKIP_PAGER", True);
	Atom netwmhidden 	= XInternAtom(display, "_NET_WM_STATE_HIDDEN", True);
	Atom netwmfs 		= XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);
	Atom netwmabove 	= XInternAtom(display, "_NET_WM_STATE_ABOVE", True);
	Atom netwmbelow 	= XInternAtom(display, "_NET_WM_STATE_BELOW", True);
	Atom netwmdematt 	= XInternAtom(display, "_NET_WM_STATE_DEMANDS_ATTENTION", True);



	Atom netwmtype = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);

	Atom netwmdesktop 	= XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
	Atom netwmdock 		= XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
	Atom netwmtoolbar 	= XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
	Atom netwmmenu		= XInternAtom(display, "_NET_WM_WINDOW_TYPE_MENU", False);
	Atom netwmutility 	= XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
	Atom netwmsplash 	= XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
	Atom netwmdialog 	= XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	Atom netwmnormal 	= XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);


	Atom state = get_atom(w, netwmstate);
	Atom type = get_atom(w, netwmtype);

	if (state == netwmmodal)
		printf("_________FUNCTION_%s__________________WINDOW_%d________MODAL_____\n", __FUNCTION__, w);
	if (state == netwmsticky)
		printf("_________FUNCTION_%s__________________WINDOW_%d________STICKY_____\n", __FUNCTION__, w);
	if (state == netwmmaxvert)
		printf("_________FUNCTION_%s__________________WINDOW_%d________MAX_VERT_____\n", __FUNCTION__, w);
	if (state == netwmmaxhor)
		printf("_________FUNCTION_%s__________________WINDOW_%d________MAX_HORZ____\n", __FUNCTION__, w);
	if (state == netwmshaded)
		printf("_________FUNCTION_%s__________________WINDOW_%d________SHADED_____\n", __FUNCTION__, w);
	if (state == netwmskiptask)
		printf("_________FUNCTION_%s__________________WINDOW_%d________SKIP_TASK_____\n", __FUNCTION__, w);
	if (state == netwmskippager)
		printf("_________FUNCTION_%s__________________WINDOW_%d________SKIP_PAGER_____\n", __FUNCTION__, w);
	if (state == netwmhidden)
		printf("_________FUNCTION_%s__________________WINDOW_%d________HIDDEN_____\n", __FUNCTION__, w);
	if (state == netwmfs)
		printf("_________FUNCTION_%s__________________WINDOW_%d________FULLSCREEN_____\n", __FUNCTION__, w);
	if (state == netwmabove)
		printf("_________FUNCTION_%s__________________WINDOW_%d________ABOVE_____\n", __FUNCTION__, w);
	if (state == netwmbelow)
		printf("_________FUNCTION_%s__________________WINDOW_%d________BELOW_____\n", __FUNCTION__, w);
	if (state == netwmdematt)
		printf("_________FUNCTION_%s__________________WINDOW_%d________DEMANDS_____\n", __FUNCTION__, w);


	if (type == netwmdesktop)
		printf("_________FUNCTION_%s__________________WINDOW_%d________TYPE_DESKTOP_____\n", __FUNCTION__, w);
	if (type == netwmdock)
		printf("_________FUNCTION_%s__________________WINDOW_%d________TYPE_DOCK_____\n", __FUNCTION__, w);
	if(type == netwmtoolbar)
		printf("_________FUNCTION_%s__________________WINDOW_%d________TYPE_TOOLBAR_____\n", __FUNCTION__, w);
	if(type == netwmmenu)
		printf("_________FUNCTION_%s__________________WINDOW_%d________TYPE_MENU_____\n", __FUNCTION__, w);
	if(type == netwmutility)
		printf("_________FUNCTION_%s__________________WINDOW_%d________TYPE_UTILITY_____\n", __FUNCTION__, w);
	if(type == netwmsplash)
		printf("_________FUNCTION_%s__________________WINDOW_%d________TYPE_SPLASH_____\n", __FUNCTION__, w);
	if(type == netwmdialog)
		printf("_________FUNCTION_%s__________________WINDOW_%d________TYPE_DIALOG_____\n", __FUNCTION__, w);

	XWindowChanges borders;

	if(type == netwmnormal) {
		borders.border_width = border_width;
		printf("_________FUNCTION_%s__________________WINDOW_%d________TYPE_NORMAL_____\n", __FUNCTION__, w);
	} else { borders.border_width = 0; }

	XConfigureWindow(display, w, CWBorderWidth, &borders);


}










































void do_not_go_fucking_out(Window win) {
	program *prog = get_prog(win);
	if(prog->monitor == curr_monitor) {
		XWindowAttributes xwa;
		XGetWindowAttributes(display, win, &xwa);

		if (xwa.width < min_width || xwa.height < min_height) {
			XResizeWindow(display, win, min_width, min_height);
		}

		if(xwa.x >= sw || xwa.x < 0   ||   xwa.y >= sh || xwa.y < 0) {
			int x = (sw / 2) - (xwa.width / 2);
			int y = (sh / 2) - (xwa.height / 2);
			XMoveWindow(display, win, x, y);
		}
	}
}












program * add_prog_with_check(Window win) {

	printf("________________________PROG_COUNT___%d\n", prog_count);

	bool exist = false;

	program *obj = first;
	for(int i = 0; i < prog_count; i++) {
		printf("DEBUG X11 		__________________CHECKING PROG %d             CALLER_Window %d\n", obj->window, win);
		if ( (int) obj->window ==  (int) win) {
			exist = true;
			break;
		} else {
		}
		obj = obj->next;
	}

	printf("__________________________________________________________________________BOOL_EXIST___%b\n", exist);

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

		founds(__FUNCTION__);


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
		printf("DEBUG X11 				new Program______Window %d\n", win);

		monitor *mon = get_mon(prog->monitor);

		mon->count = mon->count + 1;

		mon->prevs = (mon->count > 0) ? mon->lasts : root;
		mon->lasts = prog->window;

		XSync(display, False);

		founds(__FUNCTION__);

		prog_count++;
		return prog;
	}
	return obj;
	XSync(display, False);
}











Atom netwmstate, netwmfs, netwmtype, netwmdialog;
Atom AtomUTF8, AtomNetSupported, AtomNetWMName, AtomNetWMState, AtomNetWMCheck, AtomNetWMFullscreen, AtomNetActiveWindow, AtomNetWMWindowType, AtomNetWMWindowTypeDialog, AtomNetClientList;
Atom AtomWMProtocols, AtomWMDelete, AtomWMState, AtomWMTakeFocus;


















// #change_monitor #changemon

void change_monitor(int numb) {

	Window winfocus;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);

	curr_monitor = numb;
	monitor *mon = get_mon(numb);
	lastmon = mon;
	program *prog = first;



//	XRaiseWindow(display, root);
//	XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
	XSync(display, False);


//	XSetInputFocus(display, None, RevertToPointerRoot, CurrentTime);
//	if (winfocus !=root)   //XSetInputFocus(display, winfocus, RevertToPointerRoot, CurrentTime);



	for (int i = 0; i < prog_count; i++) {
//		if (prog->next != NULL) prog = prog->next;
		if ( (int) prog->window != root) {
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
//					XSetInputFocus(display, prog->window, 0, CurrentTime);

//					if (prog->window != root) XSetInputFocus(display, prog->window, RevertToPointerRoot, CurrentTime);

				}
			}
		}
		prog = prog->next;
	}

	XSync(display, False);

/*	monitor *mond;
	mond = get_mon(numb);
*/
	XRaiseWindow(display, mon->lasts);
	XSetInputFocus(display, mon->lasts, RevertToPointerRoot, CurrentTime);



/*	if (mon->count > 0) {
		XRaiseWindow(display, mon->lasts);
		XSetInputFocus(display, mon->lasts, RevertToPointerRoot, CurrentTime);
	} else {
		XRaiseWindow(display, root);
		XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
	}
*/

//	XRaiseWindow(display, root);
//	XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);

	printf("________FUNC_%s__________________MONITOR_%d_______LAST_WINDOW_%d_____\n", __FUNCTION__, mon->id, mon->lasts);
	printf("________FUNC_%s_____PROG_COUNT_%d___WINFOCUS_%d___REV_FOCUS_%d___CURR_MONITOR_%d_____\n", __FUNCTION__, prog_count, winfocus, revert_focus, curr_monitor);


	XSync(display, False);
}








// #fullscreen

void fullscreen(Window w) {

//	if (w == root) return;

	printf("________FUNC_%s__________PROG%d\n", __FUNCTION__, w);

	program *prog = get_prog(w);

	printf("________FUNC_%s__________PROG%d\n", __FUNCTION__, w);


	if(prog == first) return;

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

//	everything(e->window);

	if (e->window == root) return;


	Window winfocus;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);



	founds(__FUNCTION__);

	program *progg = add_prog_with_check(e->window);

	//	do_not_go_fucking_out(e->window);

	//	if (progg->xwa.width > sw || progg->xwa.height > sh) XResizeWindow(display, e->window, sw - 200, sh - 200);

	program *prog;
	monitor *mon;
	if ( prog = get_prog(e->window) ) {
		if ( mon = get_mon(prog->monitor) ) {
/*lastchanged			mon->lasts = e->window; */
			printf("________FUNC_%s__________SWITCH________WINDOW_%d___________MONITOR_%d______LASTS_%d_______WINFOCUS_%d_____\n", __FUNCTION__, e->window, mon->id, mon->lasts, winfocus);
		}
	} else {
//		XRaiseWindow(display, e->window);
//maybe		XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);
	}

	XSync(display, False);

}


void process_win(int _mask, Window winfocus, Window window, int revert_focus, int monnumb) {
	if (_mask == (Mod1Mask | ControlMask) ) {

		printf("__________CONTROLMASKPRESSED______WINDOW_%d______WINFOCUS_%d______REVERT_FOCUS_%d____\n", window, winfocus, revert_focus);

		program *prog = get_prog(winfocus);

		monitor *mt = get_mon(prog->monitor);

		prog->monitor = monnumb;

		mt->lasts = mt->prevs;
		mt->count -= 1;
		monitor *mn = get_mon(monnumb);
		mn->count += 1;

		XSetInputFocus(display, mt->lasts, RevertToPointerRoot, CurrentTime);

		change_monitor(curr_monitor);
		return;
//		break;
	} else {
		printf("__________CHANGING_MON___CURRENT_%d___NEW_%d______WINDOW_%d______WINFOCUS_%d______REVERT_FOCUS_%d____\n", curr_monitor, 1, window, winfocus, revert_focus);

		lastmon->lasts = lastmon->prevs;
		change_monitor(monnumb);
		XSync(display, False);
	}

}



void handle_key_press(XKeyEvent *e) {

	printf("\nDEBUG X11 func: %s window: %d\n", __FUNCTION__, e->window);

	XSync(display, False);


//	if (e->window == root) return;

//	program *prgroot = get_prog(root);
//	prgroot->monitor = curr_monitor;

//	everything(e->window);

	Window winfocus;
//	winfocus = e->window;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);


//lastchange!	handle_expose(e);


	program *prog;
	monitor *mon;
	if ( prog = get_prog(winfocus) ) {
		if ( mon = get_mon(prog->monitor) ) {
//			mon->lasts = winfocus; //maybe
			printf("________FUNC_%s__________SWITCH________WINDOW_%d________FOCUSED_%d___________MONITOR_%d______LASTS_%d______\n", __FUNCTION__, e->window, winfocus, mon->id, mon->lasts);
		}
	}

//lastchangealso	if (e->window == root) XSetInputFocus(display, 1, RevertToPointerRoot, CurrentTime);

//	if (winfocus != root || winfocus != 0) XSetInputFocus(display, winfocus, 0, CurrentTime);


	int _mask = (e->state & (ShiftMask | ControlMask | Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask) );


	switch (XKeycodeToKeysym(display, e->keycode, 0)) {


	case XK_7:

		founds(__FUNCTION__);

//		XSetWindowAttributes attrs;
//		attrs.event_mask = ExposureMask | FocusChangeMask | KeyPressMask
//				| StructureNotifyMask | VisibilityChangeMask | ExposureMask;
//		XChangeWindowAttributes(display, e->window, 0, &attrs);
//		XMapWindow(display, e->window);
		break;
	case XK_Alt_L:
//		printf("DEBUG X11 func: %s ALT\n", __FUNCTION__);

		alt_pressed = true;
		break;
	case XK_Shift_L:
//		printf("DEBUG X11 func: %s SHIFT\n", __FUNCTION__);

		shift_pressed = true;
		break;
	case XK_Return:
//		printf("DEBUG X11 func: %s RETURN\n", __FUNCTION__);

		if (alt_pressed)
			fullscreen(e->window);
		break;
	case XK_5:
		if (e->state & Mod1Mask) {

			monitor *mtr = firstmon;

			int gap = 10;
			program *prog = first;

			int _sw = sw, _sh = sh;
			int _bw = border_width + gap;
			int _x = 0 - _bw + gap, _y = 0 - _bw + gap;

			int ci = 1;

			for (int c = 0; c <= prog_count; c++   /*	:)	*/) {

				if (prog->window != root, prog->monitor == curr_monitor) {
					mtr = get_mon(prog->monitor);
				//	XWindowAttributes xwa;
				//	XGetWindowAttributes(display, prog->window, &xwa);
					if (ci % 2 != 0) { _sw = _sw / 2; if (ci >= mtr->count) _sw = _sw * 2;
					} else { 	  _sh = _sh / 2; if (ci >= mtr->count) _sh = _sh * 2; }

					XMoveResizeWindow(display, prog->window, _x + _bw, _y + _bw, _sw - _bw * 2, _sh - _bw * 2);

					if (ci % 2 != 0) { _x = _x + _sw;
					} else { 	  _y = _y + _sh; }
					ci++;
				}
			prog = prog->next;


/*

			//	if (prog->next == NULL) return;
//				printf("prog = prog->next;");
				if(prog->window != root && prog->monitor == curr_monitor) {
					mon = get_mon(prog->monitor);

					printf("_______BEGIN______EFEFEF_______PROG_%d___MONITOR_%d___CI_%d___COUNT_%d________X_%d___Y_%d___XW_%d___XH_%d______\n", prog->window, mon->id, ci, mon->count, _x, _y, _sw, _sh);

					if (ci == 0) _sw = _sw / 2;

					if (ci % 2 == 0) {
					//	y = sh * 2;
						_sw = _sw / 2;
						if (ci >= mon->count - 1) printf("___EFEFEF___CI_%d___COUNT_%d___\n", ci, mon->count); _sw = _sw * 2;
					} else {
					//	x = sw * 2;
						_sh = _sh / 2;
						if (ci >= mon->count - 1) printf("___EFEFEF___CI_%d___COUNT_%d___\n", ci, mon->count); _sh = _sh * 2;
					}
					XMoveResizeWindow(display, prog->window, _x + _bw, _y + _bw, _sw - _bw*2, _sh - _bw*2);
//					XMoveResizeWindow(display, prog->window, _x + _bw, _y + _bw, _sw - _bw*2, _sh - _bw*2);
					if (ci % 2 == 0) {
						_x = _x + _sw;
					} else {
						_y = _y + _sh;
					}

					printf("__________________EFEFEF_______PROG_%d___MONITOR_%d___CI_%d___COUNT_%d________X_%d___Y_%d___XW_%d___XH_%d______\n", prog->window, mon->id, ci, mon->count, _x, _y, _sw, _sh);
					ci++;

				}
//				if (prog->next != NULL) prog = prog->next;
				prog = prog->next;


*/

			}
		}
		break;
	case XK_4:
		process_win(_mask, winfocus, e->window, revert_focus, 4);
		break;
	case XK_1:
		process_win(_mask, winfocus, e->window, revert_focus, 1);

/*		if (e->state & Mod1Mask) {
			if (_mask == (Mod1Mask | ControlMask) ) {
			//	XGetInputFocus(display, &winfocus, &revert_focus);

			//	if (winfocus == root) printf("WINFOCUS IS ROOT________RETURN..."); return;
				printf("__________CONTROLMASKPRESSED______WINDOW_%d______WINFOCUS_%d______REVERT_FOCUS_%d____\n", e->window, winfocus, revert_focus);

				program *prog = get_prog(winfocus);

				monitor *mt = get_mon(prog->monitor);

				prog->monitor = 1;

				mt->lasts = mt->prevs;
				mt->count -= 1;
				monitor *mn = get_mon(1);
				mn->count += 1;

				XSetInputFocus(display, mt->lasts, RevertToPointerRoot, CurrentTime);

				change_monitor(curr_monitor);
				break;
			} else {
				printf("__________CHANGING_MON___CURRENT_%d___NEW_%d______WINDOW_%d______WINFOCUS_%d______REVERT_FOCUS_%d____\n", curr_monitor, 1, e->window, winfocus, revert_focus);

				lastmon->lasts = lastmon->prevs;
				change_monitor(1);
				XSync(display, False);

			}
		}	*/
		break;
	case XK_2:
		if (e->state & Mod1Mask) {
			if (_mask == (Mod1Mask | ControlMask) ) {
			//	XGetInputFocus(display, &winfocus, &revert_focus);

			//	if (winfocus == root) printf("WINFOCUS IS ROOT________RETURN..."); return;
				printf("__________CONTROLMASKPRESSED______WINDOW_%d______WINFOCUS_%d______REVERT_FOCUS_%d____\n", e->window, winfocus, revert_focus);

				program *prog = get_prog(winfocus);

				monitor *mt = get_mon(prog->monitor);

				prog->monitor = 2;

				mt->lasts = mt->prevs;
				mt->count -= 1;
				monitor *mn = get_mon(2);
				mn->count += 1;

				XSetInputFocus(display, mt->lasts, RevertToPointerRoot, CurrentTime);

				change_monitor(curr_monitor);
				break;
			} else {
				printf("__________CHANGING_MON___CURRENT_%d___NEW_%d______WINDOW_%d______WINFOCUS_%d______REVERT_FOCUS_%d____\n", curr_monitor, 2, e->window, winfocus, revert_focus);

				lastmon->lasts = lastmon->prevs;
				change_monitor(2);
				XSync(display, False);

			}
		}
		break;
	case XK_3:
		if (e->state & Mod1Mask) {
			if (_mask == (Mod1Mask | ControlMask) ) {
			//	XGetInputFocus(display, &winfocus, &revert_focus);

			//	if (winfocus == root) printf("WINFOCUS IS ROOT________RETURN..."); return;
				printf("__________CONTROLMASKPRESSED______WINDOW_%d______WINFOCUS_%d______REVERT_FOCUS_%d____\n", e->window, winfocus, revert_focus);

				program *prog = get_prog(winfocus);

				monitor *mt = get_mon(prog->monitor);

				prog->monitor = 3;

				mt->lasts = mt->prevs;
				mt->count -= 1;
				monitor *mn = get_mon(3);
				mn->count += 1;

				XSetInputFocus(display, mt->lasts, RevertToPointerRoot, CurrentTime);

				change_monitor(curr_monitor);
				break;
			} else {
				printf("__________CHANGING_MON___CURRENT_%d___NEW_%d______WINDOW_%d______WINFOCUS_%d______REVERT_FOCUS_%d____\n", curr_monitor, 3, e->window, winfocus, revert_focus);

				lastmon->lasts = lastmon->prevs;
				change_monitor(3);
				XSync(display, False);

			}
		}
		break;
	case XK_0:
		if (alt_pressed) {
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
	XSync(display, False);

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
	}
}

void handle_configure_request(XConfigureRequestEvent *e) {

	printf("DEBUG X11 func: %s window: %d\n", __FUNCTION__, e->window);




//	everything(e->window);




/////////////////////Every time updates////////////////////////

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
	//		XMoveResizeWindow(display, e->window, size.min_aspect.x, size.min_aspect.y, size.min_width, size.min_height);
	//		XMoveResizeWindow(display, e->window, size.min_aspect.x, size.min_aspect.y, size.base_width, size.base_height);

	//	XSetInputFocus(display, last->window, 0, CurrentTime);
	//	XSetInputFocus(display, last->window, RevertToPointerRoot, CurrentTime);


//	if (e->window != root) XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);




	program *prog;
	XWindowChanges wc;
	//if ( (prog = get_prog(e->window)) ) {
		//if (e->value_mask & CWBorderWidth) {
	XWindowChanges wcs;
	wcs.x = e->x;
	wcs.y = e->y;
	wcs.width = e->width;
	wcs.height = e->height;
	wcs.border_width = border_width;
	wcs.sibling = e->above;
	wcs.stack_mode = e->detail;
	XConfigureWindow(display, e->window, e->value_mask, &wcs);
	XSetWindowBorder(display, e->window, border_color);


/*

	XConfigureEvent ce;
	ce.type = ConfigureNotify;
	ce.display = display;
	ce.event = e->window;
	ce.window = e->window;
	ce.x = e->x;
	ce.y = e->y;
	ce.width = e->width;
	ce.height = e->height;
	ce.border_width = border_width;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(display, e->window, False, StructureNotifyMask, (XEvent *)&ce);
*/

		//}
	//}


	//	if(e->window == root || e->window == rootw) return;
	//	program *prog;
	///	if (get_prog(e->window)) prog = get_prog(e->window);
	//	else prog = add_prog(e->window);


	XSync(display, False);

}

void handle_configure_notify(XConfigureEvent *e) {

	printf("DEBUG X11 func: %s window: %d\n", __FUNCTION__, e->window);




//	everything(e->window);




	XWindowAttributes xwa;
	XGetWindowAttributes(display, e->window, &xwa);

//	XMoveResizeWindow(display, e->window, xwa.x, xwa.y, xwa.width, xwa.height);

//	XSetInputFocus(display, last->window, 0, CurrentTime);


/*lastchange	if (e->window != root) XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);
*/

//	XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);

/*	Atom netwmstate = XInternAtom(display, "_NET_WM_STATE", False);
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

*/

//	XReparentWindow(display, e->window, e->window, 0, 0);
//	XSetWindowBorderWidth(display, e->window, ( ( (xwa.x == 0 && xwa.y == 0) && (xwa.width >= 1920 && xwa.height >= 1080 ) ) ? 0 : border_width ) );
	XSync(display, False);
}

void mapreq(XMapRequestEvent *e) {

	printf("AAAAAAAAAAAAAAAAAAAAAAAAAATTENTION BLYAT");
//	everything(e->window);

//	if (e->window == root) return;

	XWindowAttributes xc;
	if (!XGetWindowAttributes(display, e->window, &xc) || xc.override_redirect) return;

	program *prog;
	if ( (prog = get_prog(e->window) ) ) {
		printf("____________________EXISTS____________!!!___________\n");
		return;
	}

	add_prog_with_check(e->window);

	XWindowAttributes xwa;
//	XGetWindowAttributes(display, e->window, &xwa);

	XSetWindowAttributes attrs;
	attrs.event_mask = SubstructureRedirectMask |
			   SubstructureNotifyMask |
			   StructureNotifyMask |
			   ExposureMask |
			   KeyPressMask |
			   KeyReleaseMask |
			   PointerMotionMask |
		//	   VisibilityChangeMask |
			   ButtonPressMask |
			   ButtonReleaseMask; // |
		//	   EnterWindowMask |
		//	   LeaveWindowMask |
		//	   FocusChangeMask |
		//	   PropertyChangeMask;


//	XSetWindowBorderWidth(display, e->window, border_width);


//	XMoveResizeWindow(display, e->window, 100, 100, 1000, 800);


//	XChangeProperty(display, root, AtomNetClientList, XA_WINDOW, 32, PropModeAppend, (unsigned char*) &(e->window), 1);
/*
	XChangeWindowAttributes(display, e->window, CWEventMask | CWCursor, &attrs);

	Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(display, e->window, &delWindow, 1);
*/

	XMapWindow(display, e->window);

	XRaiseWindow(display, e->window);
	XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);

}

void handle_map_request(XMapRequestEvent *e) {
	printf("DEBUG X11 func: %s window: %d\n", __FUNCTION__, e->window);

//	everything(e->window);

	founds(__FUNCTION__);

	Pixmap color = 0x00fefe;

/*	XGrabButton(display, Button1, Mod1Mask, e->window, False,
	ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
	GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, Mod1Mask, e->window, False,
	ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
	GrabModeAsync, GrabModeAsync, None, None);
			*/

	XWindowAttributes xwa;
	XGetWindowAttributes(display, e->window, &xwa);


	XSetWindowAttributes attrs;
	attrs.event_mask = SubstructureRedirectMask |
			   SubstructureRedirectMask |
			   SubstructureNotifyMask |
			   StructureNotifyMask |
			   ExposureMask |
			   KeyPressMask |
			   KeyReleaseMask |
			   PointerMotionMask |
		//	   VisibilityChangeMask |
			   ButtonPressMask |
			   ButtonReleaseMask; // |
		//	   EnterWindowMask |
		//	   LeaveWindowMask |
		//	   FocusChangeMask |
		//	   PropertyChangeMask;


	int x, y, width, height;

	long msize;

	XSizeHints size;
	if (!XGetWMNormalHints(display, e->window, &size, &msize)) {
		size.flags = PSize;
	}





	XSetWindowBorderWidth(display, e->window, border_width);
	XSetWindowBorder(display, e->window, border_color);

	XChangeProperty(display, root, AtomNetClientList, XA_WINDOW, 32, PropModeAppend, (unsigned char*) &(e->window), 1);

	XChangeWindowAttributes(display, e->window, CWEventMask | CWCursor, &attrs);

	Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(display, e->window, &delWindow, 1);


	x = xwa.x < 1 ? (sw / 2) - (xwa.width / 2) : xwa.x;
	y = xwa.y < 1 ? (sh / 2) - (xwa.height / 2) : xwa.y;
	XMoveWindow(display, e->window, x, y);

	width = MAX(size.base_width, xwa.width);
	height = MAX(size.base_height, xwa.height);

	width = MAX(size.min_width, width);
	height = MAX(size.min_height, height);

//		XMoveResizeWindow(display, e->window, size.min_aspect.x, size.min_aspect.y, size.min_width, size.min_height);
//	XMoveResizeWindow(display, e->window, size.min_aspect.x, size.min_aspect.y, width, height);
	XResizeWindow(display, e->window, width, height);
//	if (xwa.x < 250 || xwa.y < 250) XResizeWindow(display, e->window, width, height);

//	XSetTransientForHint(display, root, e->window);


//	XSetInputFocus(display, last->window, 0, CurrentTime);
			//	XSetInputFocus(display, last->window, RevertToPointerRoot, CurrentTime);

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


//	XGrabServer(display);

	add_prog_with_check(e->window);



			//	XRaiseWindow(display, e->window);
	XMapWindow(display, e->window);

			//	XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);

//	if (xwa.width > sw || xwa.height > sh) XResizeWindow(display, e->window, sw - 200, sh - 200);


//	XSync(display, False);
//	XUngrabServer(display);
	XSync(display, False);
}

void handle_button_press(XButtonEvent *e) {
	printf("________FUNC_%s_______WINDOW_%d\n", __FUNCTION__, e->window);

//	everything(e->window);

//	if (e->window == root) return;
//	XSetInputFocus(display, e->window, 0, CurrentTime);

//	if (e->window != root) XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);

	if (alt_pressed) {

		program *prog = get_prog(e->window);

//		t_prog *prog = get_prog(e->window);

//		printf("_______FUNC_%s_______PROG_%d\n", __FUNCTION__, prog->window);

		int cx, cy, rx, ry = 0;
		unsigned int maskq = 0;

		XWindowAttributes xwa;
		XGetWindowAttributes(display, e->window, &xwa);

		Window c, r;
		XQueryPointer(display, e->window, &c, &r, &rx, &ry, &cx, &cy, &maskq);

		XRaiseWindow(display, e->window);
//		XSetInputFocus(display, e->window, 0, CurrentTime);
//		XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);
		if (e->window != root) XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);

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
				printf("_________MOVEWINDOW_NOTIFY_________%d\n", e->window);
//				XSetTransientForHint(display, root, e->window);

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
	Window winfocus;
	int revert_focus;
	XGetInputFocus(display, &winfocus, &revert_focus);

//	printf("________FUNC_%s__________WINDOW_%d__________FOCUSED_%d______\n", __FUNCTION__, e->window, winfocus);

//	XSetInputFocus(display, e->window, 0, CurrentTime);
	if (e->window != root) XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);

	XSync(display, False);
}







void handle_focus_event(XEvent *e) {
	XFocusChangeEvent *event = &e->xfocus;
	printf("________FUNC_%s__________WINDOW_%d________________\n", __FUNCTION__, event->window);
}

void handle_unfocus_event(XEvent *e) {
	XFocusChangeEvent *event = &e->xfocus;
	printf("________FUNC_%s__________WINDOW_%d________________\n", __FUNCTION__, event->window);
}

void handle_enter_notify(XEvent *e) {
	XCrossingEvent *event = &e->xcrossing;
	printf("________FUNC_%s__________WINDOW_%d__________SUBWINDOW_%d______\n", __FUNCTION__, event->window, event->subwindow);
}

void handle_leave_notify(XEvent *e) {
	XCrossingEvent *event = &e->xcrossing;
	printf("________FUNC_%s__________WINDOW_%d__________SUBWINDOW_%d______\n", __FUNCTION__, event->window, event->subwindow);
}






void handle_unmap_notify(XUnmapEvent *e) {

	printf("DEBUG X11 func: %s | window: %d \n", __FUNCTION__, e->window);


//	XGrabServer(display);

	//	XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);

//	if (e->window != root) XSetInputFocus(display, e->window, RevertToPointerRoot, CurrentTime);

	//	XSetInputFocus(display, e->window, 0, CurrentTime);
	//	XSync(display, False);

	//	add_prog(e->window);

	//	Program *prog = get_program(e->window);
	//	program *prog = get_prog(e->window);
	//	printf("________FUNC_%s__________PROG%d\n", __FUNCTION__, prog->window);

	//	if (prog != NULL) remove_program(prog);

	/*	if (prog->window < 1) {
			return;
		}

		if (e->event == root || e->window == root) {
		return;
	}*/

//	rem_prog(e->window);

	founds(__FUNCTION__);

//	XUnmapWindow(display, e->window);

//	XSync(display, False);
//	XUngrabServer(display);
//	XSync(display, False);

}

void handle_destroy_notify(XDestroyWindowEvent *e) {

	printf("________FUNC_%s__________\n", __FUNCTION__);

//	XGrabServer(display);

	rem_prog(e->window);

	mkill(e->window);

//	XSync(display, False);
//	XUngrabServer(display);
	XSync(display, False);
}

void handle_property_notify(XPropertyEvent *e) {

//	founds(__FUNCTION__);

	printf("DEBUG X11 func: %s | window: %d \n", __FUNCTION__, e->window);
}

int kz(int x) {
	printf("________FUNC_%s__________\n", __FUNCTION__);

	if (signal(SIGCHLD, kz) == SIG_ERR)
		exit(-1);
	while (0 < waitpid(-1, NULL, WNOHANG))
		;
}

void handle_client_message(XClientMessageEvent *e) {
	printf("DEBUG X11 func: %s | window: %d ___________________________________TYPE_%d_____________ATOM_%d \n", __FUNCTION__, e->window, e->type, e->message_type);

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

	lastmon = (struct monitor*) malloc( sizeof ( struct monitor ) );
	lastmon->id = 0;
	lastmon->count = 0;
	lastmon->lasts = (int) root;
	lastmon->prevs = (int) root;
	lastmon->next = lastmon;
	lastmon->prev = lastmon;
	firstmon = lastmon;


	for (int i = 0; i < num_monitors; i++) {
		add_mon(i + 1);
	}

//	init_progs();

	last = (struct program*) malloc( sizeof ( struct program ) );
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
		//	   SubstructureRedirectMask |
			   SubstructureNotifyMask |
			   StructureNotifyMask |
			   ExposureMask |
			   KeyPressMask |
			   KeyReleaseMask |
		//	   PointerMotionMask |
		//	   ButtonPressMask |
		//	   ButtonReleaseMask |
		//	   EnterWindowMask |
		//	   LeaveWindowMask |
		//	   FocusChangeMask |
			   PropertyChangeMask;

	attrs.do_not_propagate_mask = 0;

	attrs.cursor = cursor;




//	unsigned long attrs_mask = CWEventMask | NoEventMask | CWBackPixel;
	unsigned long attrs_mask = CWEventMask | CWCursor | NoEventMask | CWBackPixel;

	XSelectInput(display, root, attrs.event_mask);
	XSync(display, False);

	XChangeWindowAttributes(display, root, attrs_mask, &attrs);

//	XGrabKey(display, XKeysymToKeycode(display, XK_1),
//			XKeysymToKeycode(display, XK_Alt_L), root, False,
//			GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_2),
//			XKeysymToKeycode(display, XK_Alt_L), root, False,
//			GrabModeAsync, GrabModeAsync);
//	XGrabKey(display, XKeysymToKeycode(display, XK_3),
//			XKeysymToKeycode(display, XK_Alt_L), root, False,
//			GrabModeAsync, GrabModeAsync);


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



	XSync(display, False);
	XFlush(display);



	XSync(display, False);


	XEvent e;
	XSync(display, False);
	while (1) {

		XNextEvent(display, &e);

//		XGrabServer(display);

		switch (e.type) {
		case ClientMessage:
		//	handle_client_message(&e.xclient);
			break;
		case Expose:
			handle_expose(&e.xexpose);
			break;
		case KeyPress:
			handle_key_press(&e.xkey);
			break;
		case KeyRelease:
			handle_key_release(&e.xkey);
			break;
		case ConfigureRequest:
			handle_configure_request(&e.xconfigurerequest);
			break;
		case ConfigureNotify:
			handle_configure_notify(&e.xconfigure);
			break;
		case MapRequest:
		//	handle_map_request(&e.xmaprequest);
			mapreq(&e.xmaprequest);
			break;
		case ButtonPress:
			handle_button_press(&e.xbutton);
			break;
		case ButtonRelease:
		//	handle_button_release(&e.xbutton);
			break;
		case MotionNotify:
		//	handle_motion_notify(&e.xmotion);
			break;
		case FocusIn:
		//	handle_focus_event(&e);
			break;
		case FocusOut:
		//	handle_unfocus_event(&e);
			break;
		case EnterNotify:
		//	handle_enter_notify(&e);
			break;
		case LeaveNotify:
		//	handle_leave_notify(&e);
			break;
		case UnmapNotify:
		//	handle_unmap_notify(&e.xunmap);
			break;
		case DestroyNotify:
		//	handle_destroy_notify(&e.xdestroywindow);
			break;
//		case PropertyNotify:
//			handle_property_notify(&e.xproperty);
//			break;
		}

//		XUngrabServer(display);

	}
	return EXIT_SUCCESS;
}
