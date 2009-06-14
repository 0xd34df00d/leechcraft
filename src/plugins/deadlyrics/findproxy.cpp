#include "findproxy.h"
#include <QTextEdit>
#include <QUrl>
#include <QtDebug>
#include <interfaces/structures.h>
#include "searcher.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			FindProxy::FindProxy (const LeechCraft::Request& request,
					QObject *parent)
			: QAbstractItemModel (parent)
			, Request_ (request)
			, FetchedSomething_ (false)
			{
				setObjectName ("DeadLyRicS FindProxy");
				LyricsHolder_ = new QTextEdit ();
				QStringList subs = Request_.String_.split (" - ", QString::SkipEmptyParts);
				if (subs.size () < 2)
					return;
			
				searchers_t searchers = Core::Instance ().GetSearchers (Request_.Category_);
				for (searchers_t::iterator i = searchers.begin (),
						end = searchers.end (); i != end; ++i)
				{
					connect (i->get (),
							SIGNAL (textFetched (const Lyrics&, const QByteArray&)),
							this,
							SLOT (handleTextFetched (const Lyrics&, const QByteArray&)),
							Qt::QueuedConnection);
					connect (i->get (),
							SIGNAL (error (const QString&)),
							this,
							SLOT (handleError (const QString&)),
							Qt::QueuedConnection);
					QByteArray hash;
					(*i)->Start (subs, hash);
					Hashes_.push_back (hash);
				}
			}
			
			FindProxy::~FindProxy ()
			{
				size_t size = Hashes_.size ();
				if (size)
				{
					searchers_t searchers = Core::Instance ().GetSearchers (Request_.Category_);
					for (size_t i = 0; i < size; ++i)
						searchers [i]->Stop (Hashes_ [i]);
				}
			}
			
			QAbstractItemModel* FindProxy::GetModel ()
			{
				return this;
			}
			
			int FindProxy::columnCount (const QModelIndex&) const
			{
				return 3;
			}
			
			QVariant FindProxy::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				if (Lyrics_.size ())
				{
					Lyrics lyrics = Lyrics_ [index.row ()];
					if (role == Qt::DisplayRole)
					{
						switch (index.column ())
						{
							case 0:
								{
									QString result = lyrics.Author_;
									if (!lyrics.Album_.isEmpty ())
										result.append (" - ").append (lyrics.Album_);
									result.append (" - ").append (lyrics.Title_);
									return result;
								}
							case 2:
								return QString ("%1 (%2)")
									.arg (QUrl (lyrics.URL_).host ())
									.arg (lyrics.URL_);
							default:
								return Request_.Category_;
						}
					}
					else if (role == LeechCraft::RoleAdditionalInfo)
					{
						LyricsHolder_->setPlainText (lyrics.Text_);
						return QVariant::fromValue<QWidget*> (LyricsHolder_);
					}
					else
						return QVariant ();
				}
				else
				{
					if (role == Qt::DisplayRole)
					{
						switch (index.column ())
						{
							case 0:
								return Request_.String_;
							case 2:
								if (ErrorString_.size ())
									return ErrorString_;
								else
									return tr ("Searching...");
							default:
								return Request_.Category_;
						}
					}
					else
						return QVariant ();
				}
			}
			
			QModelIndex FindProxy::index (int row, int column,
					const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
				
				return createIndex (row, column);
			}
			
			QModelIndex FindProxy::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int FindProxy::rowCount (const QModelIndex& index) const
			{
				if (index.isValid ())
					return 0;
				else
				{
					if (FetchedSomething_)
						return Lyrics_.size ();
					else
						return 1;
				}
			}
			
			void FindProxy::handleTextFetched (const Lyrics& lyrics,
					const QByteArray& hash)
			{
				if (std::find (Hashes_.begin (), Hashes_.end (), hash) == Hashes_.end ())
					return;
			
				if (!Lyrics_.size ())
				{
					beginRemoveRows (QModelIndex (), 0, 0);
					FetchedSomething_ = true;
					endRemoveRows ();
				}
				beginInsertRows (QModelIndex (), Lyrics_.size (), Lyrics_.size ());
				Lyrics_.push_back (lyrics);
				endInsertRows ();
			}
			
			void FindProxy::handleError (const QString& message)
			{
				ErrorString_ = message;
				if (!FetchedSomething_)
					emit dataChanged (index (0, 0), index (0, 2));
			}
		};
	};
};

