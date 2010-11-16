#include "matwm.h"

client *clients = NULL;
int cn = 0, current = 0;

void add_client(Window w) {
  XWindowAttributes attr;
  XGetWindowAttributes(dpy, w, &attr);
  int i, owner_iconic = 0, di, bounding_shaped, wm_state = get_wm_state(w);
  alloc_clients();
  clients[cn].transient = get_wm_transient_for(w, &clients[cn].transient_for);
  if(clients[cn].transient)
    for(i = 0; i < cn; i++)
      if(clients[cn].transient_for == clients[i].window)
        owner_iconic = clients[i].iconic;
  if(wm_state == WithdrawnState)
    if(!owner_iconic) {
      wm_state = getstatehint(w);
      set_wm_state(w, wm_state != WithdrawnState ? wm_state : NormalState);
    } else set_wm_state(w, IconicState);
  if(wm_state == IconicState && clients[cn].transient && !owner_iconic)
    set_wm_state(w, NormalState);
  clients[cn].width = attr.width;
  clients[cn].height = attr.height;
  clients[cn].oldbw = attr.border_width;
  clients[cn].window = w;
  if(have_shape) {
    clients[cn].shaped = XShapeQueryExtents(dpy, clients[cn].window, &bounding_shaped, &di, &di, &di, &di, &di, &di, &di, &di, &di) && bounding_shaped;
  } else clients[cn].shaped = 0;
  getnormalhints(cn);
  get_mwm_hints(cn);
  clients[cn].x = attr.x - gxo(cn, 1);
  clients[cn].y = attr.y - gyo(cn, 1);
  clients[cn].iconic = (wm_state == IconicState) ? 1 : owner_iconic;
  clients[cn].maximised = 0;
  XFetchName(dpy, w, &clients[cn].name);
  XSelectInput(dpy, w, PropertyChangeMask | EnterWindowMask);
  XShapeSelectInput(dpy, w, ShapeNotifyMask);
  XSetWindowBorderWidth(dpy, w, 0);
  clients[cn].parent = XCreateWindow(dpy, root, clients[cn].x, clients[cn].y, total_width(cn), total_height(cn), 0,
                                     DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                     CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  clients[cn].icon = XCreateWindow(dpy, root, 0, 0, icon_width, title_height + 4, 0,
                                   DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                   CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, clients[cn].parent, border(cn), border(cn) + title(cn));
  grab_button(clients[cn].parent, AnyButton, mousemodmask, ButtonPressMask | ButtonReleaseMask);
  configurenotify(cn);
  cn++;
  if(!clients[cn - 1].iconic) {
    XMapWindow(dpy, w);
    restack_client(cn - 1, 1);
  } else sort_icons();
  XMapWindow(dpy, clients[cn - 1].iconic ? clients[cn - 1].icon : clients[cn - 1].parent);
  if(current >= cn - 1)
    focus(cn - 1);
  if(clients[cn - 1].shaped)
    XShapeCombineShape(dpy, clients[cn - 1].parent, ShapeBounding, border(cn - 1), border(cn - 1) + title(cn - 1), clients[cn - 1].window, ShapeBounding, ShapeSet);
}

void remove_client(int n, int fc) {
  XEvent ev;
  int i, iconic = clients[n].iconic;
  if(fc != 2 && XCheckTypedWindowEvent(dpy, clients[n].parent, DestroyNotify, &ev) == False) {
    if(fc)
      set_wm_state(clients[n].window, WithdrawnState);
    XReparentWindow(dpy, clients[n].window, root, clients[n].x + gxo(n, 1), clients[n].y + gyo(n, 1));
    XSetWindowBorderWidth(dpy, clients[n].window, clients[n].oldbw);
    XRemoveFromSaveSet(dpy, clients[n].window);
  }
  XDestroyWindow(dpy, clients[n].parent);
  XDestroyWindow(dpy, clients[n].icon);
  if(clients[n].name)
    XFree(clients[n].name);
  cn--;
  for(i = n; i < cn; i++)
    clients[i] = clients[i + 1];
  alloc_clients();
  if(iconic)
    sort_icons();
}

void draw_client(int n) {
  if(clients[n].name && (title(n) || clients[n].iconic))
    XDrawString(dpy, clients[n].iconic ? clients[n].icon : clients[n].parent, (n == current) ? gc : igc, (clients[n].iconic ? 2 : border_width) + font->max_bounds.lbearing, (clients[n].iconic ? 2 : border_width) + font->max_bounds.ascent, clients[n].name, strlen(clients[n].name));
  if(border(n) || clients[n].iconic) {
    XDrawRectangle(dpy, clients[n].iconic ? clients[n].icon : clients[n].parent, (n == current) ? gc : igc, 0, 0, (clients[n].iconic ? icon_width : (clients[n].width + (border_width * 2))) - 1, (clients[n].iconic ? title_height + 4 : (clients[n].height + (border_width * 2) + title(n))) - 1);
    XClearArea(dpy, clients[n].iconic ? clients[n].icon : clients[n].parent, clients[n].iconic ? icon_width - 3 : (clients[n].width + border_width - 1), (clients[n].iconic ? 2 : border_width), clients[n].iconic ? 2 : border_width, title_height, False);
  }
}

void alloc_clients(void) {
  client *newptr = (client *) realloc((client *) clients, (cn + 1) * sizeof(client));
  if(!newptr)
    error();
  clients = newptr;
}

void move(int n, int x, int y) {
  if(x == clients[n].x && y == clients[n].y)
    return;
  clients[n].maximised = 0;
  XMoveWindow(dpy, clients[n].parent, x, y);
  configurenotify(n);
  clients[n].x = x;
  clients[n].y = y;
}

void resize(int n, int width, int height) {
  if(clients[n].normal_hints.flags & PResizeInc) {
    width -= (width - ((clients[n].normal_hints.flags & PBaseSize) ? clients[n].normal_hints.base_width : 0)) % clients[n].normal_hints.width_inc;
    height -= (height - ((clients[n].normal_hints.flags & PBaseSize) ? clients[n].normal_hints.base_height : 0)) % clients[n].normal_hints.height_inc;
  }
  if(clients[n].normal_hints.flags & PAspect) {
    if(height < (width *  clients[n].normal_hints.min_aspect.y) / clients[n].normal_hints.min_aspect.x)
      height = (width * clients[n].normal_hints.min_aspect.y) / clients[n].normal_hints.min_aspect.x;
    if(height > (width *  clients[n].normal_hints.max_aspect.y) / clients[n].normal_hints.max_aspect.x)
      height = (width * clients[n].normal_hints.max_aspect.y) / clients[n].normal_hints.max_aspect.x;
  }
  if(clients[n].normal_hints.flags & PMinSize) {
    if(width < clients[n].normal_hints.min_width)
      width = clients[n].normal_hints.min_width;
    if(height < clients[n].normal_hints.min_height)
      height = clients[n].normal_hints.min_height;
  }
  if(clients[n].normal_hints.flags & PMaxSize) {
    if(width > clients[n].normal_hints.max_width)
      width = clients[n].normal_hints.max_width;
    if(height > clients[n].normal_hints.max_height)
      height = clients[n].normal_hints.max_height;
  }
  if(width < MINSIZE)
    width = MINSIZE;
  if(height < MINSIZE)
    height = MINSIZE;
  if(width == clients[n].width && height == clients[n].height)
    return;
  clients[n].maximised = 0;
  clients[n].width = width;
  clients[n].height = height;
  XResizeWindow(dpy, clients[n].parent, total_width(n), total_height(n));
  XResizeWindow(dpy, clients[n].window, width, height);
  draw_client(n);
}

void focus(int n) {
  int i = current < cn ? current : n;
  current = n;
  while(i < cn) {
    XSetWindowBackground(dpy, clients[i].parent, i == n ? bg.pixel : ibg.pixel);
    XSetWindowBackground(dpy, clients[i].icon, i == n ? bg.pixel : ibg.pixel);
    XClearWindow(dpy, clients[i].iconic ? clients[i].icon : clients[i].parent);
    draw_client(i);
    i = (i != n) ? n : cn;
  }
  if(!clients[n].iconic)
    XSetInputFocus(dpy, clients[n].window, RevertToPointerRoot, CurrentTime);
}

void next(int iconic) {
  int i = current + 1 < cn ? current + 1 : 0;
  while(i < cn && i != current) {
    if(iconic ? clients[i].iconic : !clients[i].iconic) {
      focus(i);
      restack_client(current, 1);
      warp();
      return;
    }
    i++;
    if(current < cn && i == cn)
      i = 0;
  }
}

void prev(int iconic) {
  int i = current - 1 >= 0 ? current - 1 : cn - 1;
  while(i >= 0 && i != current) {
    if(iconic ? clients[i].iconic : !clients[i].iconic) {
      focus(i);
      restack_client(current, 1);
      warp();
      return;
    }
    if(current < cn && !i)
      i = cn;
    i--;
  }
}

void restack_client(int n, int top) {
  int i;
  top ? XRaiseWindow(dpy, clients[n].parent) : XLowerWindow(dpy, clients[n].parent);
  restack_icons(0);
  for(i = 0; i < cn; i++)
    if(clients[i].transient && clients[i].transient_for == clients[n].window)
      restack_client(i, top);
}

void maximise(int n) {
  if(clients[n].maximised) {
    clients[n].maximised = 0;
    move(n, clients[n].prev_x, clients[n].prev_y);
    resize(n, clients[n].prev_width, clients[n].prev_height);
    return;
  }
  clients[n].prev_x = clients[n].x;
  clients[n].prev_y = clients[n].y;
  clients[n].prev_width = clients[n].width;
  clients[n].prev_height = clients[n].height;
  restack_client(current, 1);
  move(n, 0, 0);
  resize(n, display_width - (border(n) * 2), display_height - ((border(n) * 2) + title(n)));
  clients[n].maximised = 1;
}

void set_shape(int c) {
  int bounding_shaped, di;
  if(clients[c].shaped)
    XShapeCombineShape(dpy, clients[c].parent, ShapeBounding, border(c), border(c) + title(c), clients[c].window, ShapeBounding, ShapeSet);
}

