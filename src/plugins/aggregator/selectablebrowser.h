#ifndef PLUGINS_AGGREGATOR_SELECTABLEBROWSER_H
#define PLUGINS_AGGREGATOR_SELECTABLEBROWSER_H
#include <memory>
#include <QWidget>
#include <QTextEdit>
#include <interfaces/iwebbrowser.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class SelectableBrowser : public QWidget
			{
				Q_OBJECT

				bool Internal_;
				std::auto_ptr<QTextEdit> InternalBrowser_;
				std::auto_ptr<IWebWidget> ExternalBrowser_;
			public:
				SelectableBrowser (QWidget* = 0);
				void Construct (IWebBrowser*);

				void SetHtml (const QString&, const QUrl& = QString ());
			};
		};
	};
};

#endif

