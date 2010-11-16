#include "matwm.h"

int (*evh)(XEvent) = NULL;

void handle_event(XEvent ev) {
  client *c = owner(ev.xany.window);
  int i;
#ifdef DEBUG_EVENTS
  if(c) printf("%i (%s): %s\n", ev.xany.window, c->name, event_name(ev));
  else printf("%i: %s\n", ev.xany.window, event_name(ev));
#endif
  if((evh && evh(ev)) || button_handle_event(ev) || ewmh_handle_event(ev))
    return;
  if(c) {
    if(!has_child(c->parent, c->window) && ev.type != DestroyNotify)
      return;
    switch(ev.type) {
      case UnmapNotify:
        if(c->window == ev.xunmap.window) {
          client_deparent(c);
          set_wm_state(c->window, WithdrawnState);
          client_remove(c);
          ewmh_update_clist();
        }
        return;
      case PropertyNotify:
        if(ev.xproperty.atom == XA_WM_NAME) {
          if(c->name != no_title)
            XFree(c->name);
          XFreePixmap(dpy, c->title_pixmap);
          XFetchName(dpy, c->window, &c->name);
          client_update_name(c);
          XClearWindow(dpy, c->title);
          if(evh == wlist_handle_event) {
            XClearWindow(dpy, c->wlist_item);
            wlist_item_draw(c);
          }
        }
        if(ev.xproperty.atom == XA_WM_NORMAL_HINTS)
          get_normal_hints(c);
        return;
      case ClientMessage:
        if(ev.xclient.message_type == xa_wm_change_state && ev.xclient.data.l[0] == IconicState)
          client_iconify(c);
        return;
      case EnterNotify:
        if(c != current)
          client_focus(c);
        return;
      case Expose:
        if(ev.xexpose.count == 0 && evh == wlist_handle_event && c && ev.xexpose.window == c->wlist_item)
          wlist_item_draw(c);
        return;
      case ButtonPress:
        if(buttonaction(ev.xbutton.button) == BA_MOVE)
          drag_start(MOVE, ev.xbutton.button, ev.xbutton.x_root, ev.xbutton.y_root);
        if(buttonaction(ev.xbutton.button) == BA_RESIZE)
          drag_start(RESIZE, ev.xbutton.button, ev.xbutton.x_root, ev.xbutton.y_root);
        return;
      case ButtonRelease:
        if(buttonaction(ev.xbutton.button) == BA_RAISE)
          client_raise(c);
        if(buttonaction(ev.xbutton.button) == BA_LOWER)
          client_lower(c);
        return;
      default:
        if(ev.type == shape_event) {
          set_shape(c);
          return;
        }
    }
  }
  if(current && ev.type == KeyPress)
    switch(keyaction(ev)) {
      case KA_ICONIFY:
        client_iconify(current);
        return;
      case KA_MAXIMISE:
        client_maximise(current);
        return;
      case KA_FULLSCREEN:
        client_fullscreen(current);
        return;
      case KA_EXPAND:
        client_expand(current);
        return;
      case KA_STICKY:
        client_to_desktop(current, (current->desktop == STICKY) ? desktop : STICKY);
        return;
      case KA_TITLE:
        client_toggle_title(current);
        return;
      case KA_BOTTOMLEFT:
        client_move(current, ewmh_strut[0], display_height - (client_height_total(current) + ewmh_strut[3]));
        client_warp(current);
        return;
      case KA_BOTTOMRIGHT:
        client_move(current, display_width - (client_width_total(current) + ewmh_strut[1]), display_height - (client_height_total(current) + ewmh_strut[3]));
        client_warp(current);
        return;
      case KA_TOPRIGHT:
        client_move(current, display_width - (client_width_total(current) - ewmh_strut[1]), ewmh_strut[2]);
        client_warp(current);
        return;
      case KA_TOPLEFT:
        client_move(current, ewmh_strut[0], ewmh_strut[2]);
        client_warp(current);
        return;
      case KA_ONTOP:
        if(!(current->layer == DESKTOP))
          client_set_layer(current, (current->layer == TOP) ? NORMAL : TOP);
        return;
      case KA_BELOW:
        if(!(current->layer == DESKTOP))
          client_set_layer(current, (current->layer == BOTTOM) ? NORMAL : BOTTOM);
        return;
      case KA_CLOSE:
        delete_window(current);
        return;
    }
  switch(ev.type) {
    case MapRequest:
      c = owner(ev.xmaprequest.window);
      if(c) {
        if(c->desktop == ICONS && has_child(c->parent, c->window))
          client_restore(c);
      } else if(has_child(root, ev.xmaprequest.window)) {
        client_add(ev.xmaprequest.window);
        ewmh_update_clist();
      }
      break;
    case DestroyNotify:
      c = owner(ev.xdestroywindow.window);
      if(c && c->window == ev.xdestroywindow.window) {
        client_remove(c);
        ewmh_update_clist();
      }
      break;
    case ConfigureRequest:
      c = owner(ev.xconfigurerequest.window);
      if(c) {
        if(!has_child(c->parent, c->window))
          break;
        if(ev.xconfigurerequest.value_mask & CWX)
          c->x = ev.xconfigurerequest.x - gxo(c, 0);
        if(ev.xconfigurerequest.value_mask & CWY)
          c->y = ev.xconfigurerequest.y - gyo(c, 0);
        if(ev.xconfigurerequest.value_mask & CWWidth)
          c->width = ev.xconfigurerequest.width;
        if(ev.xconfigurerequest.value_mask & CWHeight)
          c->height = ev.xconfigurerequest.height;
        client_update(c);
      } else if(has_child(root, ev.xconfigurerequest.window)) {
        XWindowChanges wc;
        wc.sibling = ev.xconfigurerequest.above;
        wc.stack_mode = ev.xconfigurerequest.detail;
        wc.x = ev.xconfigurerequest.x;
        wc.y = ev.xconfigurerequest.y;
        wc.width = ev.xconfigurerequest.width;
        wc.height = ev.xconfigurerequest.height;
        XConfigureWindow(dpy, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &wc);
      }
      break;
    case MappingNotify:
      if(ev.xmapping.request != MappingPointer) {
        keys_ungrab();
        XRefreshKeyboardMapping(&ev.xmapping);
        keys_update();
      }
      break;
    case KeyPress:
      if(keyaction(ev) == KA_NEXT || keyaction(ev) == KA_PREV)
        wlist_start(ev);
      if(cn && keyaction(ev) == KA_ICONIFY_ALL)
        for(i = 0; i < cn; i++)
          client_iconify(clients[i]);
      if(keyaction(ev) == KA_EXEC)
        spawn(keyarg(ev));
      if(keyaction(ev) == KA_NEXT_DESKTOP && desktop < dc - 1)
        desktop_goto(desktop + 1);
      if(keyaction(ev) == KA_PREV_DESKTOP && desktop > 0)
        desktop_goto(desktop - 1);
      break;
    case ConfigureNotify:
      if(root == ev.xconfigure.window) {
        display_width = ev.xconfigure.width;
        display_height = ev.xconfigure.height;
        if(evh == wlist_handle_event)
          wlist_update();
        ewmh_update_geometry();
        for(i = 0; i < cn; i++)
          client_update_size(clients[i]);
      }
      break;
  }
}
