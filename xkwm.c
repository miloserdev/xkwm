//#include <bits/signum-arch.h>
//#include <bits/signum-generic.h>
//#include <bits/types/FILE.h>

#define __GNU_SOURCE
#include <dlfcn.h>

#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>

#include <X11/X.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include <sys/types.h>
#include <fcntl.h>

#include <setjmp.h>
#define try int __error=-1; 		//bool __HadError=false;
#define catch(x) ExitJmp:if(__error > -1)
#define throw(x) {__error = x; goto ExitJmp;}

#define KeyMask Mod4Mask

void kz(int x) {
	if ( signal(SIGCHLD, kz) == SIG_ERR ) { exit(-1); }
	while (0 < waitpid(-1, NULL, WNOHANG) );
}


FILE *log_file;

void _log( const char *fmt, ... ) {
	log_file = fopen("~/xkwm.log", "a");
	if (log_file == NULL) { exit(1); }
	va_list args;
	va_start(args, fmt);
	vfprintf(log_file, fmt, args);
	va_end(args);
	fclose(log_file);
}


void segv_handler(int nSignum, siginfo_t* si, void* vcontext) {
	_log("______SEGFAULT______ADDR_%p\n", si->si_addr);

	ucontext_t* context = (ucontext_t*)vcontext;
}


int lerp(int a, int b, int t) {
	return a + t * (b - a);
}


static int (*xerrorxlib)(Display*, XErrorEvent *);


static int xerrorstart(Display *display, XErrorEvent *e) {
	_log("another instance" -1);
	return -1;
}


static int xerror(Display *display, XErrorEvent *e) {
	_log("request code %d, error code: %d\n", e->request_code, e->error_code);
	return xerrorxlib(display, e);
}


static int xerrordummy(Display *display, XErrorEvent *e) {
	_log("xid: %s, request code: %d, error code: %d...idle....   \n", e->resourceid, e->request_code, e->error_code);
}


unsigned long _RGB(int r, int g, int b) {
	return b + (g << 8) + (r << 16);
}


static Display *display;
static Window root;
int rootw;

static int def_screen;
static int sw, sh;

static int padding_top = 0;//30;
static int padding_bottom = 0;
static int padding_left = 0;
static int padding_right = 0;


unsigned long black, white;

static Window checkwin;

Window under_mouse = 0;

Font font;
Cursor cursor, cursor_move, cursor_size;

int border_width = 1;
int border_color;
int border_color_focused;
bool fs_border = false;

bool add_stackable = true;

int mon_count = 3;
int curr_mon = 1;



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
	Atom netdesktop	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
	Atom netdock	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
	Atom nettoolbar	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
	Atom netmenu	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_MENU", False);
	Atom netutility	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
	Atom netsplash	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
	Atom netdialog	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	Atom netnormal	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);

	Atom netwmtype = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	Atom type = get_atom(window, netwmtype);

	if (type == netdesktop) {
		_log("__________%s______________WINDOW_%d__ATYPE_DESKTOP__________\n", __FUNCTION__, window);
	} else if (type == netdock) {
		_log("__________%s______________WINDOW_%d__ATYPE_DOCK__________\n", __FUNCTION__, window);
	} else if (type == nettoolbar) {
		_log("__________%s______________WINDOW_%d__ATYPE_TOOLBAR__________\n", __FUNCTION__, window);
	} else if (type == netmenu) {
		_log("__________%s______________WINDOW_%d__ATYPE_MENU__________\n", __FUNCTION__, window);
	} else if (type == netutility) {
		_log("__________%s______________WINDOW_%d__ATYPE_UTILITY__________\n", __FUNCTION__, window);
	} else if (type == netsplash) {
		_log("__________%s______________WINDOW_%d__ATYPE_SPLASH__________\n", __FUNCTION__, window);
	} else if (type == netdialog) {
		_log("__________%s______________WINDOW_%d__ATYPE_DIALOG__________\n", __FUNCTION__, window);
	} else if (type == netnormal) {
		_log("__________%s______________WINDOW_%d__ATYPE_NORMAL__________\n", __FUNCTION__, window);
	} else {
		_log("__________%s______________WINDOW_%d__ATYPE_UNKNOWN__________\n", __FUNCTION__, window);
	}
}


typedef struct program {
	Window window;
	int id;
	int monitor;
	bool raised;
	bool fullscreen;
	bool stacked;

	int x, y;
	int width, height;
	int x_fs, y_fs, wid_fs, hei_fs;
	int x_rs, y_rs, wid_rs, hei_rs;

	struct program *next;
	struct program *prev;
} program;


struct program *last, *first;
int prog_count = 0;


void init_prog() {
	last = (struct program*) malloc(sizeof(struct program));
	last->id = 0;
	last->window = root;
	last->monitor = 0;

	last->next = last;
	last->prev = last;
	first = last;

	_log("__________%s______________ROOT_WINDOW_INIT_%d___MONITOR_%d__________\n", __FUNCTION__, last->window, last->monitor);

	prog_count++;
}


program * add_prog(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	if (win == root || win == rootw || win == 1) {
		_log("__________%s______________IMPOSTOR______WINDOW_%d__________\n", __FUNCTION__, win);
		return NULL;
	}

	XWindowAttributes xwa;
	if (!XGetWindowAttributes(display, win, &xwa)) return NULL;
	//XGetWindowAttributes(display, win, &xwa);

	struct program *obj;
	bool exist = false;

	program *prog = first;
	for (;;) {
		prog = prog->next;
		if (prog->window == win) {
			_log("__________%s______________WINDOW_%d___EXIST__________\n", __FUNCTION__, win);
			exist = true;
			obj = prog;
		}
		if (prog == first) break;
	}

	if (!exist) {
		obj = (struct program*) malloc(sizeof(struct program));
		obj->next = first;
		obj->prev = last;
		last->next = obj;
		last = last->next;
	}

	obj->id = prog_count;
	obj->window = win;
	obj->monitor = curr_mon;
	obj->raised = true;
	obj->fullscreen = false;
	obj->stacked = add_stackable;

	obj->x = xwa.x;
	obj->y = xwa.y;
	obj->width = xwa.width;
	obj->height = xwa.height;
	obj->x_rs = 0;
	obj->y_rs = 0;
	obj->x_fs = 0;
	obj->y_fs = 0;

	prog_count++;

	_log("__________%s______________WINDOW_%d___%d_<_%d_>_%d__________\n", __FUNCTION__, win, obj->prev->window, obj->window, obj->next->window);
	counter();
	return obj;

}


program * get_prog(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	program *prog = first;
	for (;;) { //for (int p = 0; p <= prog_count; p++) {
		prog = prog->next;
		if (prog->window == win) {
			_log("__________%s______________WINDOW_%d___FOUND__________\n", __FUNCTION__, win);
			return prog;
		}
		if (prog == first) break;
	}

//	if (win == 1) return first;

	_log("__________%s______________WINDOW_%d___DOES_NOT_EXIST__________\n", __FUNCTION__, win);

	return NULL;
}


void killer(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	XEvent event;
	event.xclient.type = ClientMessage;
	event.xclient.window = win;
	event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", False); // True
	event.xclient.format = 32;
	event.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
	event.xclient.data.l[1] = CurrentTime;
	int result = XSendEvent(display, win, False, NoEventMask, &event);

	if (!result) {
		XGrabServer(display);

		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(display, DestroyAll);
		XKillClient(display, win);
		XSync(display, False);

		XSetErrorHandler(xerror);
		XUngrabServer(display);
	}
}

Window under_pointer() {
	_log("__________%s________________________\n", __FUNCTION__);
	int ret, pos;
	unsigned int mask;
	Window child, contnr;
	ret = XQueryPointer(display, root, &contnr, &child, &pos, &pos, &pos, &pos, &mask);
	if (!ret) {
		return root;
	}
	_log("__________%s______________WINDOW_UNDER_POINTER_%d__________\n", __FUNCTION__, ret);
	return ret;
}







//mons

typedef struct monitor {
	Window last_focused;
	int id;
	struct monitor *next;
	struct monitor *prev;
} monitor;

struct monitor *last_mon, *first_mon;

void init_mon() {
	last_mon = (struct monitor*) malloc(sizeof(struct monitor));
	last_mon->id = 0;
	last_mon->next = last_mon;
	last_mon->prev = last_mon;
	first_mon = last_mon;

	_log("__________%s______________ROOT_MONITOR_INIT_%d___ID_%d__________\n", __FUNCTION__, last_mon, last_mon->id);
}

void add_mon(int numb) {
	if (numb == 0) return;

	struct monitor *obj;
	bool exist = false;

	monitor *mon = first_mon;
	for (;;) {
		mon = mon->next;
		if (mon->id == numb) return;
		if (mon == first_mon) break;
	}

	obj = (struct monitor*) malloc(sizeof(struct monitor));
	obj->last_focused = 0;
	obj->next = first_mon;
	obj->prev = last_mon;
	last_mon->next = obj;
	last_mon = last_mon->next;
	obj->id = numb;

	_log("__________%s______________MONITOR_%d_ADDED__________\n", __FUNCTION__, numb);
}


monitor *get_mon(int numb) {
	_log("__________%s______________MONITOR_%d__________\n", __FUNCTION__, numb);

	monitor *mon = first_mon;
	for (;;) {
		mon = mon->next;
		if (mon->id == numb) return mon;
		if (mon == first_mon) break;
	}
	return NULL;
}


void rem_prog(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	program *prog = first;

	uint32_t i = 0;
	uint32_t max_stack = 255;

	for (;;) { //for (int i = 0; i <= prog_count; i++) {
		if (i > max_stack) {
			_log("__________%s______________MAX_STACK___WINDOW_%d__________\n", __FUNCTION__, prog->window);
			break;
		}
		i++;

		prog = prog->next;
		_log("__________%s______________REMOVE_ITERATION___WINDOW_%d__________\n", __FUNCTION__, prog->window);

		if (prog->window == win) {
			_log("__________%s______________WINDOW_%d___REMOVING___NEXT_%d___PREV_%d___LAST_%d_____________\n", __FUNCTION__, win, prog->next->window, prog->prev->window, last->window);

			prog->next->prev = prog->prev;
			prog->prev->next = prog->next;


		//	free(first);			

			last = prog->next != first ? prog->next : prog->prev;
//			last = prog != first ? prog->next : prog->prev;

			_log("__________%s______________WINDOW_%d___REMOVING___NEXT_%d___PREV_%d___LAST_%d_____________\n", __FUNCTION__, win, prog->next->window, prog->prev->window, last->window);

			killer(win);

			//monitor *mon = get_mon(prog->monitor);
			//mon->last_focused = prog->prev;
			//if (mon->last_focused != 0) focus(mon->last_focused);
			change_program(prog->window, prog->monitor);
			stack_programs(prog->monitor);

			_log("__________%s______________WINDOW_%d___REMOVED___ID_%d__________\n", __FUNCTION__, win, prog->id);
			break;

//			if (prog_count > 1) prog_count--;
		}
		if (prog == first) break;

	}
}




void restack_mon(int numb) {
	_log("__________%s______________CURR_MON_%d__________\n", __FUNCTION__, curr_mon);
	monitor *mon_ = get_mon(numb);
	program *upd = first;
	for (;;) {
		upd = upd->next;
		if (upd->window != root && upd->window != 1) {
			if (upd->monitor != curr_mon) {
				if (upd->raised) {
					XWindowAttributes xwa;
					XGetWindowAttributes(display, upd->window, &xwa);
					upd->x_rs = xwa.x;
					upd->y_rs = xwa.y;
					upd->width = xwa.width;
					upd->height = xwa.height;
				}
				XMoveResizeWindow(display, upd->window, upd->x, ((1080 * 2) + 100), upd->width, upd->height);
				upd->raised = false;
			} else {
				XMoveResizeWindow(display, upd->window, upd->x_rs, upd->y_rs, upd->width, upd->height);
				upd->raised = true;
			}
		}
		if (upd == first) break;
	}

	//	monitor *currmon_ = get_mon(curr_mon);

/*	program *tmpp = first;
	for (;;) {
		tmpp = tmpp->next;
		//sss if (tmpp->monitor 
		if (tmpp == first) break;
	}
*/
	if (mon_->last_focused != 0) {
		focus(mon_->last_focused);
	} else {
		XSetInputFocus(display, root, RevertToPointerRoot, CurrentTime);
	}
}

/// MAKE FOCUS TO LAST WINDOW PREV OF MOVED	


void change_mon(int numb, Window focused, int _mask) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, focused);
	_log("__________%s______________MASK_%d__________\n", __FUNCTION__, _mask);

	if (_mask == (KeyMask | ControlMask) ) {
		if (focused == root || focused == 1) return;
		program *prog = get_prog(focused);
		if (prog->monitor != numb) {
			prog->monitor = numb;
			_log("__________%s______________WINDOW_%d___NOW_MONITOR_%d______CURR_MON_%d__________\n", __FUNCTION__, prog->window, prog->monitor, curr_mon);
		}

	} else {
		curr_mon = numb;
	}

	monitor *tmpmons = get_mon(curr_mon);
	program *tmpprog = get_prog(tmpmons->last_focused);
	if (tmpmons != NULL && tmpprog != NULL && tmpprog->monitor != curr_mon) {

		program *tmp = first;
		for (;;) {
			tmp = tmp->next;
			if (tmp->window != root && tmp->monitor == curr_mon) {
				tmpmons->last_focused = tmp->window;
				_log("__________%s______________CURRMON_%d___MONITOR_%d___LASTFOCUSED_%d__________\n", __FUNCTION__, curr_mon, tmpmons->id, tmpmons->last_focused);

			//	break;
			}
			if (tmp == first) break;
		}

	}


	restack_mon(curr_mon);
	stack_programs(curr_mon, false);


	counter();
}












void update_border(Window win, int width, int color) {
	_log("__________%s_____________WINDOW_%d__________\n", __FUNCTION__, win);
	XSetWindowBorderWidth(display, win, width);
	XSetWindowBorder(display, win, color);
}


void update_bordered(Window win) {
	_log("__________%s_____________WINDOW_%d__________\n", __FUNCTION__, win);
	program *prog = get_prog(win);
	if (prog == NULL || prog->window == root) return;

	int b_w = border_width;
	if (prog->fullscreen && !fs_border) b_w = 0;
	update_border(win, b_w, border_color);
}


void focus(Window window) {
	_log("__________%s_____________WINDOW_%d__________\n", __FUNCTION__, window);

	XRaiseWindow(display, window);
	XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);

	program *prog = get_prog(window);
	if (prog == NULL) return;
	monitor *mon = get_mon(prog->monitor);
	mon->last_focused = window;

	update_bordered(window);
	update_focused();

//	update_border(window, border_width, border_color_focused );
}


Window get_focused() {
//	_log("__________%s______________________\n", __FUNCTION__);
	Window winfoc;
	int rev_foc;
	XGetInputFocus(display, &winfoc, &rev_foc);
//	_log("__________%s_____________WINDOW_FOCUSED_%d__________\n", __FUNCTION__, winfoc);
	return winfoc;
}


void update_focused() {
	_log("__________%s______________________\n", __FUNCTION__);

	Window focused = get_focused();
	if (focused == root || focused == 1) return;

	program *upd = first;
	for (;;) {
		upd = upd->next;
		if (upd->monitor == curr_mon) {
			int b_w = border_width;
			if (upd->fullscreen && !fs_border) b_w = 0;

			if (upd->window == focused) {
				update_border(upd->window, b_w, border_color_focused);
			} else {
				update_border(upd->window, b_w, border_color);
			}
		}
		if (upd == first) break;
	}
}


void change_program(Window win, int mon) {
	_log("__________%s______________MONITOR_%d__________\n", __FUNCTION__, mon);

	program *prog = get_prog(win);
	if (prog == NULL) prog = first;//return;

	update_bordered(prog->window);

	int count = 0;
	uint32_t max_stack = 255;

	for (;;) {//for (int i = 0; i <= prog_count; i++) {

		if (count > max_stack) {
			_log("__________%s______________MAX_STACK___WINDOW_%d__________\n", __FUNCTION__, prog->window);
			break;
		}
		count++;


		prog = prog->next;
		_log("__________%s______________PROG_%d___WINDOW_%d__________\n", __FUNCTION__, prog->id, prog->window);

		if (prog->monitor == mon) {
			if (prog->window != root || prog->window != 1) {
				focus(prog->window);
				break;
			}
		}
	}
}


void counter() {
	Window focused = get_focused();
	_log("__________%s______________MONITOR_%d__________\n", __FUNCTION__, focused);

	int count = 0;
	program *prog = first;
	for (;;) {
		prog = prog->next;
	//	_log("__________%s______________COUNT___WINDOW_%d___ID_%d__________\n", __FUNCTION__, prog->window, prog->id);

		_log("			__________%s______________COUNT___WINDOW_%d___ID_%d___NEXT_%d___PREV_%d___LAST_%d___MONITOR_%d___PROG_STACKED_%b_____________\n", __FUNCTION__, prog->window, prog->id, prog->next->window, prog->prev->window, last->window, prog->monitor, prog->stacked);

		count++;
		if (prog == first) break;

	}
	_log("			__________%s______________COUNT_%d__________\n", __FUNCTION__, count);


	_log("			__________%s______________FOCUSED_%d__________\n", __FUNCTION__, focused);

}


void switch_program(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	counter();

	program *prog1 = get_prog(win);
	if (prog1 == NULL) return;

	program *prog2 = first->next;

	_log("__________%s______________SWITCH_%d_ID_%d___TO_%d_ID_%d__________\n", __FUNCTION__, prog1->window, prog1->id, prog2->window, prog2->id);

	int temp1_id = prog1->id;
	int temp1_window = prog1->window;

	int temp2_id = prog2->id;
	int temp2_window = prog2->window;

	prog2->id = prog1->id;
	prog2->window = prog1->window;

	prog1->id = temp2_id;
	prog1->window = temp2_window;


	_log("__________%s______________SWITCHED_%d_ID_%d___TO_%d_ID_%d__________\n", __FUNCTION__, prog1->window, prog1->id, prog2->window, prog2->id);

	counter();

//	change_program(win, prog->monitor);
	focus(temp1_window);
	stack_programs(prog1->monitor, false);
}


void fullscreen(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);


	if (win == root || win == 1) return;
	program *prog = get_prog(win);
	if (prog == NULL) return;

	if ( prog->window != root && win != 1) {
		XWindowAttributes xwa;
		XGetWindowAttributes(display, prog->window, &xwa);

		if (prog->monitor == curr_mon) {
			if (!prog->fullscreen) {

				if (!prog->fullscreen) {
					prog->x_fs = xwa.x;
					prog->y_fs = xwa.y;
					prog->wid_fs = xwa.width;
					prog->hei_fs = xwa.height;
				}

				int wid = fs_border ? sw - (border_width * 2) : sw;
				int hei = fs_border ? sh - (border_width * 2) : sh;

				int wwid = 100;
				int whei = 100;
				int wx = 500;
				int wy = 500;

				XSetWindowBorderWidth(display, prog->window,
					(fs_border ? border_width : 0)			// not working!
				);

				update_border(prog->window, (fs_border ? border_width : 0), border_color);
				XMoveResizeWindow(display, prog->window, 0, 0, wid, hei);
				prog->fullscreen = true;
			} else {
				update_bordered(prog->window);
				XSetWindowBorderWidth(display, prog->window, border_width);
				XMoveResizeWindow(display, prog->window, prog->x_fs, prog->y_fs, prog->wid_fs, prog->hei_fs);
				prog->fullscreen = false;
			}
		}

		 focus(prog->window);
	}
}


void stack_programs(int mon, bool alls) {
	_log("__________%s_____________MONITOR_%d__________\n", __FUNCTION__, mon);

	int gap = 10;
	int _sw = sw, _sh = sh;
	int _bw = border_width + gap;
	int _x = 0 - _bw + gap, _y = 0 - _bw + gap;
	int ci = 1; //2;

	int count = 0;
	int progs = 0;
	program *prog = first; // last;
	for (;;) { //for (int i = 0; i <= prog_count + 10; i++) {

		prog = prog->next;
		progs++;
		if (prog->monitor == mon && (!alls ? prog->stacked == true : true) ) {
			prog->stacked = true;
			_log("__________%s_________________CURRENT_%d___ROOT_%d__________\n", __FUNCTION__, prog->window, root);
			count++;
		}
		if (prog == first) break;

	}

	_log("__________%s_________________PROGS_ALL_%d___PROGS_AT_SCREEN_%d__________\n", __FUNCTION__, progs, count);


	prog = first;

	for (;;) {
		prog = prog->next;

		_log("__________%s_________________NEXT_%d__________\n", __FUNCTION__, prog->window);

		if (prog->window != root && prog->monitor == curr_mon && (!alls ? prog->stacked == true : true) ) {

			if (ci % 2 != 0) { _sw = _sw / 2; if (ci >= count) _sw = _sw * 2;
			} else { _sh = _sh / 2; if (ci >= count) _sh = _sh * 2; }

			if (prog->fullscreen) {
				XMoveResizeWindow(display, prog->window, 0, 0, sw, sh);
			} else {
				XMoveResizeWindow(display, prog->window,
					_x + _bw + padding_left,
					_y + _bw + padding_top,
					(_sw - _bw * 2) - padding_left - padding_right,
					(_sh - _bw * 2) - padding_top - padding_bottom);
			}

			XLowerWindow(display, prog->window);
			_log("__________%s_________________WINDOW_%d_STACKED_X_%d_Y_%d_WIDTH_%d_HEIGHT_%d_________\n", __FUNCTION__, prog->window, _x + _bw, _y + _bw, _sw - _bw * 2, _sh - _bw * 2);

			if (ci % 2 != 0) { _x = _x + _sw;
			} else { _y = _y + _sh; }
			ci++;
		}
		if (prog == first) break;

	}

	prog = first;
	for (;;) {
		prog = prog->next;
		if (prog->window != root && prog->stacked == false) XRaiseWindow(display, prog->window);
		if (prog == first) break;
	}
}


void stack_window(Window win, int un_stack) {
	program *prog = get_prog(win);
	if (prog == NULL) return;

	switch(un_stack) {
		case 0:
			prog->stacked = !prog->stacked;
			break;
		case 1:
			prog->stacked = true;
			break;
		case 2:
			prog->stacked = false;
			break;

	}
	stack_programs(prog->monitor, false);

}


void move_resize_window(XButtonEvent *e) {
	_log("__________%s_____________CALLER_%d__________\n", __FUNCTION__, e->window);

	Window win = under_mouse;//get_focused();
	_log("__________%s_____________FOCUSED_%d__________\n", __FUNCTION__, win);

	if (win == root || win == 1) return;

	XWindowAttributes xwa;
	if (!XGetWindowAttributes(display, win, &xwa)) return;

	int wid = xwa.width, hei = xwa.height;
	int rx, ry, cx, cy = 0;
	unsigned int mask = 0;
	Window c, r;

	XQueryPointer(display, win, &r, &c, &rx, &ry, &cx, &cy, &mask);

	if (rx < xwa.x || ry < xwa.y || rx > xwa.x + xwa.width || ry > xwa.y + xwa.height) return;

	_log("__________%s_____________C_%d___R_%d___RX_%d___RY_%d___CX_%d___CY_%d__________\n", __FUNCTION__, c, r, rx, ry, cx, cy);

	stack_window(win, 2);

	XGrabPointer(display, root, True, ButtonPressMask | ButtonReleaseMask |
		ButtonMotionMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None,
		(e->button == Button1 ? cursor_move : cursor_size), CurrentTime);
	XEvent ev;

	do {
		XMaskEvent(display, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | PointerMotionMask | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
			case Expose:
			case MapRequest:			// return it back for not some pizdets
			case ConfigureRequest:
				break;
			case MotionNotify:
				XSetTransientForHint(display, root, win);
				if (e->button == Button1) {
					XMoveWindow(display, win, xwa.x + (ev.xmotion.x - rx), xwa.y + (ev.xmotion.y - ry));
				} else {
					wid = ev.xmotion.x - xwa.x;
					hei = ev.xmotion.y - xwa.y;
					XResizeWindow(display, win, wid, hei);
				}
				break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(display, CurrentTime);
}


void handle_focus_change(XFocusChangeEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
	update_focused();
//	print_types(e->window);
}


void handle_enter_notify(XEvent *ev) {
	XCrossingEvent *e = &ev->xcrossing;
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

	if (e->window == root || e->window == 1) return;
	under_mouse = e->window;

//	focus(e->window);
}


void handle_leave_notify(XEvent *ev) {
	XCrossingEvent *e = &ev->xcrossing;
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

//	if (e->window == root || e->window == 1) return;
//	focus(e->window);
}


void handle_expose(XExposeEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
	update_focused();
	XSync(display, False);
}


void handle_map_request(XMapRequestEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

	XWindowAttributes xwa;
	if (!XGetWindowAttributes(display, e->window, &xwa) || xwa.override_redirect) return;

	XSetWindowAttributes at;
	at.event_mask = SubstructureRedirectMask |
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
			FocusChangeMask ;

//	XChangeWindowAttributes(display, e->window, CWCursor | CWEventMask | NoEventMask, &at);
	XSelectInput(display, e->window, EnterWindowMask | LeaveWindowMask | FocusChangeMask | //Added Leave
		PropertyChangeMask | StructureNotifyMask);

//	XSetWindowBorderWidth(display, e->window, border_width);
//	XSetWindowBorder(display, e->window, border_color);
	update_bordered(e->window);

	program *prog = add_prog(e->window);
	monitor *mon = get_mon(curr_mon);
	mon->last_focused = prog->window;

	Atom del_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(display, e->window, &del_window, 1);

	XMapWindow(display, e->window);
	focus(e->window);

	stack_programs(prog->monitor, false);
//	update_focused();

	print_types(e->window);
}


//key_release
void handle_key_release(XKeyEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
}


//key_press
void handle_key_press(XKeyEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

	Window focused = get_focused();

	counter();

	int _mask = (e->state & (ShiftMask | ControlMask | Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask) );

	_log("__________%s______________MASK_%d___MASKCTRL_%d__________\n", __FUNCTION__, _mask, ControlMask);
	_log("__________%s______________MASKED_%d__________\n", __FUNCTION__, e->keycode);


	switch (XKeycodeToKeysym(display, e->keycode, 0)) {

//		case XK_F4:
		case XK_j:
			//padding_top = padding_top == 0 ? 30 : 0;
			stack_programs(curr_mon, false);
			break;
		case XK_f:
			if (_mask == (KeyMask | ControlMask) ) {
				stack_programs(curr_mon, true);
			} else {
				stack_programs(curr_mon, false);
			}
			break;

		case XK_c:
			if (e->state & KeyMask) {
				if (focused == root || focused == 1) return;
				rem_prog(focused);
				//killer(focused);
			}
			break;
		case XK_space:
			if (e->state & KeyMask) {
				stack_window(focused, 0);
			}
			break;
		case XK_Tab:
			if (_mask == (KeyMask | ControlMask) ) {
				switch_program(focused);
			} else {
				change_program(focused, curr_mon);
			}
			break;
		case XK_Return:
			fullscreen(focused);
			break;
//elf

		case XK_Left:
			curr_mon = curr_mon <= 1 ? 9 : curr_mon - 1;
			change_mon(curr_mon, focused, _mask);
			break;
		case XK_Right:
			curr_mon = curr_mon >= 9 ? 1 : curr_mon + 1;
			change_mon(curr_mon, focused, _mask);
			break;

		case XK_9:
			change_mon(9, focused, _mask);
			break;
		case XK_8:
			change_mon(8, focused, _mask);
			break;
		case XK_7:
			change_mon(7, focused, _mask);
			break;
		case XK_6:
			change_mon(6, focused, _mask);
			break;
		case XK_5:
			change_mon(5, focused, _mask);
			break;
		case XK_4:
			change_mon(4, focused, _mask);
			break;
		case XK_3:
			change_mon(3, focused, _mask);
			break;
		case XK_2:
			change_mon(2, focused, _mask);
			break;
		case XK_1:
			change_mon(1, focused, _mask);
			break;

		case XK_0:
			if (e->state & KeyMask) {
				if (fork() == 0) {
					if (display) close(ConnectionNumber(display));
					char *cmd[] = { "dmenu_run", NULL };
					setsid();
					execvp(cmd[0], cmd);
					perror("failed");
					exit(EXIT_SUCCESS);
				}
			}
			break;
	}
}


void handle_configure_request(XConfigureRequestEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

	Window focused = get_focused();

	XWindowAttributes xwa;
	XGetWindowAttributes(display, e->window, &xwa);

	long msize;
	XSizeHints size;
	if (!XGetWMNormalHints(display, e->window, &size, &msize)) {
		size.flags = PSize;
	}

	XWindowChanges wcs;
	wcs.x = e->x;
	wcs.y = e->y;
	wcs.width = e->width;
	wcs.height = e->height;
	wcs.border_width = border_width;
	wcs.sibling = e->above;
	wcs.stack_mode = e->detail;
	XConfigureWindow(display, e->window, e->value_mask, &wcs);

//	update_border(e->window);
	XSync(display, False);
}


void handle_configure_notify(XConfigureEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

	program *prog = get_prog(e->window);
	if (prog == NULL) {
	//	prog = add_prog(e->window);
	}

/*
	Atom netnormal	=	XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	Atom netwmtype = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
	Atom type = get_atom(e->window, netwmtype);

	if (type != netnormal) {
		prog->stacked = false;
	}
*/

	print_types(e->window);

//	_log("__________%s______________PROGRAM_%d__________\n", __FUNCTION__, prog->id);

//	update_focused();
}


void handle_button_press(XButtonEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

	Window win = under_mouse; //under_pointer();
	if (win == root || win == 1 || win == 0) return;
	focus(win);

	move_resize_window(e);
}


void handle_motion_notify(XMotionEvent *e) {
//	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

//	program *prog = get_focused();//get_prog(e->window);

	// if (prog == NULL || prog->window == root) return;
}


void handle_button_release(XButtonEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
}


void handle_unmap_notify(XUnmapEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
	rem_prog(e->window);
	XUnmapWindow(display, e->window);
}


void handle_destroy_notify2(XDestroyWindowEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
	XDestroyWindow(display, e->window);
	rem_prog(e->window);
}

void handle_destroy_notify(XEvent *ev) {
	XDestroyWindowEvent *e = &ev->xclient;
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);


//	print_types(e->window);

//	XDestroyWindow(display, e->window);
	rem_prog(e->window);
}


void main(void) {

	log_file = fopen("fuck.log", "w");
	fprintf(log_file, "");
	fclose(log_file);

	_log("_____________________________________\n");
	_log("________________BEGIN________________\n");
	_log("_____________________________________\n");


	//Segfault
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = segv_handler;
	sigaction(SIGSEGV, &action, NULL);



	kz(0);

	display = XOpenDisplay(display);
	def_screen = DefaultScreen(display);

	sw = DisplayWidth(display, def_screen);
	sh = DisplayHeight(display, def_screen);
	root = RootWindow(display, def_screen);
	rootw = XDefaultRootWindow(display);

	black = BlackPixel(display, def_screen);
	white = WhitePixel(display, def_screen);
	border_color = _RGB(20,20,20);
	border_color_focused = _RGB(255,100,100);

	XSync(display, False);
	XFlush(display);

	xerrorxlib = XSetErrorHandler(xerrorstart);
//	XSetErrorHandler(xerrordummy);
//	XSetIOErrorHandler(xerrordummy);



	XSync(display, False);
	//init progs
	init_prog();

	//init mons
	init_mon();
	for (int i = 0; i < 9; i++) {
		add_mon(i + 1);
	}

	font = XLoadFont(display, "-*-*-*-R-Normal--*-180-100-100-*-*");
	cursor = XCreateFontCursor(display, XC_left_ptr);
	cursor_move = XCreateFontCursor(display, XC_fleur);
	cursor_size = XCreateFontCursor(display, XC_sizing);


//	XSetFont(display, gc, font);
//	Drawable dr;

//	XSetBackground(display, gc, white);
//	XSetForeground(display, gc, black);

//	XDrawString(display, dr, gc, 120, 120, _num, strlen(_num));

	XSetWindowAttributes at;
	at.event_mask = SubstructureRedirectMask |
			SubstructureNotifyMask |
			StructureNotifyMask |
			ExposureMask |
			KeyPressMask |
			KeyReleaseMask |

			PointerMotionMask |
			ButtonPressMask |
			ButtonReleaseMask |

			PropertyChangeMask ;

	at.do_not_propagate_mask = 0;
	at.cursor = cursor;

	unsigned long at_mask = CWEventMask | CWCursor | NoEventMask; // | CWBackPixel; - Causes color blink at start;
	XSelectInput(display, root, at.event_mask);
	XSync(display, False);
	XChangeWindowAttributes(display, root, at_mask, &at);


	XUngrabKey(display, AnyKey, AnyModifier, root);

//	XGrabKey(display, XKeysymToKeycode(display, XK_F4), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_j), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_f), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_c), KeyMask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_space), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Tab), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Return), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Control_L), KeyMask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_0), KeyMask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_1), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_2), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_3), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_4), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_5), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_6), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_7), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_8), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_9), KeyMask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_Left), KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Right), KeyMask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_f), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_1), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_2), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_3), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_4), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_5), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_6), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_7), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_8), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_9), ControlMask | KeyMask, root, True, GrabModeAsync, GrabModeAsync);



	XGrabButton(display, Button1, KeyMask, root, False, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, KeyMask, root, False, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);

	XSync(display, False);
	XFlush(display);

	char *_num = "text";

	XGCValues xgc;
	xgc.foreground = white;
	xgc.background = black;

	GC gc = XCreateGC(display, root, GCForeground | GCBackground, &xgc);
	XFillRectangle(display, root, gc, 100, 100, 200, 200);




//asda
	XEvent e;
	while (1) {

		XDrawString(display, root, gc, 100, 100, _num, sizeof(_num));

	/*	program *prog = first;
		for (;;) { //for (int p = 0; p <= prog_count; p++) {
			prog = prog->next;
			if (prog != NULL && prog->window != root) XFillRectangle(display, prog->window, prog->gc, 100, 100, 200, 200);
			if (prog == first) break;
		}
	*/

		XNextEvent(display, &e);

//		Window ww = get_focused();

		switch (e.type) {
			case FocusIn:
			case FocusOut:
				handle_focus_change(&e.xfocus);
				break;
			case EnterNotify:
				handle_enter_notify(&e);
				break;
			case LeaveNotify:
				handle_leave_notify(&e);
				break;
		/*	case GraphicsExpose:
			case NoExpose:
			case VisibilityNotify:
			case UnmapNotify:
			case MapNotify:
			case ReparentNotify:
			case GravityNotify:
			case CirculateRequest:
			case CirculateNotify: _log("Circulate\n"); break;
			case PropertyNotify:
				_log("PropertyNotify\n");

				XPropertyEvent *ev = &e.xproperty;
				_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, ev->window);

				program *prog = get_prog(get_focused());//get_prog(ev->window);
				if (prog != NULL && prog->window != root) XFillRectangle(display, prog->window, prog->gc, 100, 100, 200, 200);

				break;
			case MotionNotify: _log("MotionNotify\n"); break;
			case CreateNotify: _log("CreateNotify\n"); break;
			case VisibilityNotify: _log("VisibilityNotify\n"); break;
			case GraphicsExpose: _log("GraphicsExpose\n"); break;
			case NoExpose: _log("NoExpose\n"); break;
			case ClientMessage: _log("ClientMessage\n"); break;

			case ColormapNotify: _log("ColormapNotify\n"); break;
			case SelectionNotify:
			case SelectionRequest:
			case SelectionClear:
				 _log("SelectionNotify\n"); break;
			case MappingNotify: _log("MappingNotify\n"); break;
			case GravityNotify: _log("GravityNotify\n"); break;
			case ReparentNotify: _log("ReparentNotify\n"); break;
	*/
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
			case DestroyNotify:
				//handle_destroy_notify(&e.xdestroywindow);
				handle_destroy_notify(&e);
				break;
			default:
				_log("__________%s______________UNKNOWN_EVENT_%d__________\n", __FUNCTION__, e.type);
				break;
		}
	}
}












/*
jmp_buf ret[16];
int rs = -1;
#define endtry rs--
#define try(e) rs++; if ((e = setjmp(ret[rs])) == 0)
#define catch else
#define perror(e) printf("Netsing level %d: error %dn", rs, e);
*/

/*
#define try do { jmp_buf ex_buf__; switch( setjmp(ex_buf__) ) { case 0: while (1) {
#define catch(x) break; case x:
#define finally break; } default: {
#define etry break; } } } while(0)
#define throw(x) longjmp(ex_buf__, x)
*/



/*
jmp_buf *g__ActiveBuf;
#define try jmp_buf __LocalJmpBuf; jmp_buf *__OldActiveBuf=g__ActiveBuf;bool __WasThrown=false;g__ActiveBuf=&__LocalJmpBuf;if(setjmp(__LocalJmpBuf)){__WasThrown=true;}else
#define catch(x) g__ActiveBuf=__OldActiveBuf;if(__WasThrown)
#define throw(x) longjmp(*g__ActiveBuf, 1);
*/
