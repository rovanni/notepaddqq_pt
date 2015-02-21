#include "include/EditorNS/customqwebview.h"
#include "include/EditorNS/websocketclientwrapper.h"
#include <QEventLoop>
#include <QBuffer>
#include <QMimeData>

#ifdef USE_QTWEBENGINE
    #include <QtWebChannel/QWebChannel>
    #include <QtWebSockets/QWebSocketServer>
#else
    #include <QWebFrame>
#endif

namespace EditorNS
{

    CustomQWebView::CustomQWebView(QWidget *parent) :
        WEBVIEWNAME(parent)
    {

    }

    void CustomQWebView::wheelEvent(QWheelEvent *ev)
    {
        emit mouseWheel(ev);
        WEBVIEWNAME::wheelEvent(ev);
    }

    QVariant CustomQWebView::evaluateJavaScript(QString expr)
    {
#ifdef USE_QTWEBENGINE

        if (expr.startsWith("UiDriver.connectSocket(")) {
            // Special case: CustomQWebView::JavascriptEvaluated doesn't get fired when connecting socket.
            page()->runJavaScript(expr);
            emit JavascriptEvaluated();
            return QVariant();
        } else {

            QEventLoop loop;
            connect(this, &CustomQWebView::JavascriptEvaluated, &loop, &QEventLoop::quit);

            QByteArray byteArray;

            page()->runJavaScript(expr, [&](const QVariant &result) {
                // Serialize result to byteArray
                QBuffer writeBuffer(&byteArray);
                writeBuffer.open(QIODevice::WriteOnly);
                QDataStream out(&writeBuffer);
                out << result;
                writeBuffer.close();

                emit JavascriptEvaluated();
            });

            loop.exec();

            // Deserialize result
            QBuffer readBuffer(&byteArray);
            readBuffer.open(QIODevice::ReadOnly);
            QDataStream in(&readBuffer);
            QVariant data;
            in >> data;

            return data;
        }
#else
        return page()->mainFrame()->evaluateJavaScript(expr);
#endif
    }

    void CustomQWebView::connectJavaScriptObject(QString name, QObject *obj)
    {
#ifdef USE_QTWEBENGINE
        // FIXME Spostare l'inizializzazione del socket nel costruttore!! Qui deve rimanere solo la riga registerObject. Ma anche no: l'inizializzazione deve essere fatta solo quando è documentReady

        // setup the QWebSocketServer
        QWebSocketServer *server = new QWebSocketServer(QStringLiteral("QWebChannel Notepadqq Server"), QWebSocketServer::NonSecureMode, this);
        if (!server->listen(QHostAddress::LocalHost, 0)) {
            qFatal("Failed to open web socket server.");
            return; // FIXME Return error
        }

        // wrap WebSocket clients in QWebChannelAbstractTransport objects
        WebSocketClientWrapper *clientWrapper = new WebSocketClientWrapper(server, this);

        // setup the channel
        QWebChannel *channel = new QWebChannel(this);
        QObject::connect(clientWrapper, &WebSocketClientWrapper::clientConnected,
                         channel, &QWebChannel::connectTo);

        // setup the dialog and publish it to the QWebChannel
        channel->registerObject(name, obj);

        evaluateJavaScript("UiDriver.connectSocket('" + jsStringEscape(server->serverUrl().toString()) + "')");
#else
        page()->mainFrame()->
                addToJavaScriptWindowObject(name, obj);
#endif
    }

    QString CustomQWebView::jsStringEscape(QString str) const {
        return str.replace("\\", "\\\\")
                .replace("'", "\\'")
                .replace("\"", "\\\"")
                .replace("\n", "\\n")
                .replace("\r", "\\r")
                .replace("\t", "\\t")
                .replace("\b", "\\b");
    }

    void CustomQWebView::keyPressEvent(QKeyEvent *ev)
    {
        switch (ev->key()) {
        case Qt::Key_Insert:
            ev->ignore();
            break;
        default:
            WEBVIEWNAME::keyPressEvent(ev);
        }
    }

    void CustomQWebView::dropEvent(QDropEvent *ev)
    {
        if (ev->mimeData()->hasUrls()) {
            ev->ignore();
            emit urlsDropped(ev->mimeData()->urls());
        } else {
            WEBVIEWNAME::dropEvent(ev);
        }
    }

}
