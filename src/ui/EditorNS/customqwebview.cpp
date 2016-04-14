#include "include/EditorNS/customqwebview.h"
#include <QEventLoop>
#include <QBuffer>
#include <QMimeData>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

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
        QWebChannel *channel = new QWebChannel(page());

        // set the web channel to be used by the page
        // see http://doc.qt.io/qt-5/qwebenginepage.html#setWebChannel
        page()->setWebChannel(channel);
    }

    void CustomQWebView::wheelEvent(QWheelEvent *ev)
    {
        emit mouseWheel(ev);
        WEBVIEWNAME::wheelEvent(ev);
    }

    QVariant CustomQWebView::evaluateJavaScript(const QString &expr)
    {
        static int cnt = 0;
        cnt++;
        qDebug() << QString::number(cnt) + " <------------- COUNT";
#ifdef USE_QTWEBENGINE

        //if (expr.startsWith("UiDriver.connectSocket(")) {
            // Special case: CustomQWebView::JavascriptEvaluated doesn't get fired when connecting socket.
        //    page()->runJavaScript(expr);
        //    emit JavascriptEvaluated();
        //    return QVariant();
        //} else {

            QEventLoop loop;
            int currId = cnt;
            connect(this, &CustomQWebView::JavascriptEvaluated, &loop, [currId, &loop](int requestId) {
                if (requestId == cnt) {
                    loop.quit();
                }
            });
               // &QEventLoop::quit);

            //QByteArray *byteArray = new QByteArray();
            //QJsonDocument doc;
            QVariant _result;
            //QString json_result
            page()->runJavaScript(expr, [&, currId](const QVariant &result) {
                // Serialize result to byteArray

                /*qDebug() << "      ~~ follows: ";
                qDebug() << expr;
                qDebug() << "      ~~ result: ";
                qDebug() << result;*/

                /*QString msgData = "null";
                QJsonValue jsonData = QJsonValue::fromVariant(result);
                if (jsonData.isArray()) {
                    msgData = QJsonDocument(jsonData.toArray()).toJson();
                } else if (jsonData.isBool()) {
                    msgData = jsonData.toBool() ? "true" : "false";
                } else if (jsonData.isDouble()) {
                    msgData = QString::number(jsonData.toDouble());
                } else if (jsonData.isObject()) {
                    msgData = QString(QJsonDocument(jsonData.toObject()).toJson());
                } else if (jsonData.isString()) {
                    msgData = "'" + jsStringEscape(jsonData.toString()) + "'";
                } else if (jsonData.isUndefined()) {
                    msgData = "undefined";
                }
                qDebug() << result;
                qDebug() << msgData;

                doc = QJsonDocument::fromJson(msgData.toUtf8());*/

                _result = result;

                //QDataStream dsw(byteArray,QIODevice::WriteOnly);
                //dsw << result;


                /*QBuffer writeBuffer(byteArray);
                writeBuffer.open(QIODevice::WriteOnly);
                QDataStream out(&writeBuffer);
                out << result;
                writeBuffer.close();*/

                emit JavascriptEvaluated(currId);
            });

            loop.exec();

            // Deserialize result
            /*QBuffer readBuffer(byteArray);
            readBuffer.open(QIODevice::ReadOnly);
            QDataStream in(&readBuffer);
            QVariant data;
            in >> data;*/
            //QDataStream dsr(byteArray,QIODevice::ReadOnly);
            //dsr>>data;
            //QVariant result = doc.toVariant();
            QVariant result = _result;
            qDebug() << "      ~~ follows: ";
            qDebug() << expr;
            qDebug() << "      ~~ result: ";
            qDebug() << result;

            //delete byteArray;
            return result;
        //}
#else
        return page()->mainFrame()->evaluateJavaScript(expr);
#endif
    }

    void CustomQWebView::connectJavaScriptObject(QString name, QObject *obj)
    {
#ifdef USE_QTWEBENGINE
        // setup the dialog and publish it to the QWebChannel
        page()->webChannel()->registerObject(name, obj);
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
