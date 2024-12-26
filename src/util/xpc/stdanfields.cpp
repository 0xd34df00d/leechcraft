/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stdanfields.h"
#include <QMap>
#include <QList>
#include <interfaces/an/ianemitter.h>
#include <interfaces/an/constants.h>

namespace LC::Util
{
	QList<AN::FieldData> GetStdANFields (const QString& type)
	{
		static const QMap<QString, QList<AN::FieldData>> values
		{
			{
				AN::TypeIMEventTuneChange,
				{
					{
						AN::Field::MediaTitle,
						QObject::tr ("Track title"),
						QObject::tr ("The track that the contact is currently listening to."),
						QVariant::String,
						{
							AN::TypeIMEventTuneChange
						}
					},
					{
						AN::Field::MediaAlbum,
						QObject::tr ("Track album"),
						QObject::tr ("The album that the contact is currently listening to."),
						QVariant::String,
						{
							AN::TypeIMEventTuneChange
						}
					},
					{
						AN::Field::MediaArtist,
						QObject::tr ("Track artist"),
						QObject::tr ("The artist the contact is currently listening to."),
						QVariant::String,
						{
							AN::TypeIMEventTuneChange
						}
					},
					{
						AN::Field::MediaLength,
						QObject::tr ("Track length"),
						QObject::tr ("Length of the track the contact is currently listening to."),
						QVariant::Int,
						{
							AN::TypeIMEventTuneChange
						}
					}
				}
			},
			{
				AN::TypeIMEventActivityChange,
				{
					{
						AN::Field::IMActivityGeneral,
						QObject::tr ("General activity"),
						QObject::tr ("General activity of the contact."),
						QVariant::String,
						{
							AN::TypeIMEventActivityChange
						}
					},
					{
						AN::Field::IMActivitySpecific,
						QObject::tr ("Specific activity"),
						QObject::tr ("Specific activity of the contact within the given general activity."),
						QVariant::String,
						{
							AN::TypeIMEventActivityChange
						}
					},
					{
						AN::Field::IMActivityText,
						QObject::tr ("Activity text"),
						QObject::tr ("The comment set by the contact."),
						QVariant::String,
						{
							AN::TypeIMEventActivityChange
						}
					}
				}
			},
			{
				AN::TypeIMEventMoodChange,
				{
					{
						AN::Field::IMMoodGeneral,
						QObject::tr ("Mood"),
						QObject::tr ("The mood of the contact."),
						QVariant::String,
						{
							AN::TypeIMEventMoodChange
						}
					},
					{
						AN::Field::IMMoodText,
						QObject::tr ("Mood explanation text"),
						QObject::tr ("The text accompanying the mood set by the contact."),
						QVariant::String,
						{
							AN::TypeIMEventMoodChange
						}
					}
				}
			},
			{
				AN::TypeIMEventLocationChange,
				{
					{
						AN::Field::IMLocationLatitude,
						QObject::tr ("Latitude"),
						QObject::tr ("The latitude of the contact's current position."),
						QVariant::Double,
						{
							AN::TypeIMEventLocationChange
						}
					},
					{
						AN::Field::IMLocationLongitude,
						QObject::tr ("Longitude"),
						QObject::tr ("The longitude of the contact's current position."),
						QVariant::Double,
						{
							AN::TypeIMEventLocationChange
						}
					},
					{
						AN::Field::IMLocationCountry,
						QObject::tr ("Country"),
						QObject::tr ("The country the contact is currently in."),
						QVariant::String,
						{
							AN::TypeIMEventLocationChange
						}
					},
					{
						AN::Field::IMLocationLocality,
						QObject::tr ("Locality"),
						QObject::tr ("The the exact locality (like a city or a town) the contact is currently in."),
						QVariant::String,
						{
							AN::TypeIMEventLocationChange
						}
					}
				}
			},
			{
				AN::CatMediaPlayer,
				{
					{
						AN::Field::MediaPlayerURL,
						QObject::tr ("File URL"),
						QObject::tr ("URL to the file being played."),
						QVariant::Url,
						{
							AN::TypeMediaPlaybackStatus
						}
					},
					{
						AN::Field::MediaPlaybackStatus,
						QObject::tr ("Playback status"),
						QObject::tr ("The status of the currently playing media file."),
						QVariant::String,
						{
							AN::TypeMediaPlaybackStatus
						},
						{
							"Stopped",
							"Paused",
							"Playing"
						}
					},
					{
						AN::Field::MediaTitle,
						QObject::tr ("Title metadata"),
						QObject::tr ("Title of the track being played."),
						QVariant::String,
						{
							AN::TypeMediaPlaybackStatus
						}
					},
					{
						AN::Field::MediaAlbum,
						QObject::tr ("Album metadata"),
						QObject::tr ("Album of the track being played."),
						QVariant::String,
						{
							AN::TypeMediaPlaybackStatus
						}
					},
					{
						AN::Field::MediaArtist,
						QObject::tr ("Artist metadata"),
						QObject::tr ("Artist of the track being played."),
						QVariant::String,
						{
							AN::TypeMediaPlaybackStatus
						}
					},
					{
						AN::Field::MediaLength,
						QObject::tr ("Length metadata"),
						QObject::tr ("Length of the track being played."),
						QVariant::Int,
						{
							AN::TypeMediaPlaybackStatus
						}
					}
				}
			},
			{
				AN::CatNews,
				{
					{
						AN::Field::NewsSourceName,
						QObject::tr ("News source name"),
						QObject::tr ("The name of the news source that caused the notification."),
						QVariant::String,
						{
							AN::TypeNewsSourceBroken,
							AN::TypeNewsSourceUpdated
						}
					},
					{
						AN::Field::NewsSourceURL,
						QObject::tr ("News source URL"),
						QObject::tr ("The URL of the news source that caused the notification."),
						QVariant::String,
						{
							AN::TypeNewsSourceBroken,
							AN::TypeNewsSourceUpdated
						}
					}
				}
			},
			{
				AN::CatTerminal,
				{
					{
						AN::Field::TerminalActive,
						QObject::tr ("Terminal is active"),
						QObject::tr ("The terminal tab that caused the notification is active."),
						QVariant::Bool,
						{
							AN::TypeTerminalBell,
							AN::TypeTerminalActivity,
							AN::TypeTerminalInactivity
						}
					}
				}
			}
		};

		if (!type.isEmpty ())
			return values.value (type);

		QList<AN::FieldData> result;
		for (const auto& list : values)
			result << list;
		return result;
	}
}
