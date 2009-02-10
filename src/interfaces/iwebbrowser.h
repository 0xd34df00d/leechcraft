#ifndef IWEBBROWSER_H
#define IWEBBROWSER_H
#include <QString>
#include <QtPlugin>

class IWebWidget
{
public:
	virtual ~IWebWidget () {}

	virtual void Load (const QString&) = 0;
	virtual void SetHtml (const QString&, const QString& = QString ()) = 0;
};

class IWebBrowser
{
public:
	virtual void Open (const QString&) = 0;
	virtual IWebWidget* GetWidget () const = 0;

	virtual ~IWebBrowser () {}
};

Q_DECLARE_INTERFACE (IWebBrowser, "org.Deviant.LeechCraft.IWebBrowser/1.0");

#endif

