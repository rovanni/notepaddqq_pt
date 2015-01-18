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

var UiDriver = new function() {
    var handlers = [];

    var socket;
    var channel;

    var msgQueue = [];

    function usingQtWebChannel() {
        return (typeof cpp_ui_driver === 'undefined')
    }

    this.connectSocket = function(url) {
        var _this = this;

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
            new QWebChannel(socket, function(_channel) {
                channel = _channel;

                // Send the messages in the queue
                for (var i = 0; i < msgQueue.length; i++) {
                    _this.sendMessage(msgQueue[i][0], msgQueue[i][1]);
                }
            });
        }
    }

    this.sendMessage = function(msg, data) {
        if (usingQtWebChannel()) {
            // QtWebEngine

            if (channel === undefined) {
                // Communication with the C++ part is not yet completed.
                msgQueue.push([msg, data]);
                return;
            }

            if (data !== null && data !== undefined) {
                channel.objects.cpp_ui_driver.receiveMessage(msg, data);
            } else {
                channel.objects.cpp_ui_driver.receiveMessage(msg, "");
            }

        } else {
            // QtWebKit
            cpp_ui_driver.receiveMessage(msg, data);
        }
    }

    this.registerEventHandler = function(msg, handler) {
        if (handlers[msg] === undefined)
            handlers[msg] = [];

        handlers[msg].push(handler);
    }

    this.messageReceived = function(msg) {
        var data;
        if (usingQtWebChannel())
            data = channel.objects.cpp_ui_driver.getMsgData();
        else
            data = cpp_ui_driver.getMsgData();

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
