/*

	         Synchronization with QtWebEngine

  [JS]               [QtWebEngine]                [C++]
   |                       |                        |
   |                  pageLoaded()  --------------->|
   |                       |                   Create socket and bind cpp_ui_driver with QWebChannel
   |<----------------------|-----------------  Call js function connectSocket()
connectSocket()            |                        |
J_EVT_READY ---------------|----------------------->|
Editor is ready.           |                   Editor is ready.
   |                       |                        |
   -                       -                        -


	         Synchronization with QtWebKit

  [JS]                 [QtWebKit]                 [C++]
   |                       |                        |
   |               jsObjectCleared()  ------------->|
   |                       |                   Bind cpp_ui_driver
J_EVT_READY ---------------|----------------------->|
Editor is ready.           |                   Editor is ready.
   |                       |                        |
   -                       -                        -
   
*/

// FIXME Tutta questa roba deve stare nell'UI-DRIVER!!
var socket;
var cpp_ui_driver;

function connectSocket(url) {
	socket = new WebSocket(url);
	socket.onclose = function()
	{
		console.error("web channel closed");
	};
	socket.onerror = function(error)
	{
		console.error("web channel error: " + error);
		console.error(error);
	};
	socket.onopen = function()
	{
		console.log("WebSocket connected, setting up QWebChannel.");
		console.error("AAAAAAAAAAAAA" + url);
		new QWebChannel(socket, function(channel) {
			cpp_ui_driver = channel.objects.cpp_ui_driver;
			cpp_ui_driver.receiveMessage("J_EVT_READY", null); // FIXME Use UiDriver..
		});
	}
	return true;
}

console.error("REGISTERED!!!!!");

var UiDriver = new function() {
    var handlers = [];

    this.sendMessage = function(msg, data) {
		console.error("*********** Wanted to send " + msg);
	    //cpp_ui_driver.messageReceived(msg, data); // FIXME Fare coda dei messaggi in attesa?
    }

    this.registerEventHandler = function(msg, handler) {
        if (handlers[msg] === undefined)
		    handlers[msg] = [];

	    handlers[msg].push(handler);
    }

    this.messageReceived = function(msg) {
		console.error("*********** Received " + msg);
        var data = cpp_ui_driver.getMsgData();

        // Only one of the handlers (the last that gets
        // called) can return a value. So, to each handler
        // we provide the previous handler's return value.
        var prevReturn = undefined;

        if (handlers[msg] !== undefined) {
            handlers[msg].forEach(function(handler) {
                prevReturn = handler(msg, data, prevReturn);
            });
        }

        return prevReturn;
    }
}
