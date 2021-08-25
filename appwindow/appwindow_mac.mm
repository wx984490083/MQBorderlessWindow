#include "appwindow.h"

class AppWindowPriv
{
public:
    AppWindow* owner;
    QWidget* content = nullptr;

    void doLayout()
    {
        if (content)
        {
            content->setGeometry(owner->rect());
        }
    }
};


AppWindow::AppWindow(QWidget *parent) : QWidget(parent), priv(new AppWindowPriv)
{
    priv->owner = this;
}

AppWindow::~AppWindow()
{
    delete priv;
}

void AppWindow::setContent(QWidget* content)
{
    if (priv->content)
    {
        priv->content->hide();
        priv->content->deleteLater();
    }
    priv->content = content;
    if (priv->content)
    {
        priv->content->setParent(this);
        priv->content->show();
    }
    priv->doLayout();
}

QWidget* AppWindow::content()
{
    return priv->content;
}

void AppWindow::resizeEvent(QResizeEvent *)
{
    priv->doLayout();
}

bool AppWindow::event(QEvent *event)
{
    return QWidget::event(event);
}

bool AppWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    return QWidget::nativeEvent(eventType, message, result);
}
