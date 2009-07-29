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
				setEnabled (false);

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
				
				if (!url.path ().endsWith ("/"))
				{
					QUrl nu = url;
					int lastIndex = url.path ().lastIndexOf ("/");
					if (lastIndex >= 0)
						nu.setPath (url.path ().left (lastIndex + 1));
					else
						nu.setPath ("/");
					Ui_.Address_->setText (nu.toString ());
					JobID_ = Core::Instance ().Browse (nu);
				}
				else
				{
					Ui_.Address_->setText (url.toString ());
					JobID_ = Core::Instance ().Browse (url);
				}
			}

			void Pane::Navigate (const QString& string)
			{
				if (!IsLocal ())
					Ui_.Address_->completer ()->setModel (DirModel_);

				if (!string.endsWith ('/'))
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

			QString Pane::GetString () const
			{
				return Ui_.Address_->text ();
			}

			void Pane::on_Address__returnPressed ()
			{
				QString text = Ui_.Address_->text ();
				if (text.startsWith ("ftp://"))
					SetURL (QUrl (text));
				else
					Navigate (text);
			}

			void Pane::on_Tree__activated (const QModelIndex& si)
			{
				QModelIndex index = StaticSource_->mapToSource (si);
				if (IsLocal ())
				{
					QString path = DirModel_->filePath (index);
					if (DirModel_->isDir (index))
						Navigate (path);
					else
					{
						if (XmlSettingsManager::Instance ()
								.property ("ActivatedTransfers").toBool ())
							emit uploadRequested (path);
					}
				}
				else
				{
					int row = index.row ();
					QUrl url = RemoteModel_->item (row, CName)->data (RDUrl).toUrl ();
					if (RemoteModel_->item (row, CType)->data (RDIsDir).toBool ())
						SetURL (url);
					else
					{
						if (XmlSettingsManager::Instance ()
								.property ("ActivatedTransfers").toBool ())
							emit downloadRequested (url);
					}
				}
			}

			void Pane::on_Up__released ()
			{
				QString text = Ui_.Address_->text ();
				if (IsLocal ())
				{
					int lastIndex = text.lastIndexOf ('/');
					if (lastIndex >= 0)
					{
						text = text.left (lastIndex + 1);
						Navigate (text);
					}
				}
				else
				{
					QUrl url (text);
					int lastIndex = url.path ().lastIndexOf ('/', -2);
					if (lastIndex > 0)
						url.setPath (url.path ().left (lastIndex + 1));
					else
						url.setPath ("/");
					SetURL (url);
				}
			}

			void Pane::on_Root__released ()
			{
				if (IsLocal ())
					Navigate ("/");
				else
				{
					QUrl url (Ui_.Address_->text ());
					url.setPath ("/");
					SetURL (url);
				}
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

				QStandardItem *name = new QStandardItem (icon.pixmap (32, 32), e.Name_);
				name->setData (e.URL_, RDUrl);
				items << name;

				QStandardItem *size = new QStandardItem (Util::Proxy::Instance ()->MakePrettySize (e.Size_));
				size->setTextAlignment (Qt::AlignRight);
				items << size;

				QStandardItem *type = new QStandardItem (e.IsDir_ ?
						tr ("Directory") :
						tr ("File"));
				type->setData (e.IsDir_, RDIsDir);
				items << type;

				items << new QStandardItem (e.DateTime_.toString (Qt::SystemLocaleShortDate));

				Q_FOREACH (QStandardItem *item, items)
					item->setEditable (false);
				RemoteModel_->appendRow (items);

				setEnabled (true);
			}
		};
	};
};

