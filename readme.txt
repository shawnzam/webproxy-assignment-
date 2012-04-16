Assignment 6: Threaded HTTP Server
Shawn Zamechek

I wrote a threaded proxy server with no cacheing. The proxy can be started by calling make. By default it will use port 8099, but feel free to pass a port to make like so:

make main PORT=xxxxx

I have tested the proxy on my local machine within a web browser and was surprised by how well it worked. Some pages with heavy use of ajax or complicated requests caused the core to dump.  I tested it on eniac by using telnet and sending simple get requests. Like:

get http://zamechek.com/ http/1.1

Within the main function is a socket listener running in a while loop. When a connection arrives main spawns a new thread and passes the request to the handleConnection function. Here the newly spawned thread moves along to handle the request.

The proxy performs rather well on eniac. On more complicated sites, it sometimes returns the HTML, but the thread does not exit. I am not sure what causes this, but the proxy still can respond to new requests because the main thread is still listening for connections.

All activity is logged within a file called server.log. If this file does not exist(first run), the program will create it. Additional runs will just append to the file. 



