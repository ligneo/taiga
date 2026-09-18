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

#include "feed_aggregator.hpp"

#include <QNetworkRequest>
#include <QRestReply>
#include <QUrl>
#include <map>
#include <set>

#include "base/log.hpp"
#include "base/string.hpp"
#include "media/anime_db.hpp"
#include "media/anime_utils.hpp"
#include "taiga/settings.hpp"

namespace track {

Aggregator::Aggregator(QObject* parent) : QObject(parent) {
  timer_.setSingleShot(false);
  connect(&timer_, &QTimer::timeout, this, [this]() { fetch({}, true); });
  applyAutoCheckSettings();
}

void Aggregator::applyAutoCheckSettings() {
  const auto interval = taiga::settings.torrentAutoCheckInterval();

  if (!taiga::settings.torrentAutoCheckEnabled() || interval.count() < 1) {
    timer_.stop();
    return;
  }

  timer_.start(interval);
}

std::chrono::milliseconds Aggregator::timeUntilNextCheck() const {
  if (!timer_.isActive()) return std::chrono::milliseconds::zero();
  return std::chrono::milliseconds{timer_.remainingTime()};
}

void Aggregator::search(const QString& title) {
  auto url = QString::fromStdString(taiga::settings.torrentSearchUrl());

  // v1 substitutes the encoded title, so that a title with spaces or symbols stays a valid URL.
  url.replace(u"%title%"_s, QString::fromUtf8(QUrl::toPercentEncoding(title)));

  fetch(url);
}

void Aggregator::fetch(const QString& requestedUrl, const bool automatic) {
  if (fetching_) return;

  const auto url = !requestedUrl.isEmpty()
                       ? requestedUrl
                       : QString::fromStdString(taiga::settings.torrentDiscoveryUrl());

  if (url.isEmpty()) {
    emit errorOccurred(tr("No feed address is set."));
    return;
  }

  fetching_ = true;
  emit fetchingChanged(true);

  QNetworkRequest request{QUrl{url}};
  request.setHeaders(taiga::NetworkAccessManager::commonHeaders());

  manager_.get(request, this, [this, automatic](QRestReply& reply) {
    fetching_ = false;
    emit fetchingChanged(false);

    if (!reply.isHttpStatusSuccess() || reply.hasError()) {
      emit errorOccurred(reply.errorString());
      return;
    }

    const auto feed = parseFeed(QString::fromUtf8(reply.readBody()));

    if (!feed) {
      emit errorOccurred(tr("Could not read the feed."));
      return;
    }

    feed_ = *feed;
    examineFeed(feed_);
    emit feedChanged();

    // Only an automatic check notifies, so that refreshing by hand stays quiet. As in v1.
    if (automatic && taiga::settings.torrentNotifyNewEpisodes()) {
      if (const auto lines = newEpisodeLines(); !lines.isEmpty()) {
        emit newEpisodesFound(lines);
      }
    }
  });
}

// One line per anime, with the episode numbers that are new. v1 groups the same way.
QStringList Aggregator::newEpisodeLines() const {
  std::map<QString, std::set<int>> episodes;

  for (const auto& item : feed_.items) {
    if (!item.new_episode) continue;

    const auto anime = anime::db.item(item.episode.animeId());
    const auto title =
        anime ? QString::fromStdString(anime::preferredTitle(*anime))
              : QString::fromStdString(item.episode.element(anitomy::ElementKind::Title));

    if (const auto range = item.episode.episodeNumberRange()) {
      episodes[title].insert(range->second);
    }
  }

  QStringList lines;

  for (const auto& [title, numbers] : episodes) {
    QStringList parts;
    for (const auto number : numbers) parts << u"#%1"_s.arg(number);
    lines << u"\u00BB %1 %2"_s.arg(title, parts.join(u' '));
  }

  return lines;
}

bool Aggregator::fetching() const {
  return fetching_;
}

const Feed& Aggregator::feed() const {
  return feed_;
}

}  // namespace track
