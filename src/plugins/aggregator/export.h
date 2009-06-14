#ifndef EXPORT_H
#define EXPORT_H
#include <QDialog>
#include "ui_export.h"
#include "channel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class Export : public QDialog
			{
				Q_OBJECT

				Ui::Export Ui_;
				QString Title_;
				QString Choices_;
			public:
				Export (const QString&, const QString&, const QString&, QWidget* = 0);
				virtual ~Export ();

				QString GetDestination () const;
				QString GetTitle () const;
				QString GetOwner () const;
				QString GetOwnerEmail () const;
				std::vector<bool> GetSelectedFeeds () const;

				void SetFeeds (const channels_shorts_t&);
			private slots:
				void on_File__textEdited (const QString&);
				void on_Browse__released ();
			};
		};
	};
};

#endif

