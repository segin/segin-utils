/*
 * Copyright (c) 2011, Kirn Gill <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __LastFM__
#define __LastFM__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
    class LastFM;
}

class LastFM : public ::java::lang::Object
{

public:
  LastFM();
  static void main(JArray< ::java::lang::String * > *);
private:
  ::java::lang::String * authKey(::java::lang::String *);
  ::java::lang::String * getSessionKey(::java::lang::String *);
  ::java::lang::String * doAskterisks(::java::lang::String *);
  void readConfig(::java::lang::String *);
public: // actually package-private
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) sessionkey;
  ::java::lang::String * username;
  ::java::lang::String * password;
public:
  static ::java::lang::Class class$;
};

#endif // __LastFM__
