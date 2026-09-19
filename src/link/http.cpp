/**
 * Taiga
 * Copyright (C) 2010-2026, Eren Okka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "http.hpp"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <map>

#include "base/log.hpp"
#include "media/anime_db.hpp"
#include "media/anime_utils.hpp"
#include "taiga/network.hpp"
#include "taiga/settings.hpp"

namespace link::http {

using namespace Qt::StringLiterals;

namespace {

// The fields v1's default format uses. Everything is percent-encoded, because the result is a
// request body.
QString formatBody(const QString& format, const track::Episode& episode) {
  const auto item = anime::db.item(episode.animeId());
  const auto entry = anime::db.entry(episode.animeId());

  const auto number = [&episode]() -> QString {
    const auto range = episode.episodeNumberRange();
    return range ? QString::number(range->second) : QString{};
  }();

  const std::map<QString, QString> fields{
      {u"%title%"_s, item ? QString::fromStdString(anime::preferredTitle(*item))
                          : QString::fromStdString(episode.element(anitomy::ElementKind::Title))},
      {u"%episode%"_s, number},
      {u"%total%"_s,
       item && item->episode_count > 0 ? QString::number(item->episode_count) : QString{}},
      {u"%score%"_s, entry && entry->score ? QString::number(entry->score) : QString{}},
      {u"%image%"_s, item ? QString::fromStdString(item->image_url) : QString{}},
      {u"%group%"_s, QString::fromStdString(episode.element(anitomy::ElementKind::ReleaseGroup))},
      {u"%watched%"_s, entry ? QString::number(entry->watched_episodes) : QString{}},
  };

  auto result = format;

  for (const auto& [name, value] : fields) {
    result.replace(name, QString::fromUtf8(QUrl::toPercentEncoding(value)));
  }

  return result;
}

}  // namespace

void announce(const track::Episode& episode) {
  if (!taiga::settings.httpShareEnabled()) return;

  const auto url = QString::fromStdString(taiga::settings.httpShareUrl());

  if (url.isEmpty()) return;

  const auto body = formatBody(QString::fromStdString(taiga::settings.httpShareFormat()), episode);

  QNetworkRequest request{QUrl{url}};
  request.setHeader(QNetworkRequest::ContentTypeHeader, u"application/x-www-form-urlencoded"_s);

  const auto reply = taiga::network()->post(request, body.toUtf8());

  QObject::connect(reply, &QNetworkReply::finished, reply, [reply]() {
    if (reply->error() != QNetworkReply::NoError) {
      qWarning() << "Could not announce over HTTP:" << reply->errorString();
    }
    reply->deleteLater();
  });
}

}  // namespace link::http
