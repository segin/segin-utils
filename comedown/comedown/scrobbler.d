/* scrobbler.d: Main scrobbler class of Comedown 
 *
 * Copyright (c) 2010 Kirn Gill <segin2005@gmail.com>
 * 
 * Portions based on HTMLget example by Christopher E. Miller
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
import std.xml;
import std.md5;
import std.uri;
import std.conv;
import std.date;
import std.stdio;
import std.string;
import std.socket;
import std.socketstream;
import std.c.stdio;
import std.c.string;

struct APIPath {

public:
	string host;
	ushort port;
	string path;

	this(string host, ushort port, string path)
	{
		this.host = host;
		this.port = port;
		this.path = path;
	}
};

class Scrobbler {

private:
	Scrobble[] scrobbles;
	string appid;
	string appver;
	string user;
	string pass;
	string status;
	string sessionkey;
	string server;
	APIPath npPath;
	APIPath submitPath;		

	void init() 
	{
		sessionkey = getSessionKey();
	}

	/* This function also sets the path for "now playing" and 
	 * "scrobble" submits. 
	 */
	string getSessionKey()
	{
		writefln("comedown.scrobbler: Last.fm username: " ~ 
			user ~ ", password: " ~ "*".repeat(pass.length));
		auto curtime = to!string(getUTCtime() / ticksPerSecond);
		auto authkey = MD5Str(MD5Str(pass) ~ curtime); 
		writefln("curtime: " ~ curtime ~ ", authkey: " ~ authkey);
		auto a = APIPath(, 80, "/");
		auto getdata = [ 
				"hs" : "true", 
				"p"  : "1.2.1",
				"c"  : appid,
				"v"  : appver,
				"u"  : user,
				"t"  : curtime,
				"a"  : authkey
			    ];
		auto response = submitData(a, getdata, "GET");
		if(response.length == 0) {
			return "";
		} else {
			;
		}
		return "";
	}

	string[] submitData(APIPath path, string[string] request, string requestType)
	{
		string[] returnData;
		auto requestSerial = requestType == "GET" ? "?" : "";
		foreach(requestKey, requestValue; request) {
			requestSerial ~= encodeComponent(requestKey) ~ "=" ~
				encodeComponent(requestValue) ~ "&";
		};

		requestSerial.popBack;

		writefln(requestSerial);
		
		try { 
			auto sock = new TcpSocket(new InternetAddress
				(path.host, cast(ushort) path.port));
			auto ss = new SocketStream(sock);
		} 

		catch (Exception e) {
			std.stdio.stderr.writefln("Exception: " ~ e.toString());
		}
		
		return [];
	}

	string MD5Str(string str)
	{	
		ubyte[16u] digest;
		MD5_CTX context;
		context.start();
		context.update(str);
		context.finish(digest);
		return digestToString(digest);
	}

public: 
	this(string server, string appid, string appver, string user, string pass)
	{
		this.server = server;
		this.appid = appid;
		this.user = user;
		this.pass = pass;
		init();
	}

	void setUser(string user, string pass)
	{
		sessionkey = "";
		this.user = user;
		this.pass = pass;
	}

}