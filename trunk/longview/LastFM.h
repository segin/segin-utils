
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