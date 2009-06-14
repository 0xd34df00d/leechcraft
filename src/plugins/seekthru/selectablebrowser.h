#ifndef PLUGINS_SEEKTHRU_SELECTABLEBROWSER_H
#define PLUGINS_SEEKTHRU_SELECTABLEBROWSER_H
#include <QWidget>
#include <memory>
#include <QTextBrowser>
#include <interfaces/iwebbrowser.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class SelectableBrowser : public QWidget
			{
				Q_OBJECT

				bool Internal_;
				std::auto_ptr<QTextBrowser> InternalBrowser_;
				std::auto_ptr<IWebWidget> ExternalBrowser_;
			public:
				SelectableBrowser (QWidget* = 0);
				void Construct (IWebBrowser*);

				void SetHtml (const QString&, const QUrl& = QUrl ());
			};
		};
	};
};

#endif

