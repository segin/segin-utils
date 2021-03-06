/* test.d - main() code to test scrobbleD. 
 * Copyright (c) 2011, Kirn Gill <segin2005@gmail.com>
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

import std.md5;
import std.stdio;
import std.uri;
import std.string;
import std.file;

import comedown.scrobbler;

void main(string[] args) 
{
	try { 
		string config = cast(string) std.file.read("lastfm_config.txt");
		auto lines = std.string.splitlines(config);
		if(lines.length < 2) { 
			writefln("Config file contains too few lines!");
			return;
		}
		
		for(int i = 0; i < lines.length ; i++) { 
			lines[i] = lines[i].strip();
		}

		auto scrobbler = new Scrobbler("tst", "1.0", lines[0], lines[1]);

		
			
	} 

	catch(FileException e) {
		writefln("Cannot open config file: " ~ e.toString());
		return;	
	}

	writefln("Hello World!");
	//auto encoded = encodeComponent(orig);
	//writefln("Orginal: %s, Encoded: %s", orig, encoded);
}
