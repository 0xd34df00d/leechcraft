/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverinfowidget.h"
#include <algorithm>
#include <tuple>
#include <QCheckBox>
#include <util/sll/statichash.h>
#include <util/sll/qtutil.h>
#include "ircserverclentry.h"
#include "ircserverhandler.h"

namespace LC::Azoth::Acetamide
{
	namespace
	{
		QWidget* MakeWidget (const QVariant& value)
		{
			switch (value.type ())
			{
			case QVariant::ByteArray:
			{
				auto edit = new QLineEdit;
				edit->setText (value.toString ());
				edit->setReadOnly (true);
				return edit;
			}
			case QVariant::Bool:
			{
				auto box = new QCheckBox;
				box->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
				box->setEnabled (false);
				return box;
			}
			default:
				qWarning () << "unknown RPLI type"
						<< value;
				return nullptr;
			}
		}

		QString Translate (std::string_view);
	}

	ServerInfoWidget::ServerInfoWidget (IrcServerCLEntry *isEntry , QWidget *parent)
	: QWidget (parent)
	, ISCLEntry_ (isEntry)
	{
		Ui_.setupUi (this);

		Ui_.ChanModesA_->setToolTip (Translate ("CHANMODES_A"));
		Ui_.ChanModesB_->setToolTip (Translate ("CHANMODES_B"));
		Ui_.ChanModesC_->setToolTip (Translate ("CHANMODES_C"));
		Ui_.ChanModesD_->setToolTip (Translate ("CHANMODES_D"));
		Ui_.PrefixTable_->setToolTip (Translate ("PREFIX"));

		auto isupport = ISCLEntry_->GetIrcServerHandler ()->GetISupport ();
		HandleSpecial (isupport);

		std::vector<std::pair<QByteArray, QVariant>> pairs { isupport.keyValueBegin (), isupport.keyValueEnd () };
		std::sort (pairs.begin (), pairs.end (),
				[] (const auto& l, const auto& r)
				{
					return std::make_tuple (r.second.type (), l.first) < std::make_tuple (l.second.type (), r.first);
				});
		for (const auto& [key, value] : pairs)
		{
			const auto widget = MakeWidget (value);
			if (!widget)
				continue;

			widget->setToolTip (Translate (Util::AsStringView (key)));
			Ui_.BaseParamsLayout_->addRow (key, widget);
		}
	}

	void ServerInfoWidget::HandleSpecial (QHash<QByteArray, QVariant>& supports)
	{
		if (auto chantypes = supports.take ("CHANTYPES");
			!chantypes.isNull ())
			Ui_.ChanTypes_->setText (chantypes.toString ());

		if (auto chanmodes = supports.take ("CHANMODES");
			!chanmodes.isNull ())
		{
			const auto& split = chanmodes.toByteArray ().split (',');
			Ui_.ChanModesA_->setText (split.value (0));
			Ui_.ChanModesB_->setText (split.value (1));
			Ui_.ChanModesC_->setText (split.value (2));
			Ui_.ChanModesD_->setText (split.value (3));
		}

		if (auto prefix = supports.take ("PREFIX");
			!prefix.isNull ())
			SetPrefix (prefix.toByteArray ());

		if (auto targmax = supports.take ("TARGMAX");
			!targmax.isNull ())
			SetTargMax (targmax.toByteArray ());
	}

	void ServerInfoWidget::SetPrefix (const QByteArray& str)
	{
		const int mode_end = str.indexOf (')');
		const auto& modeStr = str.mid (1, mode_end - 1);
		const auto& prefixStr = str.mid (mode_end + 1);
		const int rowCount = qMin (modeStr.length (), prefixStr.length ());

		if (modeStr.length () != prefixStr.length ())
			qWarning () << "number of modes is not equal to number of prefixes";

		Ui_.PrefixTable_->setRowCount (rowCount);

		for (int i = 0; i < rowCount; ++i)
		{
			Ui_.PrefixTable_->setItem (i, 0, new QTableWidgetItem (modeStr [i]));
			Ui_.PrefixTable_->setItem (i, 1, new QTableWidgetItem (prefixStr [i]));
		}
	}

	void ServerInfoWidget::SetTargMax (const QByteArray& str)
	{
		const auto& list = str.split (',');

		Ui_.TargetMax_->setRowCount (list.count ());

		int row = 0;
		for (const auto& param : list)
		{
			const int index = param.indexOf (':');
			Ui_.TargetMax_->setItem (row, 0, new QTableWidgetItem (QString { param.mid (0, index) }));
			Ui_.TargetMax_->setItem (row, 1, new QTableWidgetItem (QString { param.mid (index + 1) }));
			++row;
		}
	}

	namespace
	{
		using Util::KVPair;

		constexpr auto Tooltips = Util::MakeStringHash<const char*> (
				// Strings
				KVPair
				{
					"CASEMAPPING",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Case mapping used for nick- and channel name comparing. Current possible values:\n"
						"ascii: The chars [a-z] are lowercase of [A-Z].\n"
						"rfc1459: ascii with additional {}|~ the lowercase of []^.\n"
						"strict-rfc1459: ascii with additional {}| the lowercase of [].\n"
						"Note: RFC1459 forgot to mention the ~ and ^ although in all known implementations those are considered equivalent too.")
				},
				KVPair
				{
					"CHANLIMIT",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Maximum number of channels allowed to join by channel prefix.")
				},
				KVPair
				{
					"CHANNELLEN",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Maximum channel name length.")
				},
				KVPair
				{
					"IDCHAN",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Indicates the existence of \"safe\" channels as described in RFC 2811, and the length of the \"id\" portion of those channel names.\n"
						"[Example: IDCHAN=!:5 means the client should expect IDs which are 5 characters in length on \"!\" channels; "
						"for example  \"!JNB4Sircd\", where \"JNB4S\" is the ID and \"ircd\" is the channel's short name.]")
				},
				KVPair
				{
					"KICKLEN",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Maximum kick comment length.")
				},
				KVPair
				{
					"MAXLIST",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Maximum number entries in the list per mode. \n"
						"[Example: Given \"b:25,eI:50\", it would be possible to set up to 25 \"+b\" modes, "
						"and up to 50 of a combination of \"+e\" and \"+I\"  modes, e.g. 30 \"+e\" and 20 \"+I\" modes, making up a total of 50.]")
				},
				KVPair
				{
					"MODES",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Maximum number of channel modes with parameter allowed per MODE command.")
				},
				KVPair
				{
					"NETWORK",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"The IRC network name.")
				},
				KVPair
				{
					"NICKLEN",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Maximum nickname length.")
				},
				KVPair
				{
					"STD",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Indicates which form(s) of the ISUPPORT numeric are used by the server.")
				},
				KVPair
				{
					"STATUSMSG",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"The server supports a method of sending a NOTICE message to only those people on a channel with the specified status. "
						"This is done via a NOTICE command, with the channel prefixed by the desired status flag as the target.\n"
						"[Example: NOTICE @#channel :Hi there]")
				},
				KVPair
				{
					"TOPICLEN",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Maximum topic length.")
				},

				// Booleans
				KVPair
				{
					"EXCEPTS",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Indicates that the server supports \"ban exceptions\" (channel mode +e), as defined in RFC 2811, section 4.3.1")
				},
				KVPair
				{
					"SAFELIST",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Indicates that the client may request a \"LIST\" command from the server, "
						"without being disconnected due to the large amount of data generated by the command.")
				},


				// Chanmodes
				KVPair
				{
					"CHANMODES_A",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Mode that adds or removes a nick or address to a list. Always has a parameter.")
				},
				KVPair
				{
					"CHANMODES_B",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Mode that changes a setting and always has a parameter.")
				},
				KVPair
				{
					"CHANMODES_C",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Mode that changes a setting and only has a parameter when set.")
				},
				KVPair
				{
					"CHANMODES_D",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"Mode that changes a setting and never has a parameter.")
				},
				KVPair
				{
					"PREFIX",
					QT_TRANSLATE_NOOP ("LC::Azoth::Acetamide::ServerInfoWidget",
						"A list of channel modes a person can get and the respective prefix a channel or nickname will get in case the person has it. "
						"The order of the modes goes from most powerful to least powerful. "
						"Those prefixes are shown in the output of the WHOIS, WHO and NAMES command. "
						"[Example: (ab)&* maps the channel mode 'a' to the channel status flag '&', and channel mode 'b' to the channel status flag '*'.]\n"
						"[Example: PREFIX=(ohv)@%+ maps channel mode 'o' to status '@', 'h' to status '%', and 'v' to status +.]")
				}
		);

		QString Translate (std::string_view str)
		{
			return ServerInfoWidget::tr (Tooltips (str, str.data ()));
		}
	}

	void ServerInfoWidget::accept ()
	{
	}
}
