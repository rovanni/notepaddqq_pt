#ifndef WEBSOCKETCLIENTWRAPPER_H
#define WEBSOCKETCLIENTWRAPPER_H

#include <QtWebSockets/QWebSocketServer>
#include "websockettransport.h"

class QWebSocketServer;
class WebSocketTransport;
class WebSocketClientWrapper : public QObject
{
    Q_OBJECT

public:
    WebSocketClientWrapper(QWebSocketServer *server, QObject *parent = 0);

signals:
    void clientConnected(WebSocketTransport* client);

private slots:
    void handleNewConnection();

private:
    QWebSocketServer *m_server;
};

#endif // WEBSOCKETCLIENTWRAPPER_H
