#include "defcfg.h"

char *def_cfg[DEF_CFG_LINES] = { /* we split up the default configuration into a lot of separate strings (one for each line) because ISO C90 forbids strings longer then 509 chars */
	"background            darkred",
	"inactive_background   dimgray",
	"foreground            lightgray",
	"inactive_foreground   darkgray",
	"border_color          lightgray",
	"inactive_border_color darkgray",
	"font                  fixed",
	"border_spacing        2",
	"border_width          1",
	"title_spacing         2",
	"button_spacing        2",
	"buttons_left          sticky ontop",
	"buttons_right         iconify expand maximize close",
	"center_title          false",
	"center_wlist_items		false",
	"desktops              4",
	"snap                  5",
	"taskbar_ontop         false",
	"focus_new             false",
	"fullscreen_stacking   ontop",
	"click_focus           false",
	"click_raise           false",
	"map_center            false",
	"button1               move",
	"button2               none",
	"button3               resize",
	"button4               raise",
	"button5               lower",
	"doubleclick_time      300",
	"double1               maximize",
	"double2               none",
	"double3               none",
	"double4               none",
	"double5               none",
	"mouse_modifier        mod1",
	"no_snap_modifier      shift",
	"ignore_modifier       mod2 lock",
	"key                   mod1 Tab                next",
	"key                   mod1 shift Tab          prev",
	"key                   control mod1 s          iconify",
	"key                   control mod1 x          maximize",
	"key                   control mod1 h          maximize h",
	"key                   control mod1 v          maximize v",
	"key                   control mod1 f          fullscreen",
	"key                   control mod1 e          expand",
	"key                   control mod1 r          expand a",
	"key                   control mod1 z          expand h",
	"key                   control mod1 j          expand v",
	"key                   control mod1 q          close",
	"key                   control mod1 d          sticky",
	"key                   control mod1 t          title",
	"key                   control mod1 y          to_border tl",
	"key                   control mod1 u          to_border tr",
	"key                   control mod1 b          to_border bl",
	"key                   control mod1 n          to_border br",
	"key                   control mod1 o          ontop",
	"key                   control mod1 p          below",
	"key                   control mod1 a          iconify_all",
	"key                   control mod1 Right      next_desktop",
	"key                   control mod1 Left       prev_desktop",
	"key                   control mod1 Prior      raise",
	"key                   control mod1 Next       lower",
	"key                   control mod1 Return     exec xterm",
	"key                   mod4 s                  iconify",
	"key                   mod4 x                  maximize",
	"key                   mod4 h                  maximize h",
	"key                   mod4 v                  maximize v",
	"key                   mod4 f                  fullscreen",
	"key                   mod4 e                  expand",
	"key                   mod4 r                  expand a",
	"key                   mod4 z                  expand h",
	"key                   mod4 j                  expand v",
	"key                   mod4 q                  close",
	"key                   mod4 d                  sticky",
	"key                   mod4 t                  title",
	"key                   mod4 y                  to_border tl",
	"key                   mod4 u                  to_border tr",
	"key                   mod4 b                  to_border bl",
	"key                   mod4 n                  to_border br",
	"key                   mod4 o                  ontop",
	"key                   mod4 p                  below",
	"key                   mod4 a                  iconify_all",
	"key                   mod4 Right              next_desktop",
	"key                   mod4 Left               prev_desktop",
	"key                   mod4 Prior              raise",
	"key                   mod4 Next               lower",
};
