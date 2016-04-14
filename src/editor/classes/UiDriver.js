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

    var _this = this;

    function usingQtWebChannel() {
        return (typeof cpp_ui_driver === 'undefined')
    }

    document.addEventListener("DOMContentLoaded", function () {
        new QWebChannel(qt.webChannelTransport, function (_channel) {
            channel = _channel;
            console.error("DOMContentLoaded, new QWebChannel")
            // Send the messages in the queue
            for (var i = 0; i < msgQueue.length; i++) {
                _this.sendMessage(msgQueue[i][0], msgQueue[i][1]);
            }
            msgQueue = [];
            console.error("sent old msgs" + channel.objects.cpp_ui_driver)
        });
    });

    this.sendMessage = function(msg, data) {
        if (usingQtWebChannel()) {
            // QtWebEngine

            if (channel === undefined) {
                // Communication with the C++ part is not yet completed.
                msgQueue.push([msg, data]);
                return;
            }
            console.error("Sending: " + msg + " -- " + JSON.stringify( data ));
            if (data !== null && data !== undefined) {
                channel.objects.cpp_ui_driver.receiveMessage(msg, data, function(ret) { console.error(msg + " sent to c++ (async)") });
            } else {
                channel.objects.cpp_ui_driver.receiveMessage(msg, "", function(ret) { console.error(msg + " sent to c++ (async)") });
            }
            console.error("Delivered: " + msg);
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

    this.messageReceived = function(msg, data) {
        
        //var data;
        if (true || usingQtWebChannel()) {
            console.error("Received: " + msg + "  data: " + data)
            //data = channel.objects.cpp_ui_driver.getMsgData(); // FIXME Must be async!!!
            //channel.objects.cpp_ui_driver.getMsgData(function(data) {   });
        } else {
            data = cpp_ui_driver.getMsgData();
        }

        // Only one of the handlers (the last that gets
        // called) can return a value. So, to each handler
        // we provide the previous handler's return value.
        var prevReturn = undefined;

        if (handlers[msg] !== undefined) {
            handlers[msg].forEach(function(handler) {
                prevReturn = handler(msg, data, prevReturn);
            });
        }

        console.error("Return value for " + msg + ": " + prevReturn);
        return prevReturn;
    }
}
