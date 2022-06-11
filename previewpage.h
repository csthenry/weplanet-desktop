#pragma once
#pragma execution_character_set("utf-8")

#ifndef PREVIEWPAGE_H
#define PREVIEWPAGE_H

#include <QtWebEngineWidgets/qwebenginepage.h>

class PreviewPage  : public QWebEnginePage
{
	Q_OBJECT
public:
	explicit PreviewPage(QObject* parent = nullptr) : QWebEnginePage(parent) {}

protected:
	bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame);
};

#endif // PREVIEWPAGE_H