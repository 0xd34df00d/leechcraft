#ifndef PLUGINS_POSHUKU_PLUGINS_FILESCHEME_SCHEMEREPLY_H
#define PLUGINS_POSHUKU_PLUGINS_FILESCHEME_SCHEMEREPLY_H
#include <QNetworkReply>
#include <QBuffer>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace FileScheme
				{
					class SchemeReply : public QNetworkReply
					{
						Q_OBJECT

						QBuffer Buffer_;
					public:
						SchemeReply (const QNetworkRequest&, QObject* = 0);
						virtual ~SchemeReply ();

						virtual qint64 bytesAvailable () const;
						virtual void abort ();
						virtual void close ();
					protected:
						virtual qint64 readData (char*, qint64);
					private slots:
						void list ();
					};
				};
			};
		};
	};
};

#endif

