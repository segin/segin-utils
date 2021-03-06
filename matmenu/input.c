#include "matmenu.h"

unsigned int *mod_ignore = NULL;
XModifierKeymap *modmap;
keybind *keys = NULL;
int keyn = 0, nmod_ignore  = 0;

void key_bind(char *str) {
  keybind k;
  k.sym = str_key(&str, &k.mask);
  if(!str)
    return;
  k.action = str_keyaction(eat(&str, " \t"));
  if(str) {
    while(*str == ' ' || *str == '\t')
      str++;
    k.arg = (char *) malloc(strlen(str) + 1);
    strncpy(k.arg, str, strlen(str) + 1);
  } else k.arg = NULL;
  keys = (keybind *) realloc((void *) keys, (keyn + 1) * sizeof(keybind));
  if(!keys)
    error();
  keys[keyn] = k;
  keyn++;
}

void keys_update(void) {
  int i;
  modmap = XGetModifierMapping(dpy);
  for(i = 0; i < keyn; i++)
    keys[i].code = XKeysymToKeycode(dpy, keys[i].sym);
}

void keys_ungrab(void) {
  int i;
  for(i = 0; i < keyn; i++)
    key_ungrab(keys[i]);
}

void keys_free(void) {
  int i;
  for(i = 0; i < keyn; i++)
    if(keys[i].arg)
      free((void *) keys[i].arg);
  free((void *) keys);
  keys = NULL;
  keyn = 0;
}

void key_grab(keybind key) {
  int i;
  XGrabKey(dpy, key.code, key.mask, root, True, GrabModeAsync, GrabModeAsync);
  for(i = 0; i < nmod_ignore; i++)
    XGrabKey(dpy, key.code, key.mask | mod_ignore[i], root, True, GrabModeAsync, GrabModeAsync);
}

void key_ungrab(keybind key) {
  int i;
  XUngrabKey(dpy, key.code, key.mask, root);
  for(i = 0; i < nmod_ignore; i++)
    XUngrabKey(dpy, key.code, key.mask | mod_ignore[i], root);
}

int keyaction(XEvent ev) {
  int i;
  for(i = 0; i < keyn; i++)
    if(keys[i].code == ev.xkey.keycode && cmpmodmask(keys[i].mask, ev.xkey.state))
      return keys[i].action;
  return KA_NONE;
}

char *keyarg(XEvent ev) {
  int i;
  for(i = 0; i < keyn; i++)
    if(keys[i].code == ev.xkey.keycode && cmpmodmask(keys[i].mask, ev.xkey.state) && keys[i].arg)
      return keys[i].arg;
  return "";
}

int key_to_mask(KeyCode key) {
  int i;
  for(i = 0; i < 8 * modmap->max_keypermod; i++)
    if(modmap->modifiermap[i] == key)
      return 1 << (i / modmap->max_keypermod);
  return 0;
}

int cmpmodmask(int m1, int m2) {
  int i;
  for(i = 0; i < nmod_ignore; i++)
    m2 ^= m2 & mod_ignore[i];
  return m1 == m2;
}

