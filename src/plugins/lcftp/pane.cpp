#include "pane.h"
#include <QUrl>
#include <QDirModel>
#include <QCompleter>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			Pane::Pane (QWidget *parent)
			: QWidget (parent)
			, StaticSource_ (new QSortFilterProxyModel (this))
			, DirModel_ (new QDirModel (this))
			, RemoteModel_ (new QStandardItemModel (this))
			, JobID_ (-1)
			{
				Ui_.setupUi (this);
				Ui_.Address_->setCompleter (new QCompleter ());
				Ui_.Tree_->setModel (StaticSource_);
				StaticSource_->setDynamicSortFilter (true);

				DirModel_->setSorting (QDir::DirsFirst | QDir::IgnoreCase);

				connect (&Core::Instance (),
						SIGNAL (fetchedEntry (const FetchedEntry&)),
						this,
						SLOT (handleFetchedEntry (const FetchedEntry&)));
			}

			Pane::~Pane ()
			{
			}

			void Pane::SetURL (const QUrl& url)
			{
				if (IsLocal ())
					Ui_.Address_->completer ()->setModel (0);

				RemoteModel_->clear ();

				QStringList headers;
				headers << tr ("Name")
					<< tr ("Size")
					<< tr ("Type")
					<< tr ("Date");
				RemoteModel_->setHorizontalHeaderLabels (headers);

				StaticSource_->setSourceModel (RemoteModel_);
				Ui_.Address_->setText (url.toString ());
				
				if (!url.path ().endsWith ("/"))
				{
					QUrl nu = url;
					int lastIndex = url.path ().lastIndexOf ("/");
					if (lastIndex > 0)
						nu.setPath (url.path ().left (lastIndex + 1));
					else
						nu.setPath ("/");
					JobID_ = Core::Instance ().Browse (nu);
				}
				else
					JobID_ = Core::Instance ().Browse (url);

				setEnabled (false);
			}

			void Pane::Navigate (const QString& string)
			{
				if (!IsLocal ())
					Ui_.Address_->completer ()->setModel (DirModel_);

				if (!string.endsWith ("/"))
					Ui_.Address_->setText (string);
				else
					Ui_.Address_->setText (string.left (string.size () - 1));

				StaticSource_->setSourceModel (DirModel_);
				Ui_.Tree_->setRootIndex (StaticSource_->
						mapFromSource (DirModel_->index (string)));

				XmlSettingsManager::Instance ()
					.setProperty ("LastPanedLocalPath", string);
			}

			bool Pane::IsLocal () const
			{
				return Ui_.Address_->completer ()->model ();
			}

			void Pane::on_Address__returnPressed ()
			{
				QString text = Ui_.Address_->text ();
				if (text.startsWith ("ftp://"))
					SetURL (QUrl (text));
				else
					Navigate (text);
			}

			void Pane::handleFetchedEntry (const FetchedEntry& e)
			{
				if (e.PreviousTask_.ID_ != JobID_)
					return;

				QIcon icon = qApp->style ()->
					standardIcon (e.IsDir_ ?
							QStyle::SP_DirIcon :
							QStyle::SP_FileIcon);

				QList<QStandardItem*> items;
				items << new QStandardItem (icon.pixmap (32, 32), e.Name_);
				QStandardItem *size = new QStandardItem (Util::Proxy::Instance ()->MakePrettySize (e.Size_));
				size->setTextAlignment (Qt::AlignRight);
				items << size;
				items << new QStandardItem (e.IsDir_ ? tr ("Directory") : tr ("File"));
				items << new QStandardItem (e.DateTime_.toString (Qt::SystemLocaleShortDate));
				RemoteModel_->appendRow (items);

				setEnabled (true);
			}
		};
	};
};

