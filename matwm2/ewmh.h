enum {
  NET_SUPPORTED,
  NET_SUPPORTING_WM_CHECK,
  NET_WM_NAME,
  NET_NUMBER_OF_DESKTOPS,
  NET_DESKTOP_GEOMETRY,
  NET_DESKTOP_VIEWPORT,
  NET_WORKAREA,
  NET_WM_STRUT,
  NET_WM_STRUT_PARTIAL,
  NET_CURRENT_DESKTOP,
  NET_DESKTOP_NAMES,
  NET_WM_DESKTOP,
  NET_CLIENT_LIST,
  NET_CLIENT_LIST_STACKING,
  NET_ACTIVE_WINDOW,
  NET_WM_STATE,
  NET_WM_STATE_FULLSCREEN,
  NET_WM_STATE_MAXIMIZED_HORZ,
  NET_WM_STATE_MAXIMIZED_VERT,
  NET_CLOSE_WINDOW,
  NET_WM_WINDOW_TYPE,
  NET_WM_WINDOW_TYPE_DESKTOP,
  NET_WM_WINDOW_TYPE_DOCK,
  NET_WM_ALLOWED_ACTIONS,
  NET_WM_ACTION_MINIMIZE,
  NET_WM_ACTION_CLOSE,
  NET_WM_ACTION_MAXIMIZE_HORZ,
  NET_WM_ACTION_MAXIMIZE_VERT,
  NET_WM_ACTION_FULLSCREEN,
  NET_WM_ACTION_MOVE,
  NET_WM_ACTION_RESIZE,
  NET_WM_MOVERESIZE,
  EWMH_ATOM_COUNT
};

#define NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT      4
#define NET_WM_MOVERESIZE_MOVE                  8
#define NET_WM_MOVERESIZE_SIZE_KEYBOARD         9
#define NET_WM_MOVERESIZE_MOVE_KEYBOARD         10

#define NET_WM_STATE_REMOVE                     0
#define NET_WM_STATE_ADD                        1
#define NET_WM_STATE_TOGGLE                     2

