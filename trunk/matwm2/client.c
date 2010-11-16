#include "matwm.h"

client **clients = NULL, **stacking = NULL, *current = NULL, *previous = NULL; /* stacking is sorted from top to bottom */
int cn = 0, nicons = 0; /* cn keeps the number of clients, of wich nicons are iconic */

void client_add(Window w, bool mapped) {
	XWindowAttributes attr;
	int i, wm_state;
	#ifdef USE_SHAPE
	int di, bounding_shaped;
	unsigned int dui;
	#endif
	client *new = (client *) _malloc(sizeof(client));
	new->window = w;
	/* set client state */
	wm_state = get_wm_state(w);
	if(wm_state == WithdrawnState) {
		wm_state = get_state_hint(w);
		set_wm_state(w, (wm_state != WithdrawnState) ? wm_state : NormalState);
	}
	/* read attributes and fill up client structure with them */
	XGetWindowAttributes(dpy, w, &attr);
	new->width = attr.width;
	new->height = attr.height;
	new->oldbw = attr.border_width;
	new->layer = NORMAL;
	new->desktop = desktop;
	new->flags = HAS_TITLE | HAS_BORDER | HAS_BUTTONS | CAN_MOVE | CAN_RESIZE;
	if(wm_state == IconicState)
		new->flags |= ICONIC;
	#ifdef USE_SHAPE
	if(have_shape)
		if(XShapeQueryExtents(dpy, new->window, &bounding_shaped, &di, &di, &dui, &dui, &di, &di, &di, &dui, &dui) && bounding_shaped)
			new->flags |= SHAPED;
	#endif
	/* read hints - these eventually override stuff we have just set */
	get_normal_hints(new);
	get_mwm_hints(new);
	ewmh_get_hints(new);
	/* continue filling up structure - gxo and gyo need the hints to be read, and the screen to be set */
	new->x = attr.x; /* and client_update_screen() needs these */
	new->y = attr.y;
	client_update_screen(new);
	new->xo = gxo(new, true);
	new->yo = gyo(new, true);
	new->x -= new->xo;
	new->y -= new->yo;
	if(!mapped && map_center && !(new->normal_hints.flags & USPosition) && !(new->normal_hints.flags & PPosition)) {
		new->x = screens[cs].x + ((screens[cs].width / 2) - (new->width / 2));
		new->y = screens[cs].y + ((screens[cs].height / 2) - (new->height / 2));
	}
	XFetchName(dpy, w, &new->name);
	/* create the parent window */
	XSetWindowBorderWidth(dpy, w, 0);
	new->parent = XCreateWindow(dpy, root, client_x(new), client_y(new), client_width_total_intern(new), client_height_total_intern(new), (new->flags & HAS_BORDER) ? border_width : 0,
	                            depth, CopyFromParent, visual,
	                            CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWEventMask, &p_attr);
	new->title = XCreateWindow(dpy, new->parent, border_spacing, border_spacing, 1, 1, 0,
	                           depth, CopyFromParent, visual,
	                           CWOverrideRedirect | CWEventMask, &p_attr);
	new->wlist_item = XCreateWindow(dpy, wlist, 0, 0, 1, 1, 0,
	                                depth, CopyFromParent, visual,
	                                CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	#ifdef USE_XFT
	if(xftfont)
		new->wlist_draw = XftDrawCreate(dpy, new->wlist_item, visual, colormap);
	#endif
	buttons_create(new); /* must be called before client_update_name() is called, else the title window will be initially misplaced */
	client_update_name(new);
	XMapWindow(dpy, new->title);
	if(!(new->flags & DONT_LIST) && (new->flags & ICONIC || (new->desktop == desktop || new->desktop == STICKY)))
		XMapWindow(dpy, new->wlist_item);
	#ifdef USE_SHAPE
	set_shape(new);
	#endif
	/* select wich events we want from the client window */
	XSelectInput(dpy, w, PropertyChangeMask | FocusChangeMask);
	#ifdef USE_SHAPE
	XShapeSelectInput(dpy, w, ShapeNotifyMask);
	#endif
	client_grab_buttons(new);
	/* reparent the client window */
	XAddToSaveSet(dpy, w);
	XReparentWindow(dpy, w, new->parent, client_border_intern(new), client_border_intern(new) + client_title(new));
	if(new->flags & FULLSCREEN || new->flags & MAXIMIZED_L || new->flags & MAXIMIZED_R || new->flags & MAXIMIZED_T || new->flags & MAXIMIZED_B)
		client_update_size(new);
	else
		configurenotify(new);
	/* add the client to the client list and stacking, also map it if it schould be visible etc */
	cn++;
	clients_alloc();
	if(!(new->flags & ICONIC)) {
		for(i = cn - 1; i > 0 && (stacking[i - 1]->flags & ICONIC || client_layer(stacking[i - 1]) >= client_layer(new)); i--)
			stacking[i] = stacking[i - 1];
		stacking[i] = new;
		if(evh == drag_handle_event && current)
			client_raise(current); /* to make sure the window we are dragging stays on top */
		XMapWindow(dpy, new->window); /* only iconic windows expect to be unmapped */
		if(new->desktop == desktop || new->desktop == STICKY)
			client_show(new);
	} else {
		stacking[cn - 1] = new;
		nicons++;
	}
	clients[cn - 1] = new;
	if(evh == wlist_handle_event)
		wlist_update();
	else {
		i = 0;
		if(fullscreen_stacking == FS_ALWAYS_ONTOP)
			for(i = client_number(stacking, new); i >= 0; i--)
				if(stacking[i]->flags & FULLSCREEN && !(stacking[i]->flags & ICONIC) && (stacking[i]->desktop == desktop || stacking[i]->desktop == STICKY))
					break;
		/* if i now is > 0 theres a fullscreen window we cannot go above obstructing the new window so we omit focus_new behaviour */
		if(((focus_new && !i) || !current) && client_visible(new))
			client_focus(new, true);
	}
	if(!(new->flags & ICONIC)) /* client_visible isn't apropriate here (what if the window would be moved to another desktop later) */
		client_over_fullscreen(new);
	configurenotify(new); /* gvim compiled with with gtk and perhaps some other applications need this - yes, twice */
	ewmh_update_desktop(new);
	ewmh_update_allowed_actions(new);
	ewmh_update_state(new);
	ewmh_update_extents(new);
	ewmh_update_clist();
}

void client_show(client *c) {
	XMapWindow(dpy, c->parent);
	clients_apply_stacking();
}

void client_hide(client *c) {
	XUnmapWindow(dpy, c->parent);
	if(c == previous)
		previous = NULL;
	if(c == current) {
		if(evh == drag_handle_event)
			evh = drag_release_wait;
		client_focus(NULL, true);
	}
}

void client_deparent(client *c) { /* reparent a client's window to the root window */
	XReparentWindow(dpy, c->window, root, client_x(c) + c->xo, client_y(c) + c->yo);
	XSetWindowBorderWidth(dpy, c->window, c->oldbw);
	XRemoveFromSaveSet(dpy, c->window);
}

void client_remove(client *c) {
	int i;
	if(c->flags & ICONIC)
		nicons--;
	for(i = 0; i < c->nbuttons; i++)
		if(button_current == &c->buttons[i])
			button_current = NULL;
	#ifdef USE_XFT
	if(xftfont) {
		XftDrawDestroy(c->title_draw);
		XftDrawDestroy(c->wlist_draw);
	}
	#endif
	XDestroyWindow(dpy, c->parent);
	XDestroyWindow(dpy, c->wlist_item);
	free((void *) c->buttons);
	if(c->name != no_title)
		XFree(c->name);
	XFreePixmap(dpy, c->title_pixmap);
	for(i = client_number(clients, c) + 1; i < cn; i++)
		clients[i - 1] = clients[i];
	for(i = client_number(stacking, c) + 1; i < cn; i++)
		stacking[i - 1] = stacking[i];
	cn--;
	if(c == previous)
		previous = NULL;
	if(c == current) {
		current = NULL;
		client_focus_first();
	}
	free(c);
	if(!clients_alloc())
		return;
	if(evh == wlist_handle_event)
		wlist_update();
	ewmh_update_clist();
}

void client_grab_button(client *c, int button) {
	int na = buttonaction(button, 0), da = buttonaction(button, 1);
	if(!(!(c->flags & CAN_MOVE) && (na == BA_MOVE || na == BA_EXPAND || na == BA_MAXIMIZE || na == BA_NONE) && (da == BA_MOVE || da == BA_EXPAND || da == BA_MAXIMIZE || da == BA_NONE)) && !(!(c->flags & CAN_RESIZE) && (na == BA_RESIZE || na == BA_EXPAND || na == BA_MAXIMIZE || na == BA_NONE) && (da == BA_RESIZE || da == BA_EXPAND || da == BA_MAXIMIZE || da == BA_NONE)) && !(!(c->flags & CAN_MOVE) && !(c->flags & CAN_RESIZE) && (na == BA_MOVE || na == BA_RESIZE || na == BA_EXPAND || na == BA_MAXIMIZE || na == BA_NONE) && (da == BA_MOVE || da == BA_RESIZE || da == BA_EXPAND || da == BA_MAXIMIZE || da == BA_NONE)) && !(c->flags & DONT_FOCUS && (na == BA_MOVE || na == BA_RESIZE || na == BA_NONE) && (da == BA_MOVE || da == BA_RESIZE || da == BA_NONE))) {
		button_grab(c->parent, button, mousemodmask, ButtonPressMask | ButtonReleaseMask);
		if(nosnapmodmask && (na == BA_MOVE || na == BA_RESIZE || da == BA_MOVE || da == BA_RESIZE))
			button_grab(c->parent, button, nosnapmodmask | mousemodmask, ButtonPressMask | ButtonReleaseMask);
	}
}

void client_grab_buttons(client *c) {
	if(mousemodmask) {
		client_grab_button(c, Button1);
		client_grab_button(c, Button2);
		client_grab_button(c, Button3);
		client_grab_button(c, Button4);
		client_grab_button(c, Button5);
	}
	XGrabButton(dpy, AnyButton, AnyModifier, c->window, True, ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
}

void client_draw_title(client *c) { /* draw the title pixmap for a client */
	XFillRectangle(dpy, c->title_pixmap, (c == current) ? bgc : ibgc, 0, 0, c->title_width, text_height);
	#ifdef USE_XFT
	if(xftfont)
	  XftDrawString8(c->title_draw, (c == current) ? &xftfg : &xftifg, xftfont, 0, xftfont->ascent, (unsigned char *) c->name, strlen(c->name));
	else
	#endif
	XDrawString(dpy, c->title_pixmap, (c == current) ? gc : igc, 0, font->max_bounds.ascent, c->name, strlen(c->name));
}

void client_update_name(client *c) { /* apply changes in the name of a client */
	#ifdef USE_XFT
	XGlyphInfo extents;
	#endif
	if(!c->name)
		c->name = no_title;
	if(strlen(c->name) == 0)
		c->name = no_title;
	#ifdef USE_XFT
	if(xftfont) {
		XftTextExtents8(dpy, xftfont, (FcChar8 *) c->name, strlen(c->name), &extents);
		c->title_width = extents.xOff;
	} else
	#endif
	c->title_width = XTextWidth(font, c->name, strlen(c->name)) + 1;
	/* exept for the first time we call this function we schould free this pixmap first */
	c->title_pixmap = XCreatePixmap(dpy, c->title, c->title_width, text_height, depth);
	#ifdef USE_XFT
	/* destroy this too before recurrent calls */
	if(xftfont)
		c->title_draw = XftDrawCreate(dpy, c->title_pixmap, visual, colormap);
	#endif
	client_draw_title(c);
	XSetWindowBackgroundPixmap(dpy, c->title, c->title_pixmap);
	XMoveResizeWindow(dpy, c->title, client_title_x(c), border_spacing, client_title_width(c), text_height);
}

void client_set_bg(client *c, XColor color, XColor border) { /* set (and apply) the background color of a client's parent window */
	int i;
	XSetWindowBackground(dpy, c->parent, color.pixel);
	client_draw_title(c);
	XSetWindowBorder(dpy, c->parent, border.pixel);
	XSetWindowBackground(dpy, c->button_parent_left, color.pixel);
	XSetWindowBackground(dpy, c->button_parent_right, color.pixel);
	for(i = 0; i < c->nbuttons; i++)
		XSetWindowBackground(dpy, c->buttons[i].w, color.pixel);
	XSetWindowBackground(dpy, c->wlist_item, color.pixel);
	if(c->desktop == desktop || c->desktop == STICKY) {
		XClearWindow(dpy, c->parent);
		XClearWindow(dpy, c->title);
		XClearWindow(dpy, c->button_parent_left);
		XClearWindow(dpy, c->button_parent_right);
		buttons_draw(c);
		client_draw_title(c);
	}
	if(evh == wlist_handle_event) {
		XClearWindow(dpy, c->wlist_item);
		wlist_item_draw(c);
	}
}

void clients_apply_stacking(void) { /* apply changes in the stacking */
	int i;
	Window *wins = (Window *) _malloc((cn + 1) * sizeof(Window));
	wins[0] = wlist;
	for(i = 0; i < cn && !(stacking[i]->flags & ICONIC); i++)
		wins[i + 1] = stacking[i]->parent;
	XRestackWindows(dpy, wins, i + 1);
	free(wins);
	ewmh_update_stacking();
	if(evh == wlist_handle_event)
		wlist_update();
}

void client_update_pos(client *c) { /* apply changes in the position of a client */
	XMoveWindow(dpy, c->parent, client_x(c), client_y(c));
	configurenotify(c);
}

void client_update_size(client *c) { /* apply changes in the size of a client */
	int width = client_width(c);
	buttons_update(c);
	XMoveResizeWindow(dpy, c->title, client_title_x(c), border_spacing, client_title_width(c), text_height);
	XResizeWindow(dpy, c->parent, client_width_total_intern(c), client_height_total_intern(c));
	XResizeWindow(dpy, c->window, width, client_height(c));
	buttons_draw(c);
}

void client_update(client *c) { /* apply changes in position and size of a window */
	int width = client_width(c);
	buttons_update(c);
	XMoveResizeWindow(dpy, c->title, client_title_x(c), border_spacing, client_title_width(c), text_height);
	XMoveResizeWindow(dpy, c->parent, client_x(c), client_y(c), client_width_total_intern(c), client_height_total_intern(c));
	XMoveResizeWindow(dpy, c->window, client_border_intern(c), client_border_intern(c) + client_title(c), width, client_height(c));
	buttons_draw(c);
}

void client_update_title(client *c) { /* apply changes if titlebar is turned on or off */
	XMoveWindow(dpy, c->window, client_border_intern(c), client_border_intern(c) + client_title(c));
	client_update_size(c);
	ewmh_update_extents(c);
}

void client_update_layer(client *c, int prev) { /* apply changes in stacking if the layer of a client is changed or if it's fullscreened */
	int i;
	if(client_layer(c) > prev)
		for(i = client_number(stacking, c); i < cn - 1 && client_layer(stacking[i + 1]) < client_layer(c) && !(stacking[i + 1]->flags & ICONIC); i++)
			stacking[i] = stacking[i + 1];
	else
		for(i = client_number(stacking, c); i > 0 && client_layer(stacking[i - 1]) > client_layer(c); i--)
			stacking[i] = stacking[i - 1];
	stacking[i] = c;
	clients_apply_stacking();
	ewmh_update_state(c);
}

void client_warp(client *c) { /* moves the pointer to a client */
	XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, client_width_total_intern(c) - 1, client_height_total_intern(c) - 1);
}

void client_focus_first(void) { /* to be called when focus window is lost */
	int i;
	if(previous && (previous->desktop == desktop || previous->desktop == STICKY)) {
		client_focus(previous, true);
		return;
	}
	for(i = 0; i < cn; i++)
		if(client_visible(stacking[i]) && !(stacking[i]->flags & DONT_FOCUS)) {
			client_focus(stacking[i], true);
			break;
		}
	if(i == cn)
		client_focus(NULL, true);
}

void client_clear_state(client *c) { /* to revert a client to normal state (as opposed to maximised, expanded or fullscreened) without restoring its previous dimensions */
	c->x = client_x(c);
	c->y = client_y(c);
	c->width = client_width(c);
	c->height = client_height(c);
	if(c->flags & (MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B | EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B | FULLSCREEN)) {
		c->flags ^= c->flags & (MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B | EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B | FULLSCREEN);
		ewmh_update_state(c);
	}
	client_update(c); /* needed for restoring the border and title of ex-fullscreen windows */
}

int clients_alloc(void) { /* to make sure enough memory is allocated for cn clients */
	client **newptr;
	if(!cn) {
		free(clients);
		free(stacking);
		clients = NULL;
		stacking = NULL;
		return 1;
	}
	newptr = (client **) _realloc((void *) clients, cn * sizeof(client *));
	clients = newptr;
	newptr = (client **) _realloc((void *) stacking, cn * sizeof(client *));
	stacking = newptr;
	return 1;
}

