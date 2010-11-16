#include "matwm.h"
unsigned int mousemodmask = 0, numlockmask = 0;
XModifierKeymap *modmap;

void grab_key(Window w, unsigned int modmask, KeyCode key) {
  XGrabKey(dpy, key, modmask, w, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(dpy, key, LockMask | modmask, w, True, GrabModeAsync, GrabModeAsync);
  if(numlockmask) {
    XGrabKey(dpy, key, numlockmask | modmask, w, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, key, numlockmask | LockMask | modmask, w, True, GrabModeAsync, GrabModeAsync);
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

void drag(int n, XButtonEvent *be, int res) {
  int xo, yo;
  XEvent ev;
  if(res) {
    XWarpPointer(dpy, None, clients[n].parent, 0, 0, 0, 0, clients[n].width + border_width, clients[n].height + border_width + title_height);
    xo = clients[n].x + border_width;
    yo = clients[n].y + border_width + title_height;
  }
  XGrabPointer(dpy, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, 0, CurrentTime);
  while(1) {
    XMaskEvent(dpy, PropertyChangeMask | SubstructureNotifyMask | SubstructureRedirectMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask | EnterWindowMask, &ev);
    if(ev.type == MotionNotify) {
      while(XCheckTypedEvent(dpy, MotionNotify, &ev));
      if(res) {
        resize(n, ev.xmotion.x - xo,  ev.xmotion.y - yo);
      } else move(n, ev.xmotion.x - be->x, ev.xmotion.y - be->y);
    } else if(ev.type == ButtonRelease && ev.xbutton.button == be->button) {
      break;
    } else if(ev.type == EnterNotify || ev.type == ButtonPress) {
      continue;
    } else  {
      handle_event(ev);
      if((ev.type == UnmapNotify && ev.xunmap.window == clients[n].window) || ev.type == KeyPress)
        break;
    }
  }
  XUngrabPointer(dpy, CurrentTime);
}

int getmodifier(KeyCode key) {
  int i;
  for(i = 0; i < 8; i++)
    if(modmap->modifiermap[modmap->max_keypermod * i] == key)
      return (1 << i);
  return 0;
}

void mapkeys(void) {
  modmap = XGetModifierMapping(dpy);
  numlockmask = getmodifier(XKeysymToKeycode(dpy, XK_Num_Lock));
  string_to_key(xrm_getstr(cfg, "mouse_modifier", DEF_MOUSEMOD), &mousemodmask);
  key_next = xrm_getkey(cfg, "key_next", DEF_KEY_NEXT);
  key_prev = xrm_getkey(cfg, "key_prev", DEF_KEY_PREV);
  key_next_icon = xrm_getkey(cfg, "key_next_icon", DEF_KEY_NEXT_ICON);
  key_prev_icon = xrm_getkey(cfg, "key_prev_icon", DEF_KEY_PREV_ICON);
  key_iconify = xrm_getkey(cfg, "key_iconify", DEF_KEY_ICONIFY);
  key_close = xrm_getkey(cfg, "key_close", DEF_KEY_CLOSE);
  key_maximise = xrm_getkey(cfg, "key_maximise", DEF_KEY_MAXIMISE);
  key_bottomleft = xrm_getkey(cfg, "key_bottomleft", DEF_KEY_BOTTOMLEFT);
  key_bottomright = xrm_getkey(cfg, "key_bottomright", DEF_KEY_BOTTOMRIGHT);
  key_topleft = xrm_getkey(cfg, "key_topleft", DEF_KEY_TOPLEFT);
  key_topright = xrm_getkey(cfg, "key_topright", DEF_KEY_TOPRIGHT);
}
