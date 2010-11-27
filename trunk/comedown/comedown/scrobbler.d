/* scrobbler.d: Main scrobbler class of Comedown 
 *
 * Copyright (c) 2010 Kirn Gill <segin2005@gmail.com>
 *  
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 */

module comedown.scrobbler;
import comedown.types.scrobble;

import std.array;
import std.string;
// import std.xml;
import std.md5;

private import std.stdio;
private import std.string;
private import std.c.stdio;
private import std.c.string;

struct APIPath { 
	string host;
	int port;
	string path;
};

class Scrobbler {

private:
	Scrobble[] scrobbles;
	string user;
	string pass;
	string sessionkey;
	APIPath npPath;
	APIPath submitPath;		

public: 
	this(string user, string pass)
	{
		this.user = user;
		this.pass = pass;
	}

	

}
