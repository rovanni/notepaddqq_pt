#include "include/EditorNS/customqwebview.h"
#include "include/EditorNS/websocketclientwrapper.h"
#include <QtWebChannel/QWebChannel>
#include <QEventLoop>
#include <QtWebSockets/QWebSocketServer>
#include <QWebFrame>

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
        QEventLoop loop;
        connect(this, &CustomQWebView::JavascriptEvaluated, &loop, &QEventLoop::quit);

        QVariant data;
        page()->runJavaScript(expr, [&](const QVariant &result) {
            data = result;
            emit JavascriptEvaluated();
        });

        // FIXME!! It never exits from the event loop
        loop.exec();

        return data;
#else
        return page()->mainFrame()->evaluateJavaScript(expr);
#endif
    }

    void CustomQWebView::connectJavaScriptObject(QString name, QObject *obj)
    {
#ifdef USE_QTWEBENGINE
        // FIXME Tutte le free del caso (settare parent)
        // FIXME Spostare l'inizializzazione del socket nel costruttore!! Qui deve rimanere solo la riga registerObject. Ma anche no: l'inizializzazione deve essere fatta solo quando è documentReady

        // setup the QWebSocketServer
        QWebSocketServer *server = new QWebSocketServer(QStringLiteral("QWebChannel Standalone Example Server"), QWebSocketServer::NonSecureMode);
        if (!server->listen(QHostAddress::LocalHost, 0)) {
            qFatal("Failed to open web socket server.");
            return; // FIXME Return error
        }

        //qDebug() << server.serverUrl().toString();

        // wrap WebSocket clients in QWebChannelAbstractTransport objects
        WebSocketClientWrapper *clientWrapper = new WebSocketClientWrapper(server);

        // setup the channel
        QWebChannel *channel = new QWebChannel();
        QObject::connect(clientWrapper, &WebSocketClientWrapper::clientConnected,
                         channel, &QWebChannel::connectTo);

        // setup the dialog and publish it to the QWebChannel
        channel->registerObject(name, obj);

        evaluateJavaScript("connectSocket('" + jsStringEscape(server->serverUrl().toString()) + "')");
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
            QWebView::keyPressEvent(ev);
        }
    }

}
