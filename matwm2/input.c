#include "matwm.h"

void grab_keysym(Window w, int modmask, KeySym key) {
  XGrabKey(dpy, XKeysymToKeycode(dpy, key), modmask, w, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(dpy, XKeysymToKeycode(dpy, key), LockMask | modmask, w, True, GrabModeAsync, GrabModeAsync);
  if(numlockmask) {
    XGrabKey(dpy, XKeysymToKeycode(dpy, key), numlockmask | modmask, w, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, key), numlockmask | LockMask | modmask, w, True, GrabModeAsync, GrabModeAsync);
  }
}

void grab_button(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask) {
  XGrabButton(dpy, button, modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  XGrabButton(dpy, button, LockMask | modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  if(numlockmask) {
    XGrabButton(dpy, button, numlockmask | modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
    XGrabButton(dpy, button, numlockmask | LockMask | modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  }
}

void drag(int n, XButtonEvent *be) {
  int xo, yo;
  XEvent ev;
  if(be->button == resize_button) {
    XWarpPointer(dpy, None, clients[n].parent, 0, 0, 0, 0, clients[n].width + border_width,  clients[n].height + border_width + title_height);
    xo = clients[n].x + border_width;
    yo = clients[n].y + border_width + title_height;
  }
  XGrabPointer(dpy, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, 0, CurrentTime);
  while(1) {
    XMaskEvent(dpy, PropertyChangeMask | SubstructureNotifyMask | SubstructureRedirectMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask | EnterWindowMask, &ev);
    if(ev.type == MotionNotify) {
      if(be->button == resize_button) {
        resize(n, ev.xmotion.x - xo,  ev.xmotion.y - yo);
      } else move(n, ev.xmotion.x - be->x, ev.xmotion.y - be->y);
    } else if(ev.type == ButtonRelease && ev.xbutton.button == be->button) {
      break;
    } else if(ev.type == EnterNotify || ev.type == ButtonPress) {
      continue;
    } else if(ev.type == UnmapNotify && ev.xunmap.window == clients[n].window) {
      remove_client(n);
      break;
    } else handle_event(ev);
  }
  XUngrabPointer(dpy, CurrentTime);
}
