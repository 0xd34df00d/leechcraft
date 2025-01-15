/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entrybase.h"
#include <variant>
#include <optional>
#include <QIcon>
#include <QXmlStreamWriter>
#include <util/util.h>
#include <util/sll/urloperator.h>
#include <util/sll/qtutil.h>
#include <util/sys/extensionsdata.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/azothutil.h>
#include <interfaces/azoth/iproxyobject.h>
#include "vkaccount.h"
#include "vkmessage.h"
#include "vkconnection.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	EntryBase::EntryBase (VkAccount *acc)
	: QObject (acc)
	, Account_ (acc)
	{
	}

	void EntryBase::Store (VkMessage *msg)
	{
		Messages_ << msg;
		emit gotMessage (msg);
	}

	QObject* EntryBase::GetQObject ()
	{
		return this;
	}

	IAccount* EntryBase::GetParentAccount () const
	{
		return Account_;
	}

	IMessage* EntryBase::CreateMessage (IMessage::Type type, const QString&, const QString& body)
	{
		auto msg = new VkMessage (true, IMessage::Direction::Out, type, this);
		msg->SetBody (body);
		return msg;
	}

	QList<IMessage*> EntryBase::GetAllMessages () const
	{
		QList<IMessage*> result;
		for (auto obj : Messages_)
			result << obj;
		return result;
	}

	void EntryBase::PurgeMessages (const QDateTime& before)
	{
		AzothUtil::StandardPurgeMessages (Messages_, before);
	}

	void EntryBase::MarkMsgsRead ()
	{
		Account_->GetParentProtocol ()->GetAzothProxy ()->MarkMessagesAsRead (this);

		if (!HasUnread_)
			return;

		QList<qulonglong> ids;
		for (auto msg : Messages_)
			if (!msg->IsRead ())
			{
				ids << msg->GetID ();
				msg->SetRead ();
			}

		HasUnread_ = false;

		if (!ids.isEmpty ())
			Account_->GetConnection ()->MarkAsRead (ids);
	}

	namespace
	{
		const QString AudioDivStyle = "border-color: #CDCCCC; "
				"margin-top: 2px; margin-bottom: 0px; "
				"border-width: 1px; border-style: solid; border-radius: 5px; "
				"padding-left: 5px; padding-right: 5px; padding-top: 2px; padding-bottom: 2px;";
		const QString RepostDivStyle = "border-color: #ABAAAA; "
				"margin-top: 2px; margin-bottom: 0px; margin-left: 1em; margin-right: 0em; "
				"border-width: 1px; border-style: solid; border-radius: 5px; "
				"padding-left: 5px; padding-right: 5px; padding-top: 2px; padding-bottom: 2px;";

		struct SimpleImageInfo
		{
			const QString Url_;
			const QString Alt_ = {};

			const std::optional<QSize> Size_ = {};

			SimpleImageInfo (const QString& url)
			: Url_ { url }
			{
			}

			SimpleImageInfo (const QString& url, const QString& alt, const QSize& size)
			: Url_ { url }
			, Alt_ { alt }
			, Size_ { size }
			{
			}
		};

		struct LinkImageInfo
		{
			const QString FullUrl_;
			const QString ThumbUrl_;

			const QString Alt_;

			const std::optional<QSize> FullSize_;
			const std::optional<QSize> ThumbSize_;
		};

		using ImageInfo = std::variant<SimpleImageInfo, LinkImageInfo>;

		void WriteImgDims (QXmlStreamWriter& w, const std::optional<QSize>& size)
		{
			if (!size)
				return;

			w.writeAttribute ("width", QString::number (size->width ()));
			w.writeAttribute ("height", QString::number (size->height ()));
		}

		enum class ImageTemplateMode
		{
			Embed,
			Link
		};

		QString GetImageTemplate (const SimpleImageInfo& info, ImageTemplateMode mode)
		{
			QString result;
			QXmlStreamWriter w { &result };

			switch (mode)
			{
			case ImageTemplateMode::Embed:
				w.writeStartElement ("img");
				w.writeAttribute ("src", info.Url_);
				w.writeAttribute ("alt", info.Alt_);
				w.writeAttribute ("title", info.Alt_);
				WriteImgDims (w, info.Size_);
				w.writeEndElement ();
				break;
			case ImageTemplateMode::Link:
				w.writeStartElement ("a");
				w.writeAttribute ("href", info.Url_);

				if (!info.Alt_.isEmpty ())
					w.writeCharacters (info.Alt_);
				else if (info.Size_)
					w.writeCharacters (QObject::tr ("Image, %1 by %2 pixels.")
							.arg (info.Size_->width ())
							.arg (info.Size_->height ()));
				else
					w.writeCharacters (QObject::tr ("Image"));

				w.writeEndElement ();
				break;
			}

			return result;
		}

		QString GetImageTemplate (const LinkImageInfo& info, ImageTemplateMode mode)
		{
			QString result;
			QXmlStreamWriter w { &result };

			const auto& alt = (info.Alt_.isEmpty () && info.FullSize_) ?
					QObject::tr ("Image, %1 by %2 pixels.")
							.arg (info.FullSize_->width ())
							.arg (info.FullSize_->height ()) :
					info.Alt_;

			switch (mode)
			{
			case ImageTemplateMode::Embed:
				w.writeStartElement ("a");
				w.writeAttribute ("href", info.FullUrl_);
				w.writeAttribute ("target", "_blank");
				w.writeStartElement ("img");
				w.writeAttribute ("src", info.ThumbUrl_);
				w.writeAttribute ("alt", alt);
				w.writeAttribute ("title", alt);
				WriteImgDims (w, info.ThumbSize_);
				w.writeEndElement ();
				w.writeEndElement ();
				break;
			case ImageTemplateMode::Link:
				w.writeStartElement ("a");
				w.writeAttribute ("href", info.FullUrl_);
				w.writeAttribute ("target", "_blank");
				w.writeCharacters (alt);
				w.writeEndElement ();
				break;
			}

			return result;
		}

		template<typename T>
		QString GetImageTemplate (T&& info)
		{
			const auto& showOpt = XmlSettingsManager::Instance ()
					.property ("ShowImagesInChat").toString ();
			auto mode = showOpt == "Embedded" ?
					ImageTemplateMode::Embed :
					ImageTemplateMode::Link;
			return GetImageTemplate (std::forward<T> (info), mode);
		}

		QString Gift2Replacement (const GiftInfo& info)
		{
			return GetImageTemplate (SimpleImageInfo { info.Thumb_.toEncoded () });
		}

		QString Photo2Replacement (const PhotoInfo& info)
		{
			return GetImageTemplate (LinkImageInfo
					{
						info.Full_,
						info.Thumbnail_,
						{},
						info.FullSize_,
						info.ThumbnailSize_
					});
		}

		QString Icon2Img (const QIcon& icon, const QString& name)
		{
			return GetImageTemplate (SimpleImageInfo
					{
						LC::Util::GetAsBase64Src (icon.pixmap (16, 16).toImage ()),
						name,
						{ 16, 16 }
					},
					ImageTemplateMode::Embed);
		}

		QString Audio2Replacement (const AudioInfo& info, const ICoreProxy_ptr& proxy)
		{
			auto durStr = LC::Util::MakeTimeFromLong (info.Duration_);
			if (durStr.startsWith ("00:"))
				durStr = durStr.mid (3);

			QUrl azothUrl;
			azothUrl.setScheme ("azoth");
			azothUrl.setHost ("sendentities");
			Util::UrlOperator { azothUrl }
					("count", "1")
					("entityVar0", info.URL_.toEncoded ())
					("entityType0", "url")
					("addCount0", "1");

			auto enqueueUrl = azothUrl;
			Util::UrlOperator { enqueueUrl }
					("flags0", "OnlyHandle")
					("add0key0", "Action")
					("add0value0", "AudioEnqueue");

			auto playUrl = azothUrl;
			Util::UrlOperator { playUrl }
					("flags0", "OnlyHandle")
					("add0key0", "Action")
					("add0value0", "AudioEnqueuePlay");

			auto downloadUrl = azothUrl;
			Util::UrlOperator { downloadUrl }
					("flags0", "OnlyDownload");

			QString result;

			auto addImage = [&proxy, &result] (const QString& icon, const QString& name)
			{
				result += Icon2Img (proxy->GetIconThemeManager ()->GetIcon (icon), name);
			};

			result += "<div style='" + AudioDivStyle + "'>";
			result += "<a href='";
			result += QString::fromUtf8 (enqueueUrl.toEncoded ());
			result += "'>";
			addImage ("list-add", EntryBase::tr ("Enqueue"));
			result += "</a> <a href='";
			result += QString::fromUtf8 (playUrl.toEncoded ());
			result += "'>";
			addImage ("media-playback-start", EntryBase::tr ("Play"));
			result += "</a> <a href='";
			result += QString::fromUtf8 (downloadUrl.toEncoded ());
			result += "'>";
			addImage ("download", EntryBase::tr ("Download"));
			result += "</a> ";
			result += info.Artist_ + QString::fromUtf8 (" — ") + info.Title_;
			result += " <span style='float:right'>" + durStr + "</span>";
			result += "</div>";
			return result;
		}

		QString Video2Replacement (const VideoInfo& info, const ICoreProxy_ptr&)
		{
			QString result = "<div>";
			result += QString ("<a href='http://vk.com/video%1_%2' target='_blank'>")
					.arg (info.OwnerID_)
					.arg (info.ID_);
			result += QString ("<img src='%1' width='320' height='240' alt='' /><br />")
					.arg (info.Image_.toEncoded ().constData ());
			result += "<strong>" + info.Title_ + "</strong> ";
			if (!info.Desc_.isEmpty ())
				result += "(" + info.Desc_ + ") ";
			result += "[" + LC::Util::MakeTimeFromLong (info.Duration_) + "] <br />";
			result += "</a></div>";

			return result;
		}

		QString Document2Replacement (const DocumentInfo& info, const ICoreProxy_ptr&)
		{
			QString result = "<div>";

			const auto& icon = Util::ExtensionsData::Instance ().GetExtIcon (info.Extension_);
			result += Icon2Img (icon, info.Extension_);

			result += QString::fromUtf8 ("<a href='%1'>%2</a> — ")
					.arg (info.Url_.toEncoded ().constData ())
					.arg (info.Title_);
			result += EntryBase::tr ("%1 document, size: %2")
					.arg (info.Extension_.toUpper ())
					.arg (Util::MakePrettySize (info.Size_));

			result += "</div>";
			return result;
		}

		QString PagePreview2Replacement (const PagePreview& info)
		{
			QString result = "<div>";
			result += EntryBase::tr ("Page:") + QString (" <a href='%1'>%2</a>")
					.arg (info.Url_.toEncoded ().constData ())
					.arg (info.Title_);
			result += "</div>";

			return result;
		}

		QString StickerId2Replacement (const QString& stickerId)
		{
			const auto stickerSize = XmlSettingsManager::Instance ().property ("StickersSize").toInt ();
			return GetImageTemplate (SimpleImageInfo
					{
						QString { "https://vk.com/images/stickers/%1/%2.png" }
								.arg (stickerId)
								.arg (stickerSize),
						{},
						{ stickerSize, stickerSize }
					});
		}

		struct ContentsInfo
		{
			QString Contents_;
			bool HasAdditionalInfo_;
			QStringList FwdIds_;
		};

		QString ProcessMessageBody (QString body)
		{
			QRegularExpression rx { "\\[([a-z]+[0-9]+?)\\|(.*?)\\]", QRegularExpression::CaseInsensitiveOption };
			body.replace (rx, "<a href='https://vk.com/\\1'>\\2</a>");

			return body;
		}

		ContentsInfo ToMessageContents (const MessageInfo& info)
		{
			auto newContents = ProcessMessageBody (info.Text_);
			struct AttachInfo
			{
				QString Type_;
				QString ID_;
			};
			QMap<int, AttachInfo> attaches;

			const QString attachMarker ("attach");
			const QString typeMarker ("_type");
			for (const auto& pair : Util::Stlize (info.Params_))
			{
				auto key = pair.first;
				if (!key.startsWith (attachMarker))
					continue;

				key = key.mid (attachMarker.size ());
				const bool isType = key.endsWith (typeMarker);
				if (isType)
					key.chop (typeMarker.size ());

				bool ok = false;
				const auto num = key.toInt (&ok);
				if (!ok)
					continue;

				auto& attach = attaches [num];
				if (isType)
					attach.Type_ = pair.second.toString ();
				else
					attach.ID_ = pair.second.toString ();
			}

			QStringList photoIds, wallIds, audioIds, videoIds, docIds, pageIds;
			for (const auto& info : attaches)
				if (info.Type_ == "photo")
					photoIds << info.ID_;
				else if (info.Type_ == "wall")
					wallIds << info.ID_;
				else if (info.Type_ == "audio")
					audioIds << info.ID_;
				else if (info.Type_ == "video")
					videoIds << info.ID_;
				else if (info.Type_ == "doc")
					docIds << info.ID_;
				else if (info.Type_ == "page")
					pageIds << info.ID_;
				else if (info.Type_ == "sticker")
					newContents += "<br/>" + StickerId2Replacement (info.ID_);

			const auto hasAdditional = !photoIds.isEmpty () ||
					!wallIds.isEmpty () ||
					!audioIds.isEmpty () ||
					!videoIds.isEmpty () ||
					!docIds.isEmpty () ||
					!pageIds.isEmpty ();

			for (const auto& id : photoIds)
				newContents += "<div id='photostub_" + id + "'></div>";
			for (const auto& id : wallIds)
				newContents += "<div id='wallstub_" + id + "'></div>";
			for (const auto& id : audioIds)
				newContents += "<div id='audiostub_" + id + "'></div>";
			for (const auto& id : videoIds)
				newContents += "<div id='videostub_" + id + "'></div>";
			for (const auto& id : docIds)
				newContents += "<div id='docstub_" + id + "'></div>";
			for (const auto& id : pageIds)
				newContents += "<div id='pagestub_" + id + "'></div>";

			const auto& fwdIds = info.Params_.value ("fwd").toString ().split (',', Qt::SkipEmptyParts);
			for (const auto& id : fwdIds)
				newContents += "<div id='fwdstub_" + id + "'></div>";

			return { newContents, hasAdditional, fwdIds };
		}

		enum class FullInfoMode
		{
			Normal,
			Forward,
			Repost
		};

		QString FullInfo2Replacement (const FullMessageInfo& info, const ICoreProxy_ptr& proxy, FullInfoMode mode)
		{
			QString replacement;

			if (mode == FullInfoMode::Forward)
			{
				replacement += "<div>";
				replacement += EntryBase::tr ("Forwarded message from %1")
						.arg (info.PostDate_.toString ());
				replacement += "</div>";
			}

			replacement += ProcessMessageBody (info.Text_);

			for (const auto& gift : info.Gifts_)
				replacement += "<br/>" + Gift2Replacement (gift);

			for (const auto& sticker : info.Stickers_)
				replacement += "<br/>" + StickerId2Replacement (sticker.Id_);

			for (const auto& photo : info.Photos_)
				replacement += "<br/>" + Photo2Replacement (photo);

			for (const auto& video : info.Videos_)
				replacement += "<br/>" + Video2Replacement (video, proxy);

			for (const auto& page : info.PagesPreviews_)
				replacement += "<br/>" + PagePreview2Replacement (page);

			if (!info.Audios_.empty ())
			{
				replacement += "<div>";
				for (const auto& audio : info.Audios_)
					replacement += Audio2Replacement (audio, proxy);
				replacement += "</div>";
			}

			if (!info.Documents_.isEmpty ())
			{
				replacement += "<div>";
				for (const auto& doc : info.Documents_)
					replacement += Document2Replacement (doc, proxy);
				replacement += "</div>";
			}

			for (const auto& repost : info.ContainedReposts_)
			{
				replacement += "<div style='" + RepostDivStyle + "'>";
				replacement += FullInfo2Replacement (repost, proxy, FullInfoMode::Repost);
				replacement += "</div>";
			}

			for (const auto& fwd : info.ForwardedMessages_)
			{
				replacement += "<div style='" + RepostDivStyle + "'>";
				replacement += FullInfo2Replacement (fwd, proxy, FullInfoMode::Forward);
				replacement += "</div>";
			}

			if (mode == FullInfoMode::Repost)
			{
				replacement += "<div style='text-align:right'>";
				replacement += EntryBase::tr ("Posted on: %1")
						.arg (info.PostDate_.toString ());
				if (info.Likes_ || info.Reposts_)
				{
					replacement += "<br/>";
					replacement += EntryBase::tr ("%n like(s)", 0, info.Likes_);
					replacement += "; ";
					replacement += EntryBase::tr ("%n repost(s)", 0, info.Reposts_);
				}
				replacement += "</div>";
			}
			return replacement;
		}

		template<typename T, typename R>
		void AppendReplacements (QList<QPair<QString, QString>>& replacements,
				const QString& name, const QList<T>& items, const R& func)
		{
			for (const auto& item : items)
			{
				const auto& id = QString ("%1_%2_%3")
						.arg (name)
						.arg (item.OwnerID_)
						.arg (item.ID_);
				replacements.append ({ id, func (item) });
			}
		}
	}

	void EntryBase::HandleAttaches (VkMessage *msg, const MessageInfo& info, const FullMessageInfo& full)
	{
		if (full.ID_)
		{
			const auto& body = FullInfo2Replacement (full, Account_->GetCoreProxy (), FullInfoMode::Normal);
			msg->SetBody (body);
			return;
		}

		const auto& contentsInfo = ToMessageContents (info);

		msg->SetBody (contentsInfo.Contents_);

		QPointer<VkMessage> safeMsg (msg);

		for (const auto& idStr : contentsInfo.FwdIds_)
		{
			std::size_t endIdx = 0;
			const auto id = std::stoull (idStr.section ('_', 1, 1).toStdString (), &endIdx);
			if (!endIdx)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to parse message ID"
						<< idStr;
				continue;
			}

			Account_->GetConnection ()->GetMessageInfo (id,
					[this, safeMsg, idStr] (const FullMessageInfo& msgInfo)
					{
						if (!safeMsg)
							return;

						auto body = safeMsg->GetBody ();

						const auto& id = "fwdstub_" + idStr;
						auto repl = "<div style='" + RepostDivStyle + "'>";
						repl += FullInfo2Replacement (msgInfo,
								Account_->GetCoreProxy (), FullInfoMode::Forward);
						repl += "</div>";

						PerformReplacements ({ { id, repl } }, body);

						safeMsg->SetBody (body);
					});
		}

		if (!contentsInfo.HasAdditionalInfo_)
			return;

		Account_->GetConnection ()->GetMessageInfo (msg->GetID (),
				[safeMsg] (const FullMessageInfo& msgInfo)
				{
					if (!safeMsg)
						return;

					const auto safeThis = qobject_cast<EntryBase*> (safeMsg->ParentCLEntry ());
					safeThis->HandleFullMessageInfo (msgInfo, safeMsg);
				});
	}

	void EntryBase::HandleFullMessageInfo (const FullMessageInfo& msgInfo, VkMessage *msg)
	{
		const auto& proxy = Account_->GetCoreProxy ();

		QList<QPair<QString, QString>> replacements;
		AppendReplacements (replacements, "photostub", msgInfo.Photos_, &Photo2Replacement);
		AppendReplacements (replacements, "pagestub", msgInfo.PagesPreviews_, &PagePreview2Replacement);
		AppendReplacements (replacements, "audiostub", msgInfo.Audios_,
				[proxy] (const AudioInfo& info) { return Audio2Replacement (info, proxy); });
		AppendReplacements (replacements, "videostub", msgInfo.Videos_,
				[proxy] (const VideoInfo& info) { return Video2Replacement (info, proxy); });
		AppendReplacements (replacements, "docstub", msgInfo.Documents_,
				[proxy] (const DocumentInfo& info) { return Document2Replacement (info, proxy); });
		AppendReplacements (replacements, "wallstub", msgInfo.ContainedReposts_,
				[proxy] (const FullMessageInfo& info) { return FullInfo2Replacement (info, proxy, FullInfoMode::Repost); });

		auto body = msg->GetBody ();
		PerformReplacements (replacements, body);
		msg->SetBody (body);
	}

	void EntryBase::PerformReplacements (QList<QPair<QString, QString>> replacements, QString& body)
	{
		QString js;

		for (auto& pair : replacements)
		{
			body.replace ("<div id='" + pair.first + "'></div>",
					"<div>" + pair.second + "</div>");

			pair.second.replace ('\n', "<br/>");
			pair.second.replace ('\\', "\\\\");
			pair.second.replace ('"', "\\\"");

			js += QString ("try { document.getElementById('%1').innerHTML = \"%2\"; } catch (e) { console.log(e); };")
					.arg (pair.first)
					.arg (pair.second);
		}

		emit performJS (js);
	}
}
}
}
