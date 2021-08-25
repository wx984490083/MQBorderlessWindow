// Qt Borderless Window (Windows)
// Thanks https://github.com/melak47/BorderlessWindow



#include "appwindow.h"
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <QEvent>

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "User32.lib")

namespace
{

// we cannot just use WS_POPUP style
// WS_THICKFRAME: without this the window cannot be resized and so aero snap, de-maximizing and minimizing won't work
// WS_SYSMENU: enables the context menu with the move, close, maximize, minize... commands (shift + right-click on the task bar item)
// WSpriv->caption: enables aero minimize animation/transition
// WS_MAXIMIZEBOX, WS_MINIMIZEBOX: enable minimize/maximize
enum class Style : DWORD {
    windowed         = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
    aero_borderless  = WS_POPUP            | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
    basic_borderless = WS_POPUP            | WS_THICKFRAME              | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX
};

auto maximized(HWND hwnd) -> bool {
    WINDOWPLACEMENT placement;
    if (!::GetWindowPlacement(hwnd, &placement)) {
          return false;
    }

    return placement.showCmd == SW_MAXIMIZE;
}

/* Adjust client rect to not spill over monitor edges when maximized.
 * rect(in/out): in: proposed window rect, out: calculated client rect
 * Does nothing if the window is not maximized.
 */
auto adjust_maximized_client_rect(HWND window, RECT& rect) -> void {
    if (!maximized(window)) {
        return;
    }

    auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
    if (!monitor) {
        return;
    }

    MONITORINFO monitor_info{};
    monitor_info.cbSize = sizeof(monitor_info);
    if (!::GetMonitorInfoW(monitor, &monitor_info)) {
        return;
    }

    // when maximized, make the client area fill just the monitor (without task bar) rect,
    // not the whole window rect which extends beyond the monitor.
    rect = monitor_info.rcWork;
}


LRESULT caption_test(const POINT& cursor, QWidget* titleBar)
{
    if (!titleBar)
    {
        return HTCLIENT;
    }
    QPoint posInTitleBar = titleBar->mapFromGlobal({cursor.x, cursor.y});
    auto titleBarRect = titleBar->rect();
    if (titleBarRect.contains(posInTitleBar))
    {
        auto child = titleBar->childAt(posInTitleBar);
        return !!child ? HTCLIENT : HTCAPTION;
    }
    return HTCLIENT;
}

auto hit_test(HWND hwnd, POINT cursor, QWidget* titleBar, bool followSystemCaption) -> LRESULT {
    // identify borders and corners to allow resizing the window.
    // Note: On Windows 10, windows behave differently and
    // allow resizing outside the visible window frame.
    // This implementation does not replicate that behavior.
    const POINT border{
        ::GetSystemMetrics(SM_CXFRAME),
        ::GetSystemMetrics(SM_CYFRAME),
    };
    RECT window;
    if (!::GetWindowRect(hwnd, &window)) {
        return HTNOWHERE;
    }

    enum region_mask {
        client = 0b0000,
        left   = 0b0001,
        right  = 0b0010,
        top    = 0b0100,
        bottom = 0b1000,
    };

    const auto result =
        left    * (cursor.x <  (window.left   + border.x)) |
        right   * (cursor.x >= (window.right  - border.x)) |
        top     * (cursor.y <  (window.top    + border.y)) |
        bottom  * (cursor.y >= (window.bottom - border.y));

    switch (result) {
    case left          : return HTLEFT       ;
    case right         : return HTRIGHT      ;
    case top           : return HTTOP        ;
    case bottom        : return HTBOTTOM     ;
    case top | left    : return HTTOPLEFT    ;
    case top | right   : return HTTOPRIGHT   ;
    case bottom | left : return HTBOTTOMLEFT ;
    case bottom | right: return HTBOTTOMRIGHT;
    case client        : return followSystemCaption ? caption_test(cursor, titleBar) : HTCLIENT;
    default            : return HTNOWHERE    ;
    }
}

auto composition_enabled() -> bool {
    BOOL composition_enabled = FALSE;
    bool success = ::DwmIsCompositionEnabled(&composition_enabled) == S_OK;
    return composition_enabled && success;
}

auto set_borderless_shadow(HWND handle, bool enabled) -> void {
    auto composition = composition_enabled();
    auto style = static_cast<LONG>(composition ? Style::aero_borderless : Style::basic_borderless);
    auto oldStyle = ::GetWindowLongPtr(handle, GWL_STYLE);
    style |= oldStyle;
    if (oldStyle != style)
    {
        ::SetWindowLongPtr(handle, GWL_STYLE, style);
    }
    if (composition) {
        static const MARGINS shadow_state[2]{ { 0,0,0,0 },{ 1,1,1,1 } };
        ::DwmExtendFrameIntoClientArea(handle, &shadow_state[enabled]);
    }
}

}

class AppWindowPriv
{
public:
    AppWindow* owner;
    QWidget* caption = nullptr;
    QWidget* content = nullptr;
    bool followSystemCaption = true;
    int captionHeight = 20;

    void doLayout()
    {
        auto ch = !!caption ? this->captionHeight : 0;
        if (caption)
        {
            caption->setGeometry(0, 0, owner->width(), ch);
        }
        if (content)
        {
            content->setGeometry(0, ch, owner->width(), owner->height() - ch);
        }
    }
};


AppWindow::AppWindow(QWidget *parent) : QWidget(parent),  priv(new AppWindowPriv)
{
    priv->owner = this;
    setCustomCaption(new QWidget());
}

AppWindow::~AppWindow()
{
    delete priv;
}

void AppWindow::setCustomCaption(QWidget *caption, bool followSystemCaption, int height)
{
    if (priv->caption)
    {
        priv->caption->hide();
        priv->caption->deleteLater();
    }
    priv->caption = caption;
    priv->followSystemCaption = followSystemCaption;
    priv->captionHeight = height;
    if (priv->caption)
    {
        priv->caption->setParent(this);
        priv->caption->show();
    }
    priv->doLayout();

    auto hwnd = (HWND)winId();
    ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
}

void AppWindow::setContent(QWidget *content)
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

QWidget* AppWindow::caption()
{
    return priv->caption;
}

bool AppWindow::event(QEvent *event)
{
    if (event->type() == QEvent::Show)
    {
        if (auto hwnd = (HWND)winId())
        {
            set_borderless_shadow(hwnd, true);
        }
    }
    return QWidget::event(event);
}

void AppWindow::resizeEvent(QResizeEvent *)
{
    priv->doLayout();
}

bool AppWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    auto msg = (MSG*)message;
    switch (msg->message)
    {
    case WM_NCCALCSIZE:
    {
        if (msg->wParam == TRUE)
        {
            auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
            adjust_maximized_client_rect(msg->hwnd, params.rgrc[0]);
            *result = 0;
            return true;
        }
    } break;
    case WM_NCHITTEST:
    {
        // When we have no border or title bar, we need to perform our
        // own hit testing to allow resizing and moving.
        *result = hit_test(msg->hwnd, POINT{
            GET_X_LPARAM(msg->lParam),
            GET_Y_LPARAM(msg->lParam)
        }, priv->caption, priv->followSystemCaption);
        return true;
    } break;
    case WM_NCACTIVATE:
    {
        if (!composition_enabled())
        {
            // Prevents window frame reappearing on window activation
            // in "basic" theme, where no aero shadow is present.
            *result = 1;
            return true;
        }
    } break;
    }
    return QWidget::nativeEvent(eventType, message, result);
}



