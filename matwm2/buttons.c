#include "matwm.h"

button *button_current = NULL;
int button_down = 0;

void buttons_create(client *c) {
	int i;
	c->button_parent_left = XCreateWindow(dpy, c->parent, border_width - 1, border_width - 1, 1, 1, 0,
	                                      DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                                      CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	c->button_parent_right = XCreateWindow(dpy, c->parent, 0, 0, 1, 1, 0,
	                                       DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                                       CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	c->buttons = (void *) malloc(sizeof(button) * (nbuttons_left + nbuttons_right));
	if(!c->buttons)
		error();
	c->nbuttons = 0;
	c->buttons_left_width = 0;
	c->buttons_right_width = 0;
	for(i = 0; i < nbuttons_left; i++) {
		if((buttons_left[i] == B_EXPAND || buttons_left[i] == B_MAXIMIZE) && !(c->flags & CAN_MOVE && c->flags & CAN_RESIZE))
			continue;
		c->buttons[c->nbuttons].w = XCreateWindow(dpy, c->button_parent_left, c->buttons_left_width, 0, button_size, button_size, 0,
		                                          DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
		                                          CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
		XMapWindow(dpy, c->buttons[c->nbuttons].w);
		c->buttons[c->nbuttons].action = buttons_left[i];
		c->buttons_left_width += button_size + 2;
		c->nbuttons++;
	}
	for(i = 0; i < nbuttons_right; i++) {
		if((buttons_right[i] == B_EXPAND || buttons_right[i] == B_MAXIMIZE) && !(c->flags & CAN_MOVE && c->flags & CAN_RESIZE))
			continue;
		c->buttons[c->nbuttons].w = XCreateWindow(dpy, c->button_parent_right, c->buttons_right_width, 0, button_size, button_size, 0,
																							DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
																							CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
		XMapWindow(dpy, c->buttons[c->nbuttons].w);
		c->buttons[c->nbuttons].action = buttons_right[i];
		c->buttons_right_width += button_size + 2;
		c->nbuttons++;
	}
	buttons_update(c);
	if(c->buttons_left_width) {
		XResizeWindow(dpy, c->button_parent_left, c->buttons_left_width, button_size);
		XMapWindow(dpy, c->button_parent_left);
	}
	if(c->buttons_right_width) {
		XResizeWindow(dpy, c->button_parent_right, c->buttons_right_width, button_size);
		XMapWindow(dpy, c->button_parent_right);
	}
}

void buttons_draw(client *c) {
	int i;
	for(i = 0; i < c->nbuttons; i++)
		button_draw(c, &c->buttons[i]);
}

void button_draw(client *c, button *b) {
	XClearWindow(dpy, b->w);
	if(button_current == b)
		XDrawRectangle(dpy, b->w, gc, 0, 0, button_size - 1, button_size - 1);
	if(b->action == B_ICONIFY)
		XDrawRectangle(dpy, b->w, (c == current) ? gc : igc, (button_size / 2) - 1, (button_size / 2) - 1, 2, 2);
	if(b->action == B_EXPAND) {
		XDrawLine(dpy, b->w, (c == current) ? gc : igc, button_size / 2, 3, button_size / 2, button_size - 3);
		XDrawLine(dpy, b->w, (c == current) ? gc : igc, 3, button_size / 2, button_size - 3, button_size / 2);
	}
	if(b->action == B_MAXIMIZE)
		XDrawRectangle(dpy, b->w, (c == current) ? gc : igc, 2, 2, button_size - 5, button_size - 5);
	if(b->action == B_CLOSE) {
		XDrawLine(dpy, b->w, (c == current) ? gc : igc, 2, 2, button_size - 2, button_size - 2);
		XDrawLine(dpy, b->w, (c == current) ? gc : igc, 2, button_size - 3, button_size - 2, 1);
	}
}

void buttons_update(client *c) {
	if(c->flags & HAS_BUTTONS && c->width <= c->buttons_left_width + c->buttons_right_width) {
		c->flags ^= HAS_BUTTONS;
		XUnmapWindow(dpy, c->button_parent_left);
		XUnmapWindow(dpy, c->button_parent_right);
	} else if(!(c->flags & HAS_BUTTONS) && c->width > c->buttons_left_width + c->buttons_right_width) {
		c->flags |= HAS_BUTTONS;
		XMapWindow(dpy, c->button_parent_left);
		XMapWindow(dpy, c->button_parent_right);
	}
	XMoveWindow(dpy, c->button_parent_right, client_width(c) + border_width - (c->buttons_right_width - 1), border_width - 1);
}

int button_handle_event(XEvent ev) {
	int i, j;
	client *c = NULL;
	button *b = NULL;
	for(i = 0; i < cn; i++)
		for(j = 0; j < clients[i]->nbuttons; j++)
		if(clients[i]->buttons[j].w == ev.xany.window) {
			c = clients[i];
			b = &clients[i]->buttons[j];
		}
	if(!c || !has_child(c->parent, c->window))
		return 0;
	switch(ev.type) {
		case Expose:
			button_draw(c, b);
			return 1;
		case EnterNotify:
			if(button_down) {
				button_down = 2;
				return 1;
			}
			button_current = b;
			button_draw(c, b);
			return 1;
		case LeaveNotify:
			if(button_down == 2)
				button_down = 1;
			button_current = NULL;
			button_draw(c, b);
			return 1;
		case ButtonPress:
			if(ev.xbutton.button == Button1 || ev.xbutton.button == Button3)
				button_down = 1;
			return 1;
		case ButtonRelease:
			if(ev.xbutton.button == Button1 || ev.xbutton.button == Button3) {
				if(button_current == b) {
					if(b->action == B_ICONIFY)
						client_iconify(c);
					if(b->action == B_EXPAND)
						client_expand(c, EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B, 0);
					if(b->action == B_MAXIMIZE)
						client_toggle_state(c, MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B);
					if(b->action == B_CLOSE)
						delete_window(c);
				}
				if(button_down == 2) {
					button_current = b;
					button_draw(c, b);
				}
				button_down = 0;
			}
			return 1;
	}
	return 0;
}

