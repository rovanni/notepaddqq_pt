#ifndef WEBSOCKETTRANSPORT_H
#define WEBSOCKETTRANSPORT_H
#ifdef USE_QTWEBENGINE

#include <QtWebChannel/QWebChannelAbstractTransport>

class QWebSocket;
class WebSocketTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT

public:
    explicit WebSocketTransport(QWebSocket *socket);
    virtual ~WebSocketTransport();
    void sendMessage(const QJsonObject &message);

private slots:
    void textMessageReceived(const QString &message);

private:
    QWebSocket *m_socket;
};

#endif // USE_QTWEBENGINE
#endif // WEBSOCKETTRANSPORT_H
