/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <QDomDocument>
#include <QMetaType>
#include <QFile>
#include <QSettings>
#include <QTextCodec>
#include <QInputDialog>
#include <QCoreApplication>
#include <QtDebug>
#include <interfaces/iwebbrowser.h>
#include <plugininterface/util.h>
#include <plugininterface/syncops.h>
#include "findproxy.h"
#include "tagsasker.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			const QString Core::OS_ = "http://a9.com/-/spec/opensearch/1.1/";

			Core::Core ()
			: DeltaStorage_ ("org.LeechCraft.SeekThru", this)
			{
				qRegisterMetaType<Description> ("LeechCraft::Plugins::SeekThru::Description");
				qRegisterMetaTypeStreamOperators<UrlDescription> ("LeechCraft::Plugins::SeekThru::UrlDescription");
				qRegisterMetaTypeStreamOperators<QueryDescription> ("LeechCraft::Plugins::SeekThru::QueryDescription");
				qRegisterMetaTypeStreamOperators<Description> ("LeechCraft::Plugins::SeekThru::Description");

				ActionMapper_.AddFunctor (0, DADescrAdded,
						boost::bind (&Core::HandleDADescrAdded, this, _1));
				ActionMapper_.AddFunctor (0, DADescrRemoved,
						boost::bind (&Core::HandleDADescrRemoved, this, _1));
				ActionMapper_.AddFunctor (0, DATagsChanged,
						boost::bind (&Core::HandleDATagsChanged, this, _1));

				ReadSettings ();
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::DoDelayedInit ()
			{
				Headers_ << tr ("Short name");
			}

			int Core::columnCount (const QModelIndex&) const
			{
				return Headers_.size ();
			}

			QVariant Core::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();

				Description d = Descriptions_.at (index.row ());
				switch (index.column ())
				{
					case 0:
						switch (role)
						{
							case Qt::DisplayRole:
								return d.ShortName_;
							case RoleDescription:
								return d.Description_;
							case RoleContact:
								return d.Contact_;
							case RoleTags:
								{
									QStringList result;
									Q_FOREACH (QString tag, d.Tags_)
										result << Proxy_->GetTagsManager ()->GetTag (tag);
									return result;
								}
							case RoleLongName:
								return d.LongName_;
							case RoleDeveloper:
								return d.Developer_;
							case RoleAttribution:
								return d.Attribution_;
							case RoleRight:
								switch (d.Right_)
								{
									case Description::SROpen:
										return tr ("Open");
									case Description::SRLimited:
										return tr ("Limited");
									case Description::SRPrivate:
										return tr ("Private");
									case Description::SRClosed:
										return tr ("Closed");
								}
							default:
								return QVariant ();
						}
					default:
						return QVariant ();
				}
			}

			Qt::ItemFlags Core::flags (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;
				else
					return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			}

			QVariant Core::headerData (int pos, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return Headers_.at (pos);
				else
					return QVariant ();
			}

			QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();

				return createIndex (row, column);
			}

			QModelIndex Core::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}

			int Core::rowCount (const QModelIndex& parent) const
			{
				return parent.isValid () ? 0 : Descriptions_.size ();
			}

			void Core::SetProvider (QObject *provider, const QString& feature)
			{
				Providers_ [feature] = provider;
			}

			bool Core::CouldHandle (const LeechCraft::Entity& e) const
			{
				if (!e.Entity_.canConvert<QUrl> ())
					return false;

				QUrl url = e.Entity_.toUrl ();
				if (url.scheme () != "http" &&
						url.scheme () != "https")
					return false;

				if (e.Mime_ != "application/opensearchdescription+xml")
					return false;

				return true;
			}

			Sync::Payloads_t Core::GetAllDeltas (const Sync::ChainID_t& chainId)
			{
				if (chainId != "osengines")
					return Sync::Payloads_t ();

				quint8 version = 0;
				quint16 action = DADescrAdded;
				Sync::Payloads_t result;
				Q_FOREACH (const Description& descr, Descriptions_)
				{
					QByteArray serialized;
					{
						QDataStream s (&serialized, QIODevice::WriteOnly);
						s << version
								<< action
								<< descr;
					}
					Sync::Payload payload = Sync::CreatePayload (serialized);
					result << payload;
				}
				return result;
			}

			Sync::Payloads_t Core::GetNewDeltas (const Sync::ChainID_t& chainId)
			{
				return DeltaStorage_.Get (chainId);
			}

			void Core::PurgeNewDeltas (const Sync::ChainID_t& chainId, quint32 num)
			{
				DeltaStorage_.Purge (chainId, num);
			}

			void Core::ApplyDeltas (const Sync::Payloads_t& payloads, const Sync::ChainID_t& chainId)
			{
				if (chainId != "osengines")
					return;

				Sync::Payloads_t our = GetNewDeltas (chainId);
				quint32 ourSize = our.size ();
				bool shouldResync = false;

				Q_FOREACH (const Sync::Payload& pl, payloads)
				{
					shouldResync = shouldResync || our.removeAll (pl);
					if (!ActionMapper_.Process (pl.Data_))
						qWarning () << Q_FUNC_INFO
								<< "failed to process the payload";
				}

				WriteSettings ();
				if (shouldResync)
				{
					PurgeNewDeltas (chainId, ourSize);
					DeltaStorage_.Store (chainId, our);
				}
			}

			void Core::Add (const QUrl& url)
			{
				QString name = LeechCraft::Util::GetTemporaryName ();
				LeechCraft::Entity e = LeechCraft::Util::MakeEntity (url,
						name,
						LeechCraft::Internal |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::NotPersistent);

				int id = -1;
				QObject *provider;
				emit delegateEntity (e, &id, &provider);
				if (id == -1)
				{
					emit error (tr ("%1 wasn't delegated")
							.arg (url.toString ()));
					return;
				}

				HandleProvider (provider);
				Jobs_ [id] = name;
			}

			void Core::Remove (const QModelIndex& index)
			{
				QStringList oldCats = ComputeUniqueCategories ();

				QByteArray serialized;
				{
					QDataStream ds (&serialized, QIODevice::WriteOnly);
					ds << quint8 (0)
							<< quint16 (DADescrRemoved)
							<< Descriptions_.at (index.row ()).ShortName_;
				}
				DeltaStorage_.Store ("osengines", Sync::CreatePayload (serialized));

				beginRemoveRows (QModelIndex (), index.row (), index.row ());
				Descriptions_.removeAt (index.row ());
				endRemoveRows ();

				WriteSettings ();

				QStringList newCats = ComputeUniqueCategories ();
				emit categoriesChanged (newCats, oldCats);

				emit newDeltasAvailable ("osengines");
			}

			void Core::SetTags (const QModelIndex& index, const QStringList& tags)
			{
				SetTags (index.row (), tags);

				QByteArray serialized;
				{
					QDataStream ds (&serialized, QIODevice::WriteOnly);
					ds << quint8 (0)
							<< quint16 (DATagsChanged)
							<< Descriptions_.at (index.row ()).ShortName_
							<< tags;
				}
				DeltaStorage_.Store ("osengines", Sync::CreatePayload (serialized));

				emit newDeltasAvailable ("osengines");
			}

			void Core::SetTags (int pos, const QStringList& tags)
			{
				QStringList oldCats = ComputeUniqueCategories ();

				Descriptions_ [pos].Tags_.clear ();
				Q_FOREACH (QString tag, tags)
					Descriptions_ [pos].Tags_ <<
						Proxy_->GetTagsManager ()->GetID (tag);

				WriteSettings ();

				QStringList newCats = ComputeUniqueCategories ();
				emit categoriesChanged (newCats, oldCats);
			}

			QStringList Core::GetCategories () const
			{
				QStringList result;
				for (QList<Description>::const_iterator i = Descriptions_.begin (),
						end = Descriptions_.end (); i != end; ++i)
					Q_FOREACH (QString tag, i->Tags_)
						result += Proxy_->GetTagsManager ()->GetTag (tag);

				result.sort ();
				result.erase (std::unique (result.begin (), result.end ()), result.end ());

				return result;
			}

			IFindProxy_ptr Core::GetProxy (const LeechCraft::Request& r)
			{
				QList<SearchHandler_ptr> handlers;
				Q_FOREACH (Description d, Descriptions_)
				{
					QStringList ht;
					Q_FOREACH (QString id, d.Tags_)
						ht << Proxy_->GetTagsManager ()->GetTag (id);
					if (ht.contains (r.Category_))
					{
						SearchHandler_ptr sh (new SearchHandler (d));
						connect (sh.get (),
								SIGNAL (delegateEntity (const LeechCraft::Entity&,
										int*, QObject**)),
								this,
								SIGNAL (delegateEntity (const LeechCraft::Entity&,
										int*, QObject**)));
						connect (sh.get (),
								SIGNAL (gotEntity (const LeechCraft::Entity&)),
								this,
								SIGNAL (gotEntity (const LeechCraft::Entity&)));
						connect (sh.get (),
								SIGNAL (error (const QString&)),
								this,
								SIGNAL (error (const QString&)));
						connect (sh.get (),
								SIGNAL (warning (const QString&)),
								this,
								SIGNAL (warning (const QString&)));
						handlers << sh;
					}
				}

				boost::shared_ptr<FindProxy> fp (new FindProxy (r));
				fp->SetHandlers (handlers);
				return IFindProxy_ptr (fp);
			}

			IWebBrowser* Core::GetWebBrowser () const
			{
				if (Providers_.contains ("webbrowser"))
					return qobject_cast<IWebBrowser*> (Providers_ ["webbrowser"]);
				else
					return 0;
			}

			void Core::handleJobFinished (int id)
			{
				if (!Jobs_.contains (id))
					return;
				QString filename = Jobs_ [id];
				Jobs_.remove (id);

				QFile file (filename);
				if (!file.open (QIODevice::ReadOnly))
				{
					emit error (tr ("Could not open file %1.")
							.arg (filename));
					return;
				}

				HandleEntity (QTextCodec::codecForName ("UTF-8")->
						toUnicode (file.readAll ()));

				file.close ();
				if (!file.remove ())
					emit warning (tr ("Could not remove temporary file %1.")
							.arg (filename));
			}

			void Core::handleJobError (int id)
			{
				if (!Jobs_.contains (id))
					return;
				emit error (tr ("A job was delegated, but it failed.")
						.arg (Jobs_ [id]));
				Jobs_.remove (id);
			}

			void Core::HandleEntity (const QString& contents, const QString& useTags)
			{
				try
				{
					Description descr = ParseData (contents, useTags);

					QStringList oldCats = ComputeUniqueCategories ();

					beginInsertRows (QModelIndex (), Descriptions_.size (), Descriptions_.size ());
					Descriptions_ << descr;
					endInsertRows ();

					WriteSettings ();

					QStringList newCats = ComputeUniqueCategories ();
					emit categoriesChanged (newCats, oldCats);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}
			}

			QStringList Core::ComputeUniqueCategories () const
			{
				QStringList ids;
				Q_FOREACH (Description d, Descriptions_)
					Q_FOREACH (QString id, d.Tags_)
						if (!ids.contains (id))
							ids << id;

				QStringList result;
				Q_FOREACH (QString id, ids)
					result << Proxy_->GetTagsManager ()->GetTag (id);
				return result;
			}

			Description Core::ParseData (const QString& contents, const QString& useTags)
			{
				QDomDocument doc;
				QString errorString;
				int line, column;
				if (!doc.setContent (contents, true, &errorString, &line, &column))
				{
					qWarning () << contents;
					emit error (tr ("XML parse error %1 at %2:%3.")
							.arg (errorString)
							.arg (line)
							.arg (column));
					throw std::runtime_error ("Parse error");
				}

				QDomElement root = doc.documentElement ();
				if (root.tagName () != "OpenSearchDescription")
					throw std::runtime_error ("Parse error");

				QDomElement shortNameTag = root.firstChildElement ("ShortName");
				QDomElement descriptionTag = root.firstChildElement ("Description");
				QDomElement urlTag = root.firstChildElement ("Url");
				if (shortNameTag.isNull () ||
						descriptionTag.isNull () ||
						urlTag.isNull () ||
						!urlTag.hasAttribute ("template") ||
						!urlTag.hasAttribute ("type"))
					throw std::runtime_error ("Parse error");

				Description descr;
				descr.ShortName_ = shortNameTag.text ();
				descr.Description_ = descriptionTag.text ();

				while (!urlTag.isNull ())
				{
					UrlDescription d =
					{
						urlTag.attribute ("template"),
						urlTag.attribute ("type"),
						urlTag.attribute ("indexOffset", "1").toInt (),
						urlTag.attribute ("pageOffset", "1").toInt ()
					};
					descr.URLs_ << d;

					urlTag = urlTag.nextSiblingElement ("Url");
				}

				QDomElement contactTag = root.firstChildElement ("Contact");
				if (!contactTag.isNull ())
					descr.Contact_ = contactTag.text ();

				if (useTags.isEmpty ())
				{
					QDomElement tagsTag = root.firstChildElement ("Tags");
					if (!tagsTag.isNull ())
						descr.Tags_ = Proxy_->GetTagsManager ()->Split (tagsTag.text ());
					else
						descr.Tags_ = QStringList ("default");

					TagsAsker ta (Proxy_->GetTagsManager ()->Join (descr.Tags_));
					QString userTags;
					qDebug () << Q_FUNC_INFO;
					if (ta.exec () == QDialog::Accepted)
						userTags = ta.GetTags ();

					if (!userTags.isEmpty ())
						descr.Tags_ = Proxy_->GetTagsManager ()->Split (userTags);
				}
				else
					descr.Tags_ = Proxy_->GetTagsManager ()->Split (useTags);

				QStringList hrTags = descr.Tags_;
				descr.Tags_.clear ();
				Q_FOREACH (QString tag, hrTags)
					descr.Tags_ << Proxy_->GetTagsManager ()->GetID (tag);

				QDomElement longNameTag = root.firstChildElement ("LongName");
				if (!longNameTag.isNull ())
					descr.LongName_ = longNameTag.text ();

				QDomElement queryTag = root.firstChildElement ("Query");
				while (!queryTag.isNull () && queryTag.hasAttributeNS (OS_, "role"))
				{
					QueryDescription::Role r;
					QString role = queryTag.attributeNS (OS_, "role");
					if (role == "request")
						r = QueryDescription::RoleRequest;
					else if (role == "example")
						r = QueryDescription::RoleExample;
					else if (role == "related")
						r = QueryDescription::RoleRelated;
					else if (role == "correction")
						r = QueryDescription::RoleCorrection;
					else if (role == "subset")
						r = QueryDescription::RoleSubset;
					else if (role == "superset")
						r = QueryDescription::RoleSuperset;
					else
					{
						queryTag = queryTag.nextSiblingElement ("Query");
						continue;
					}

					QueryDescription d =
					{
						r,
						queryTag.attributeNS (OS_, "title"),
						queryTag.attributeNS (OS_, "totalResults", "-1").toInt (),
						queryTag.attributeNS (OS_, "searchTerms"),
						queryTag.attributeNS (OS_, "count", "-1").toInt (),
						queryTag.attributeNS (OS_, "startIndex", "-1").toInt (),
						queryTag.attributeNS (OS_, "startPage", "-1").toInt (),
						queryTag.attributeNS (OS_, "language", "*"),
						queryTag.attributeNS (OS_, "inputEncoding", "UTF-8"),
						queryTag.attributeNS (OS_, "outputEncoding", "UTF-8")
					};
					descr.Queries_ << d;
					queryTag = queryTag.nextSiblingElement ("Query");
				}

				QDomElement developerTag = root.firstChildElement ("Developer");
				if (!developerTag.isNull ())
					descr.Developer_ = developerTag.text ();

				QDomElement attributionTag = root.firstChildElement ("Attribution");
				if (!attributionTag.isNull ())
					descr.Attribution_ = attributionTag.text ();

				descr.Right_ = Description::SROpen;
				QDomElement syndicationRightTag = root.firstChildElement ("SyndicationRight");
				if (!syndicationRightTag.isNull ())
				{
					QString sr = syndicationRightTag.text ();
					if (sr == "limited")
						descr.Right_ = Description::SRLimited;
					else if (sr == "private")
						descr.Right_ = Description::SRPrivate;
					else if (sr == "closed")
						descr.Right_ = Description::SRClosed;
				}

				descr.Adult_ = false;
				QDomElement adultContentTag = root.firstChildElement ("AdultContent");
				if (!adultContentTag.isNull ())
				{
					QString text = adultContentTag.text ();
					if (!(text == "false" ||
							text == "FALSE" ||
							text == "0" ||
							text == "no" ||
							text == "NO"))
						descr.Adult_ = true;
				}

				QDomElement languageTag = root.firstChildElement ("Language");
				bool was = false;;
				while (!languageTag.isNull ())
				{
					descr.Languages_ << languageTag.text ();
					was = true;
					languageTag = languageTag.nextSiblingElement ("Language");
				}
				if (!was)
					descr.Languages_ << "*";

				QDomElement inputEncodingTag = root.firstChildElement ("InputEncoding");
				was = false;
				while (!inputEncodingTag.isNull ())
				{
					descr.InputEncodings_ << inputEncodingTag.text ();
					was = true;
					inputEncodingTag = inputEncodingTag.nextSiblingElement ("InputEncoding");
				}
				if (!was)
					descr.InputEncodings_ << "UTF-8";

				QDomElement outputEncodingTag = root.firstChildElement ("OutputEncoding");
				was = false;
				while (!outputEncodingTag.isNull ())
				{
					descr.InputEncodings_ << outputEncodingTag.text ();
					was = true;
					outputEncodingTag = outputEncodingTag.nextSiblingElement ("OutputEncoding");
				}
				if (!was)
					descr.InputEncodings_ << "UTF-8";

				return descr;
			}

			void Core::HandleProvider (QObject *provider)
			{
				if (Downloaders_.contains (provider))
					return;

				Downloaders_ << provider;
				connect (provider,
						SIGNAL (jobFinished (int)),
						this,
						SLOT (handleJobFinished (int)));
				connect (provider,
						SIGNAL (jobError (int, IDownload::Error)),
						this,
						SLOT (handleJobError (int)));
			}

			void Core::ReadSettings ()
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_SeekThru");
				int size = settings.beginReadArray ("Descriptions");
				for (int i = 0; i < size; ++i)
				{
					settings.setArrayIndex (i);
					Descriptions_ << settings.value ("Description").value<Description> ();
				}
				settings.endArray ();
			}

			void Core::WriteSettings ()
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_SeekThru");
				settings.beginWriteArray ("Descriptions");
				for (int i = 0; i < Descriptions_.size (); ++i)
				{
					settings.setArrayIndex (i);
					settings.setValue ("Description",
							QVariant::fromValue<Description> (Descriptions_.at (i)));
				}
				settings.endArray ();
			}

			bool Core::HandleDADescrAdded (QDataStream& in)
			{
				Description descr;
				in >> descr;
				if (in.status () != QDataStream::Ok)
				{
					qWarning () << Q_FUNC_INFO
							<< "bad stream status"
							<< in.status ();
					return false;
				}

				QList<Description>::iterator pos =
						std::find_if (Descriptions_.begin (), Descriptions_.end (),
								boost::bind (&Description::ShortName_, _1) == descr.ShortName_);
				if (pos == Descriptions_.end ())
					Descriptions_ << descr;
				else
					*pos = descr;
				return true;
			}

			bool Core::HandleDADescrRemoved (QDataStream& in)
			{
				QString shortName;
				in >> shortName;
				if (in.status () != QDataStream::Ok)
				{
					qWarning () << Q_FUNC_INFO
							<< "bad stream status"
							<< in.status ();
					return false;
				}

				QList<Description>::iterator pos =
						std::find_if (Descriptions_.begin (), Descriptions_.end (),
								boost::bind (&Description::ShortName_, _1) == shortName);
				if (pos != Descriptions_.end ())
					Descriptions_.erase (pos);
				return false;
			}

			bool Core::HandleDATagsChanged (QDataStream& in)
			{
				QString shortName;
				in >> shortName;
				QStringList tags;
				in >> tags;
				if (in.status () != QDataStream::Ok)
				{
					qWarning () << Q_FUNC_INFO
							<< "bad stream status"
							<< in.status ();
					return false;
				}

				QList<Description>::iterator pos =
						std::find_if (Descriptions_.begin (), Descriptions_.end (),
								boost::bind (&Description::ShortName_, _1) == shortName);
				if (pos != Descriptions_.end ())
				{
					SetTags (std::distance (Descriptions_.begin (), pos), tags);
					return true;
				}
				else
				{
					qWarning () << Q_FUNC_INFO
							<< "could not find the required description"
							<< shortName;
					return false;
				}
			}
		};
	};
};

