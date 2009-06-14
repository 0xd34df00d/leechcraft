#ifndef PLUGINS_BITTORRENT_ADDMULTIPLETORRENTS_H
#define PLUGINS_BITTORRENT_ADDMULTIPLETORRENTS_H
#include "ui_addmultipletorrents.h"
#include "core.h"

namespace LeechCraft
{
	namespace Util
	{
		class TagsLineEdit;
	};

	namespace Plugins
	{
		namespace BitTorrent
		{

			class AddMultipleTorrents : public QDialog, private Ui::AddMultipleTorrents
			{
				Q_OBJECT
			public:
				AddMultipleTorrents (QWidget *parent = 0);
				QString GetOpenDirectory () const;
				QString GetSaveDirectory () const;
				Core::AddType GetAddType () const;
				Util::TagsLineEdit* GetEdit ();
				QStringList GetTags () const;
			private slots:
				void on_BrowseOpen__released ();
				void on_BrowseSave__released ();
			};
		};
	};
};

#endif

