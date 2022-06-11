#include "previewpage.h"
#include <QDesktopServices>

bool PreviewPage::acceptNavigationRequest(const QUrl& url,
    QWebEnginePage::NavigationType /*type*/,
    bool /*isMainFrame*/)
{
    // Only allow qrc:/index.html.
    if (url.scheme() == QString("qrc"))
        return true;
    QDesktopServices::openUrl(url);
    return false;
}
