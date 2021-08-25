#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QWidget>

class AppWindow : public QWidget
{
public:
    explicit AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

#ifdef Q_OS_WIN
    /**
     * @brief 删除当前自定义标题栏（如果存在），并添加新的自定义标题栏，其宽度会根据 AppWindow 的宽度被自动调整
     * @param titleBar 自定义标题栏
     * @param followSystemCaption 如果值为 true ，采用系统默认标题栏响应鼠标操作， titleBar 将不能响应鼠标事件，但其子控件不受影响
     * 此函数仅在 Windows 下有效🙁
     */
    void setCustomCaption(QWidget* caption, bool followSystemCaption = true, int height = 20);
    QWidget* caption();
#endif

    /**
     * @brief setContent 删除当前内容窗口，并设置新的内容窗口，它会被放置在标题栏下方并占据下面所有空间，尺寸会跟随 AppWindow 被调整
     * @param content 内容窗口
     */
    void setContent(QWidget* content);
    QWidget* content();

protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent*) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    class AppWindowPriv* priv;
};

#endif // APPWINDOW_H
