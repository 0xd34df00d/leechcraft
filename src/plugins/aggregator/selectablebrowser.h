#ifndef SELECTABLEBROWSER_H
#define SELECTABLEBROWSER_H
#include <memory>
#include <QWidget>
#include <QTextEdit>
#include <interfaces/iwebbrowser.h>

class SelectableBrowser : public QWidget
{
	Q_OBJECT

	bool Internal_;
	std::auto_ptr<QTextEdit> InternalBrowser_;
	std::auto_ptr<IWebWidget> ExternalBrowser_;
public:
	SelectableBrowser (QWidget* = 0);
	void Construct (IWebBrowser*);

	void SetHtml (const QString&, const QString& = QString ());
};

#endif

