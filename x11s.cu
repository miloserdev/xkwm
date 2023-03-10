//#include <bits/signum-arch.h>
//#include <bits/signum-generic.h>
//#include <bits/types/FILE.h>
#define N 10000000

#define __GNU_SOURCE
#include <dlfcn.h>

#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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


void kz(int x) {
	if ( signal(SIGCHLD, kz) == SIG_ERR ) { exit(-1); }
	while (0 < waitpid(-1, NULL, WNOHANG) );
}


FILE *fp;

void _log( const char *fmt, ... ) {
	fp = fopen("fuck.log", "a");
	if (fp == NULL) { exit(1); }
	va_list args;
	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fclose(fp);
}


int lerp(int a, int b, int t) {
	return a + t * (b - a);
}





#include <linux/cuda.h>
__global__ void interpolate(int count, float *result) {

	int globalIdx = blockIdx.x * blockDim.x + threadIdx.x;

	if (globalIdx < count) {
		result[globalIdx] = result[globalIdx] * 9.5f;
	}

//	int x = 1;
//	int i = 0;

/*	try {
	//	throw(0);
	//	*((unsigned int*)0) = 0xDEAD;
		_log("__S1_%d__", 1);
	} catch (x) {
		_log("Error occured %d", __error);
	}
*/

//	int x = 18874374;

//	char fpp = dlsym(RTLD_NOW, "XCreateWindow");
	//Window fptr = funcptr(0, 2354);

/*	for (;;) {
		_log("__________%s______________I_%d___WINDOW_%d___FPTR_%c__________\n", __FUNCTION__, i, x, 'a');
//		x = x + 2097152;
		x = x + 2097152;
		i++;
		if (x > 999999999) return;
	}
*/

//	free(i);
//	free(to);
//	free(last);
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


//static Display *display;
Display *display;
static Window root;
int rootw;

static int def_screen;
static int sw, sh;

unsigned long black, white;

static Window checkwin;

Font font;
Cursor cursor, cursor_move, cursor_size;

int border_width = 1;
int border_color;
int border_color_focused;
bool fs_border = true;

int mon_count = 3;
int curr_mon = 1;


typedef struct program {
	Window window;
	int id;
	int monitor;
	bool raised;
	bool fullscreen;

	GC gc;

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

program * get_prog(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	program *prog = first;
	for (;;) { //for (int p = 0; p <= prog_count; p++) {
		prog = prog->next;
		if (prog->window == win) return prog;
		if (prog == first) break;
	}

	if (win == 1) return first;

	_log("__________%s______________WINDOW_%d___DOES_NOT_EXIST__________\n", __FUNCTION__, win);

	return NULL;
}

program * add_prog(Window win) {

	_log("__________%s____________________RTLD_%d__________\n", __FUNCTION__, RTLD_NOW);

	if (win == root || win == rootw || win == 1) {
		_log("__________%s______________IMPOSTOR______WINDOW_%d__________\n", __FUNCTION__, win);
		return NULL;
	}

	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	XWindowAttributes xwa;
	if (!XGetWindowAttributes(display, win, &xwa)) return NULL;
	//XGetWindowAttributes(display, win, &xwa);

	struct program *obj;
	obj = (struct program*) malloc(sizeof(struct program));

	bool exist = false;

	program *prog = first;
	for (;;) {
		prog = prog->next;
		if (prog->window == win) exist = true;
		if (prog == first) break;
	}

	//if ( (get_prog(win) == NULL) ) {
	if ( !exist ) {
		_log("__________%s_________BEGIN_____WINDOW_%d__________\n", __FUNCTION__, win);
		obj->id = prog_count;
		obj->window = win;
		obj->monitor = curr_mon;
		obj->raised = true;
		obj->fullscreen = false;

		obj->x = xwa.x;
		obj->y = xwa.y;
		obj->width = xwa.width;
		obj->height = xwa.height;
		obj->x_rs = 0;
		obj->y_rs = 0;
		obj->x_fs = 0;
		obj->y_fs = 0;

		obj->next = first;
		obj->prev = last;

		last->next = obj;
		last = last->next;


		prog_count++;

		_log("__________%s______________WINDOW_%d___PROG_ADDED_%d__________\n", __FUNCTION__, win, obj->id);
		return obj;

	} else {
		_log("__________%s______________WINDOW_%d_EXIST_________\n", __FUNCTION__, win);
	}
	return NULL;
}


void rem_prog(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	program *prog = first;

	for (;;) { //for (int i = 0; i <= prog_count; i++) {
		prog = prog->next;
		_log("__________%s______________REMOVE_ITERATION___WINDOW_%d__________\n", __FUNCTION__, prog->window);

		if (prog->window == win) {
			_log("__________%s______________WINDOW_%d___REMOVING___NEXT_%d___PREV_%d___LAST_%d_____________\n", __FUNCTION__, win, prog->next->window, prog->prev->window, last->window);

			prog->next->prev = prog->prev;
			prog->prev->next = prog->next;
			last = prog->next != first ? prog->next : prog->prev;

			_log("__________%s______________WINDOW_%d___REMOVING___NEXT_%d___PREV_%d___LAST_%d_____________\n", __FUNCTION__, win, prog->next->window, prog->prev->window, last->window);

			_log("__________%s______________WINDOW_%d___REMOVED___ID_%d__________\n", __FUNCTION__, win, prog->id);
			break;

//			if (prog_count > 1) prog_count--;
		}
		if (prog == first) break;

	}

/*	if ( (prog = get_prog(win)) ) {
		prog->next->prev = prog->prev;
		prog->prev->next = prog->next;
		last = prog->next;
		prog_count--;
		_log("__________%s______________WINDOW_%d___PROG_REMOVED_%d__________\n", __FUNCTION__, win, prog->id);
	}
*/
//	for (int i = 0; i <= prog_count; i++) {
//		_log("__________%s______________ITERATION_PROG_%d__________\n", __FUNCTION__, prog->window);
//	}
}


void update_border(Window win, int width, int color) {
	XSetWindowBorderWidth(display, win, width);
	XSetWindowBorder(display, win, color);
}


void update_bordered(Window win) {
	update_border(win, border_width, border_color);
}


Window get_focused() {
	_log("__________%s______________________\n", __FUNCTION__);
	Window winfoc;
	int rev_foc;
	XGetInputFocus(display, &winfoc, &rev_foc);
	_log("__________%s_____________WINDOW_FOCUSED_%d__________\n", __FUNCTION__, winfoc);
	return winfoc;
}


void update_focused() {
	Window focused = get_focused();
	if (focused == root || focused == 1) return;

	program *upd = first;
	for (;;) {
		upd = upd->next;
		if (upd->monitor == curr_mon) {
			if (upd->window == focused) {
				update_border(upd->window, border_width, border_color_focused);
			} else {
				update_border(upd->window, border_width, border_color);
			}
		}
		if (upd == first) break;
	}
}


void focus(Window window) {
	_log("__________%s_____________WINDOW_%d__________\n", __FUNCTION__, window);

	update_focused();

	XRaiseWindow(display, window);
	XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);

	update_border(window, border_width, border_color_focused );
}


void change_program(Window win, int mon) {
	_log("__________%s______________MONITOR_%d__________\n", __FUNCTION__, mon);

	program *prog = get_prog(win);
	update_bordered(prog->window);

	int count = 0;
	for (;;) {//for (int i = 0; i <= prog_count; i++) {
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
	int count = 0;
	program *prog = first;
	for (;;) {
		prog = prog->next;
	//	_log("__________%s______________COUNT___WINDOW_%d___ID_%d__________\n", __FUNCTION__, prog->window, prog->id);

		_log("			__________%s______________COUNT___WINDOW_%d___ID_%d___NEXT_%d___PREV_%d___LAST_%d_____________\n", __FUNCTION__, prog->window, prog->id, prog->next->window, prog->prev->window, last->window);

		count++;
		if (prog == first) break;

	}
	_log("			__________%s______________COUNT_%d__________\n", __FUNCTION__, count);


	_log("			__________%s______________FOCUSED_%d__________\n", __FUNCTION__, focused);

}


void stack_programs(int mon) {
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
		if (prog->monitor == mon) {
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

		if (prog->window != root && prog->monitor == curr_mon) {

			if (ci % 2 != 0) { _sw = _sw / 2; if (ci >= count) _sw = _sw * 2;
			} else { _sh = _sh / 2; if (ci >= count) _sh = _sh * 2; }

			XMoveResizeWindow(display, prog->window, _x + _bw, _y + _bw, _sw - _bw * 2, _sh - _bw * 2);
			_log("__________%s_________________WINDOW_%d_STACKED_X_%d_Y_%d_WIDTH_%d_HEIGHT_%d_________\n", __FUNCTION__, prog->window, _x + _bw, _y + _bw, _sw - _bw * 2, _sh - _bw * 2);

			if (ci % 2 != 0) { _x = _x + _sw;
			} else { _y = _y + _sh; }
			ci++;
		}
		if (prog == first) break;
	}
}


void switch_program(Window win) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, win);

	counter();

	program *prog1 = get_prog(win);
	program *prog2 = first->next;

	_log("__________%s______________SWITCH_%d_ID_%d___TO_%d_ID_%d__________\n", __FUNCTION__, prog1->window, prog2->window);

	int temp1_id = prog1->id;
	int temp1_window = prog1->window;

	int temp2_id = prog2->id;
	int temp2_window = prog2->window;

	prog2->id = prog1->id;
	prog2->window = prog1->window;

	prog1->id = temp2_id;
	prog1->window = temp2_window;


	_log("__________%s______________SWITCHED_%d_ID_%d___TO_%d_ID_%d__________\n", __FUNCTION__, prog1->window, prog2->window);

	counter();

//	change_program(win, prog->monitor);
	focus(temp1_window);
	stack_programs(prog1->monitor);
}


void fullscreen(Window win) {

	if (win == root || win == 1) return;

	program *prog = get_prog(win);

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

			//	XSetWindowBorderWidth(display, prog->window, 
			//		(fs_border ? border_width : 0)
			//	);
				update_border(prog->window, (fs_border ? border_width : 0), border_color);
				XMoveResizeWindow(display, prog->window, 0, 0, wid, hei);
				prog->fullscreen = true;
			} else {
				update_bordered(prog->window);
			//	XSetWindowBorderWidth(display, prog->window, border_width);
				XMoveResizeWindow(display, prog->window, prog->x_fs, prog->y_fs, prog->wid_fs, prog->hei_fs);
				prog->fullscreen = false;
			}
		}

		focus(prog->window);
	}
}


void move_resize_window(XButtonEvent *e) {
	_log("__________%s_____________CALLER_%d__________\n", __FUNCTION__, e->window);

	Window win = get_focused();
	_log("__________%s_____________FOCUSED_%d__________\n", __FUNCTION__, win);

	XWindowAttributes xwa;
	XGetWindowAttributes(display, win, &xwa);

	int wid = xwa.width, hei = xwa.height;
	int rx, ry, cx, cy = 0;
	unsigned int mask = 0;
	Window c, r;

	XQueryPointer(display, win, &r, &c, &rx, &ry, &cx, &cy, &mask);

	XGrabPointer(display, root, True, ButtonPressMask | ButtonReleaseMask |
		ButtonMotionMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None,
		(e->button == Button1 ? cursor_move : cursor_size), CurrentTime);
	XEvent ev;

	do {
		XMaskEvent(display, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | PointerMotionMask | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
			case Expose:
			case MapRequest:
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
			ButtonPressMask |
			ButtonReleaseMask |
			EnterWindowMask |
			LeaveWindowMask |
			FocusChangeMask ;

//	XChangeWindowAttributes(display, e->window, CWEventMask | CWCursor, &at);
	XSelectInput(display, e->window, EnterWindowMask | FocusChangeMask |
		PropertyChangeMask | StructureNotifyMask);

//	XSetWindowBorderWidth(display, e->window, border_width);
//	XSetWindowBorder(display, e->window, border_color);
	update_bordered(e->window);

	program *prog = add_prog(e->window);

	XGCValues xgc;
	xgc.foreground = white;
	xgc.background = black;

	GC gc = XCreateGC(display, prog->window, GCForeground | GCBackground, &xgc);
	prog->gc = gc;
	XFillRectangle(display, prog->window, gc, 100, 100, 200, 200);


	XMapWindow(display, e->window);
	focus(e->window);
}


//key_release
void handle_key_press(XKeyEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
}


//key_press
void handle_key_release(XKeyEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

	Window focused = get_focused();

	counter();

	int _mask = (e->state & (ShiftMask | ControlMask | Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask) );

	_log("__________%s______________MASK_%d___MASKCTRL_%d__________\n", __FUNCTION__, _mask, ControlMask);

	switch (XKeycodeToKeysym(display, e->keycode, 0)) {

		case XK_Tab:
			if (_mask == (Mod1Mask | ControlMask) ) {
				switch_program(focused);
			} else {
				change_program(focused, curr_mon);
			}
			break;
		case XK_Return:
			fullscreen(focused);
			break;
		case XK_5:
			stack_programs(curr_mon);
			break;
		case XK_7:
			XLowerWindow(display, focused);
			break;
		case XK_0:
			if (e->state & Mod1Mask) {
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
	program *prog = get_prog(e->window);
	XFillRectangle(display, prog->window, prog->gc, 100, 100, 200, 200);

	update_focused();
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
}


void handle_button_press(XButtonEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);

	// if (e->window == root) return;

	move_resize_window(e);
}


void handle_button_release(XButtonEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
}


void handle_unmap_notify(XUnmapEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
//	rem_prog(e->window);
	XUnmapWindow(display, e->window);
}


void handle_destroy_notify(XDestroyWindowEvent *e) {
	_log("__________%s______________WINDOW_%d__________\n", __FUNCTION__, e->window);
	rem_prog(e->window);
}

void gpuscc() {
	_log("GPU -> init all\n");

	int count = 1000;
	float *result, *d_result;
	result = (float *) malloc(count * sizeof(float));
//	int *result = new int[count];
	for (int i = 0; i < count; i++) {
		result[i] = 1.3f;
	}

	_log("GPU -> result init\n");


	cudaMalloc(&d_result, count * sizeof(float));
	cudaMemcpy(d_result, result, count * sizeof(float), cudaMemcpyHostToDevice);

	_log("GPU -> cuda malloc\n");

	interpolate<<<1,1>>>(count, d_result);

	_log("GPU -> exec void on device\n");

	cudaDeviceSynchronize();
	_log("GPU -> syncing\n");

	cudaMemcpy(result, d_result, count * sizeof(float), cudaMemcpyDeviceToHost);
	_log("GPU -> memcpy from device to host\n");

	cudaFree(d_result);
	_log("GPU -> free cuda memory\n");


	_log("GPU complete\n");
	_log("from GPU %d\n", result);
}

int main(void) {

	fp = fopen("fuck.log", "w");
	fprintf(fp, "");
	fclose(fp);

	_log("_____________________________________\n");
	_log("________________1BEGIN________________\n");
	_log("_____________________________________\n");

	gpuscc();


	kz(0);

//	interpolate();

	display = (Display *) XOpenDisplay(NULL);
	def_screen = DefaultScreen(display);

	sw = DisplayWidth(display, def_screen);
	sh = DisplayHeight(display, def_screen);
	root = RootWindow(display, def_screen);
	rootw = XDefaultRootWindow(display);

	black = BlackPixel(display, def_screen);
	white = WhitePixel(display, def_screen);
	border_color = _RGB(100,100,100);
	border_color_focused = _RGB(255,255,255);

	XSync(display, False);
	XFlush(display);

	xerrorxlib = XSetErrorHandler(xerrorstart);
	XSetErrorHandler(xerrordummy);
	XSetIOErrorHandler( (XIOErrorHandler) xerrordummy);



	XSync(display, False);
	//init progs
	init_prog();


	font = XLoadFont(display, "-*-*-*-R-Normal--*-180-100-100-*-*");
	cursor = XCreateFontCursor(display, XC_left_ptr);
	cursor_move = XCreateFontCursor(display, XC_fleur);
	cursor_size = XCreateFontCursor(display, XC_sizing);


//	XSetFont(display, gc, font);
//	Drawable dr;

	char *_num;
	_num = "text";//prog->id;

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

	XGrabKey(display, XKeysymToKeycode(display, XK_0), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_5), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_7), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Tab), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Return), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_Control_L), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_1), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_2), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_3), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);

	XGrabKey(display, XKeysymToKeycode(display, XK_1), ControlMask | Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_2), ControlMask | Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, XKeysymToKeycode(display, XK_3), ControlMask | Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);



	XGrabButton(display, Button1, Mod1Mask, root, False, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(display, Button3, Mod1Mask, root, False, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);



	XSync(display, False);
	XFlush(display);



	XEvent e;
	XSync(display, False);

//asda

	while (1) {

		program *prog = first;
		for (;;) { //for (int p = 0; p <= prog_count; p++) {
			prog = prog->next;

			if (prog != NULL && prog->window != root) XFillRectangle(display, prog->window, prog->gc, 100, 100, 200, 200);

			if (prog == first) break;
		}


		XNextEvent(display, &e);

//		Window ww = get_focused();

		switch (e.type) {
		/*	case FocusIn:
			case FocusOut:
			case GraphicsExpose:
			case NoExpose:
			case VisibilityNotify:
			case UnmapNotify:
			case MapNotify:
			case ReparentNotify:
			case GravityNotify:*/
	/*		case CirculateRequest:
			case CirculateNotify: _log("Circulate\n"); break;
			case PropertyNotify:
				_log("PropertyNotify\n");

				XPropertyEvent *ev;
				ev = &e.xproperty;
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
			case ReparentNotify: { _log("ReparentNotify\n"); } break;

		//	case AnyEvent:
		//		program *prog = get_prog(&e.xany->window);
		//		if (prog != NULL) XFillRectangle(display, prog->window, prog->gc, 100, 100, 200, 200);
		//		break;
	*/
			case Expose: {
				handle_expose(&e.xexpose);
				}
				break;
			case KeyPress: {
				handle_key_press(&e.xkey);
				}
				break;
			case KeyRelease: {
				handle_key_release(&e.xkey);
				}
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
			case DestroyNotify:
				handle_destroy_notify(&e.xdestroywindow);
				break;
			default:
				_log("__________%s______________UNKNOWN_EVENT_%d__________\n", __FUNCTION__, e.type);
				break;
		}
	}
	return 0;
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
