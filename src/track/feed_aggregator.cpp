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

#include "base/log.hpp"
#include "base/string.hpp"
#include "taiga/settings.hpp"

namespace track {

Aggregator::Aggregator(QObject* parent) : QObject(parent) {}

void Aggregator::fetch() {
  if (fetching_) return;

  const auto url = QString::fromStdString(taiga::settings.torrentDiscoveryUrl());

  if (url.isEmpty()) {
    emit errorOccurred(tr("No feed address is set."));
    return;
  }

  fetching_ = true;
  emit fetchingChanged(true);

  QNetworkRequest request{QUrl{url}};
  request.setHeaders(taiga::NetworkAccessManager::commonHeaders());

  manager_.get(request, this, [this](QRestReply& reply) {
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
  });
}

bool Aggregator::fetching() const {
  return fetching_;
}

const Feed& Aggregator::feed() const {
  return feed_;
}

}  // namespace track
