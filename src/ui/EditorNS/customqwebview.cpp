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
#ifdef USE_QTWEBENGINE
        static int __uniqueRequestCounter = 0;
        __uniqueRequestCounter++;

        QEventLoop loop;
        int currId = __uniqueRequestCounter;
        connect(this, &CustomQWebView::JavascriptEvaluated, &loop, [currId, &loop](int requestId) {
            if (requestId == __uniqueRequestCounter) {
                loop.quit();
            }
        });

        QVariant result;
        page()->runJavaScript(expr, [&, currId](const QVariant &_result) {
            result = _result;
            emit JavascriptEvaluated(currId);
        });

        loop.exec();

        return result;
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
