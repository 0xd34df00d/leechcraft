#ifndef CORE_H
#define CORE_H
#include <QObject>

class QString;
class QWidget;
class CustomWebView;
class BrowserWidget;

class Core : public QObject
{
	Q_OBJECT

	friend class CustomWebView;
	
	Core ();
public:
	static Core& Instance ();
	void Release ();

	bool IsValidURL (const QString&) const;
	BrowserWidget* NewURL (const QString&);
	CustomWebView* MakeWebView ();
private slots:
	void handleTitleChanged (const QString&);
signals:
	void addNewTab (const QString&, QWidget*);
	void removeTab (QWidget*);
	void changeTabName (QWidget*, const QString&);
};

#endif

