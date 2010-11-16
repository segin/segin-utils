#include "matwm.h"

int all_iconic = 0;

void client_move(client *c, int x, int y) {
	if(x == client_x(c) && y == client_y(c))
		return;
	client_clear_state(c);
	c->x = x;
	c->y = y;
	client_update_pos(c);
	return;
}

void client_resize(client *c, int width, int height) {
	if(c->normal_hints.flags & PResizeInc) {
		width -= (width - ((c->normal_hints.flags & PBaseSize) ? c->normal_hints.base_width : 0)) % c->normal_hints.width_inc;
		height -= (height - ((c->normal_hints.flags & PBaseSize) ? c->normal_hints.base_height : 0)) % c->normal_hints.height_inc;
	}
	if(c->normal_hints.flags & PAspect) {
		if(height < (width *	c->normal_hints.min_aspect.y) / c->normal_hints.min_aspect.x)
			height = (width * c->normal_hints.min_aspect.y) / c->normal_hints.min_aspect.x;
		if(height > (width *	c->normal_hints.max_aspect.y) / c->normal_hints.max_aspect.x)
			height = (width * c->normal_hints.max_aspect.y) / c->normal_hints.max_aspect.x;
	}
	if(c->normal_hints.flags & PMinSize) {
		if(width < c->normal_hints.min_width)
			width = c->normal_hints.min_width;
		if(height < c->normal_hints.min_height)
			height = c->normal_hints.min_height;
	}
	if(c->normal_hints.flags & PMaxSize) {
		if(width > c->normal_hints.max_width)
			width = c->normal_hints.max_width;
		if(height > c->normal_hints.max_height)
			height = c->normal_hints.max_height;
	}
	if(width < MINSIZE)
		width = MINSIZE;
	if(height < MINSIZE)
		height = MINSIZE;
	if(width == client_width(c) && height == client_height(c))
		return;
	client_clear_state(c);
	c->width = width;
	c->height = height;
	client_update_size(c);
	return;
}

void client_focus(client *c) {
	client *prev = current;
	if((c && c->flags & DONT_FOCUS) || evh == drag_handle_event)
		return;
	if(current && !(current->flags & ICONIC))
		previous = current;
	current = c;
	if(prev)
		client_set_bg(prev, ibg, ifg);
	if(!c) {
		XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	} else {
		client_set_bg(c, bg, fg);
		if((c->desktop == desktop || c->desktop == STICKY) && isviewable(c->window) && evh != wlist_handle_event)
			XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
	}
	ewmh_set_active(c);
}

void client_raise(client *c) {
	int i;
	for(i = client_number(stacking, c); i > 0 && (client_layer(stacking[i - 1]) >= client_layer(c) || stacking[i - 1]->flags & ICONIC || (fullscreen_stacking == FS_ONTOP && c->flags & FULLSCREEN && c->layer <= NORMAL)); i--)
		stacking[i] = stacking[i - 1];
	stacking[i] = c;
	client_over_fullscreen(c); /* fullscreen windows can go above always-on-top windows, but we can also go above them with the help of this function */
	/* above function calls clients_apply_stacking(), so we are done */
}

void client_lower(client *c) {
	int i;
	for(i = client_number(stacking, c); i < cn - 1 && client_layer(stacking[i + 1]) <= client_layer(c) && !(stacking[i + 1]->flags & ICONIC); i++)
		stacking[i] = stacking[i + 1];
	stacking[i] = c;
	clients_apply_stacking();
}

void client_over_fullscreen(client *c) { /* help a window getting above fullscreen windows by lowering it until it is under always-on-top windows */
	int i, j, cc = client_number(stacking, c);
	client *d;
	if(fullscreen_stacking != FS_ONTOP)
		return;
	for(i = cc; i >= 0; i--)
		if(stacking[i]->flags & FULLSCREEN) {
			d = stacking[i];
			for(j = i; j < cn - 1 && client_layer(stacking[j + 1]) <= client_layer(d) && !(stacking[j + 1]->flags & ICONIC) && j < cc; j++)
				stacking[j] = stacking[j + 1];
			stacking[j] = d;
		}
	clients_apply_stacking();
}

void client_fullscreen(client *c) {
	int prev = client_layer(current);
	client_toggle_state(c, FULLSCREEN);
	if(fullscreen_stacking == FS_ALWAYS_ONTOP)
	  client_update_layer(current, prev);
	if(fullscreen_stacking == FS_ONTOP && !(c->flags & FULLSCREEN))
	  client_update_layer(current, TOP);
}

void client_set_layer(client *c, int layer) { /* for making windows always-on-top, always-below, etc */
	int i, prev = client_layer(c);
	c->layer = layer;
	if(client_layer(c) != prev)
		client_update_layer(c, prev);
	for(i = 0; i < c->nbuttons; i++)
		if(c->buttons[i].action == B_ONTOP || c->buttons[i].action == B_BELOW)
			button_draw(c, &c->buttons[i]);
}

void client_toggle_state(client *c, int state) { /* used directly only for maximizing, client_fullscreen also calls this  */
	if(!(c->flags & CAN_MOVE) || !(c->flags & CAN_RESIZE))
		return;
	if((c->flags & state) == state)
		c->flags ^= state;
	else {
		c->flags |= state;
		client_raise(c);
	}
	client_update(c);
	ewmh_update_state(c);
}

void client_expand_x(client *c, int d, int first) { /* calculate horizontal dimensions for expanding */
	int i;
	for(i = 0; i < cn; i++) {
		if(!client_visible(clients[i]) || (first ? c->y : c->expand_y) >= client_y(clients[i]) + client_height_total(clients[i]) || (first ? c->y + client_height_total(c) : c->expand_height) <= client_y(clients[i]))
			continue;
		if(d & EXPANDED_L && client_x(clients[i]) + client_width_total(clients[i]) <= c->x && client_x(clients[i]) + client_width_total(clients[i]) > c->expand_x)
			c->expand_x = client_x(clients[i]) + client_width_total(clients[i]);
		if(d & EXPANDED_R && client_x(clients[i]) >= c->x + (c->width + (client_border(c) * 2)) && client_x(clients[i]) < c->expand_width)
			c->expand_width = client_x(clients[i]);
	}
}

void client_expand_y(client *c, int d, int first) {  /* calculate vertical dimensions for expanding */
	int i;
	for(i = 0; i < cn; i++) {
		if(!client_visible(clients[i]) || (first ? c->x : c->expand_x) >= client_x(clients[i]) + client_width_total(clients[i]) || (first ? c->x + client_width_total(c) : c->expand_width) <= client_x(clients[i]))
			continue;
		if(d & EXPANDED_T && client_y(clients[i]) + client_height_total(clients[i]) <= c->y && client_y(clients[i]) + client_height_total(clients[i]) > c->expand_y)
			c->expand_y = client_y(clients[i]) + client_height_total(clients[i]);
		if(d & EXPANDED_B && client_y(clients[i]) >= c->y + (c->height + (client_border(c) * 2) + client_title(c)) && client_y(clients[i]) < c->expand_height)
			c->expand_height = client_y(clients[i]);
	}
}

void client_expand(client *c, int d, int a) { /* see expand key action in the manual page for explanation of what this does */
	if(!(c->flags & CAN_MOVE) || !(c->flags & CAN_RESIZE))
		return;
	if((c->flags & d) == d) {
		c->flags ^= c->flags & d;
		client_update(c);
		return;
	}
	d ^= (d & c->flags);
	c->expand_x = (d & EXPANDED_L) ? 0 : client_x(c);
	c->expand_y = (d & EXPANDED_T) ? 0 : client_y(c);
	c->expand_width = (d & EXPANDED_R) ? display_width : (client_x(c) + client_width_total(c));
	c->expand_height = (d & EXPANDED_B) ? display_height : (client_y(c) + client_height_total(c));
	if(a) {
		client_expand_y(c, d, 1);
		client_expand_x(c, d, 0);
	} else {
		client_expand_x(c, d, 1);
		client_expand_y(c, d, 0);
	}
	c->expand_width -= c->expand_x + (client_border(c) * 2);
	c->expand_height -= c->expand_y + (client_border(c) * 2) + client_title(c);
	c->flags |= d;
	client_update(c);
}

void client_toggle_title(client *c) { /* hide or unhide title bar */
	c->flags ^= HAS_TITLE;
	client_update_title(c);
}

void client_iconify(client *c) {
	int i;
	XEvent ev;
	if(c->flags & ICONIC || c->flags & DONT_LIST)
		return;
	nicons++;
	c->flags |= ICONIC;
	set_wm_state(c->window, IconicState);
	if(c->desktop == desktop || c->desktop == STICKY) {
		client_hide(c);
		XUnmapWindow(dpy, c->window); /* window needs to be unmapped so it can send maprequest to de-iconify itself */
		XIfEvent(dpy, &ev, &isunmap, (XPointer) &c->window); /* catch the UnmapNotify event so matwm doesn't think the client is gone */
	}
	if(!current)
		client_focus_first();
	if(c->desktop != desktop && c->desktop != STICKY)
		XMapWindow(dpy, c->wlist_item);
	for(i = client_number(stacking, c); i < cn - 1; i++)
		stacking[i] = stacking[i + 1];
	stacking[cn - 1] = c;
	ewmh_update_stacking();
	if(evh == wlist_handle_event)
		wlist_update();
}

void client_restore(client *c) { /* restores iconic client */
	if(!(c->flags & ICONIC))
		return;
	nicons--;
	XMapWindow(dpy, c->window);
	client_show(c);
	if(c->desktop != STICKY && c->desktop != desktop) {
		c->desktop = desktop;
		ewmh_update_desktop(c);
	}
	c->flags ^=ICONIC;
	client_raise(c);
	set_wm_state(c->window, NormalState);
	if(c == current)
		XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
	if(evh == wlist_handle_event)
		wlist_update();
	client_end_all_iconic();
}

void client_save(client *c) { /* bring a moved off-screen window back to the screen */
	int x = client_x(c), y = client_y(c);
	if(client_x(c) >= display_width)
		x = display_width - client_width_total(c);
	if(client_y(c) >= display_height)
		y = display_height - client_height_total(c);
	if(client_x(c) + client_width_total(c) <= 0)
		x = 0;
	if(client_y(c) + client_height_total(c) <= 0)
		y = 0;
	client_move(c, x, y);
}

void client_to_border(client *c, char *a) {
	int x = client_x(c), y = client_y(c);
	if(!(c->flags & CAN_MOVE))
		return;
	while(a && *a) {
		if(*a == 'l')
			x = ewmh_strut[0];
		if(*a == 'r')
			x = display_width - (client_width_total(c) + ewmh_strut[1]);
		if(*a == 't')
			y = ewmh_strut[2];
		if(*a == 'b')
			y = display_height - (client_height_total(c) + ewmh_strut[3]);
		a++;
	}
	client_move(c, x, y);
	client_raise(c);
	client_warp(c);
}

void client_iconify_all(void) { /* iconify all windows, to be eventually restored afterwards by calling this function again */
	int i;
	if(all_iconic) {
		for(i = 0; i < cn; i++)
			if(clients[i]->flags & RESTORE) {
				client_restore(clients[i]);
				clients[i]->flags ^= RESTORE;
			}
		all_iconic = 0;
	} else {
		for(i = 0; i < cn; i++)
			if(client_visible(clients[i]) && !(clients[i]->flags & DONT_LIST)) {
				client_iconify(clients[i]);
				clients[i]->flags |= RESTORE;
			} else clients[i]->flags ^= clients[i]->flags & RESTORE;
		all_iconic = 1;
	}
	ewmh_update_showing_desktop();
}

void client_end_all_iconic(void) { /* to exit state induced by above function without restoring all windows */
	if(all_iconic) {
		all_iconic = 0;
		ewmh_update_showing_desktop();
	}
}

void client_handle_button(client *c, XEvent ev, int d) { /* for when a mouse button is clicked or doubleclicked on a window */
	int action = buttonaction(ev.xbutton.button, d);
	if(!(c->flags & DONT_FOCUS)) {
		if(action == BA_MOVE)
			drag_start(MOVE, ev.xbutton.button, ev.xbutton.x_root, ev.xbutton.y_root);
		if(action == BA_RESIZE)
			drag_start(RESIZE, ev.xbutton.button, ev.xbutton.x_root, ev.xbutton.y_root);
	}
	if(action == BA_MAXIMIZE)
		client_toggle_state(c, MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B);
	if(action == BA_EXPAND)
		client_expand(c, EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B, 0);
	if(action == BA_ICONIFY)
		client_iconify(c);
	if(action == BA_CLOSE)
		delete_window(c);
	if(action == BA_RAISE)
		client_raise(c);
	if(action == BA_LOWER)
		client_lower(c);
}

