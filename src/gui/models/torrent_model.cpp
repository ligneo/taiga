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

#include "torrent_model.hpp"

#include <QDateTime>
#include <QLocale>
#include <map>

#include "base/string.hpp"
#include "media/anime_db.hpp"
#include "media/anime_utils.hpp"
#include "track/episode.hpp"
#include "track/feed.hpp"
#include "track/feed_aggregator.hpp"

namespace gui {

namespace {

QDateTime parseDate(const track::FeedItem& item) {
  auto value = QString::fromStdString(item.pub_date).trimmed();

  // Qt only accepts a numeric zone offset, while RFC 2822 also allows the obsolete zone names that
  // some providers still use (e.g. Tokyo Toshokan publishes dates ending in `GMT`).
  static const std::map<QString, QString> zones{
      {u"UT"_s, u"+0000"_s},  {u"GMT"_s, u"+0000"_s}, {u"UTC"_s, u"+0000"_s},
      {u"EST"_s, u"-0500"_s}, {u"EDT"_s, u"-0400"_s}, {u"CST"_s, u"-0600"_s},
      {u"CDT"_s, u"-0500"_s}, {u"MST"_s, u"-0700"_s}, {u"MDT"_s, u"-0600"_s},
      {u"PST"_s, u"-0800"_s}, {u"PDT"_s, u"-0700"_s},
  };

  if (const auto pos = value.lastIndexOf(u' '); pos > 0) {
    if (const auto it = zones.find(value.mid(pos + 1).toUpper()); it != zones.end()) {
      value.replace(pos + 1, value.size() - pos - 1, it->second);
    }
  }

  auto date = QDateTime::fromString(value, Qt::RFC2822Date);
  if (!date.isValid()) date = QDateTime::fromString(value, Qt::ISODate);
  return date;
}

}  // namespace

TorrentModel::TorrentModel(QObject* parent) : QAbstractListModel(parent) {
  connect(track::aggregator(), &track::Aggregator::feedChanged, this, [this]() {
    beginResetModel();
    endResetModel();
  });
}

int TorrentModel::rowCount(const QModelIndex&) const {
  return track::aggregator()->feed().items.size();
}

int TorrentModel::columnCount(const QModelIndex&) const {
  return NUM_COLUMNS;
}

QVariant TorrentModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return {};

  const auto& item = track::aggregator()->feed().items.at(index.row());

  switch (role) {
    case Qt::DisplayRole: {
      switch (index.column()) {
        case COLUMN_TITLE: {
          // The matched anime is shown when there is one, so that different release names of the
          // same anime line up.
          if (const auto anime = anime::db.item(item.episode.animeId())) {
            return QString::fromStdString(anime::preferredTitle(*anime));
          }
          const auto title = item.episode.element(anitomy::ElementKind::Title);
          return QString::fromStdString(title.empty() ? item.title : title);
        }
        case COLUMN_EPISODE:
          return QString::fromStdString(item.episode.element(anitomy::ElementKind::Episode));
        case COLUMN_GROUP:
          return QString::fromStdString(item.episode.element(anitomy::ElementKind::ReleaseGroup));
        case COLUMN_VIDEO:
          return QString::fromStdString(
              item.episode.element(anitomy::ElementKind::VideoResolution));
        case COLUMN_SIZE:
          if (!item.file_size) return {};
          return QLocale::system().formattedDataSize(item.file_size, 1);
        case COLUMN_SEEDERS:
          return item.seeders ? QVariant(*item.seeders) : QVariant{};
        case COLUMN_LEECHERS:
          return item.leechers ? QVariant(*item.leechers) : QVariant{};
        case COLUMN_DOWNLOADS:
          return item.downloads ? QVariant(*item.downloads) : QVariant{};
        case COLUMN_DATE: {
          const auto date = parseDate(item);
          return date.isValid() ? QLocale::system().toString(date, QLocale::ShortFormat)
                                : QString{};
        }
        case COLUMN_DESCRIPTION:
          return QString::fromStdString(item.description);
        case COLUMN_FILENAME:
          return QString::fromStdString(item.title);
      }
      break;
    }

    case Qt::ToolTipRole:
      return QString::fromStdString(item.title);

    case Qt::TextAlignmentRole: {
      switch (index.column()) {
        case COLUMN_EPISODE:
        case COLUMN_SIZE:
        case COLUMN_SEEDERS:
        case COLUMN_LEECHERS:
        case COLUMN_DOWNLOADS:
        case COLUMN_DATE:
          return QVariant(Qt::AlignRight | Qt::AlignVCenter);
      }
      break;
    }

    case static_cast<int>(TorrentItemDataRole::FeedItem):
      return QVariant::fromValue(&item);

    // Columns that are displayed as text need a value that sorts in a sensible order.
    case static_cast<int>(TorrentItemDataRole::SortValue): {
      switch (index.column()) {
        case COLUMN_SIZE:
          return QVariant::fromValue(item.file_size);
        case COLUMN_DATE:
          return parseDate(item);
      }
      return data(index, Qt::DisplayRole);
    }
  }

  return {};
}

QVariant TorrentModel::headerData(int section, Qt::Orientation orientation, int role) const {
  switch (role) {
    case Qt::DisplayRole: {
      // clang-format off
      switch (section) {
        case COLUMN_TITLE: return tr("Anime title");
        case COLUMN_EPISODE: return tr("Episode");
        case COLUMN_GROUP: return tr("Group");
        case COLUMN_SIZE: return tr("Size");
        case COLUMN_VIDEO: return tr("Video");
        case COLUMN_SEEDERS: return tr("S");
        case COLUMN_LEECHERS: return tr("L");
        case COLUMN_DOWNLOADS: return tr("D");
        case COLUMN_DESCRIPTION: return tr("Description");
        case COLUMN_FILENAME: return tr("Filename");
        case COLUMN_DATE: return tr("Release date");
      }
      // clang-format on
      break;
    }

    case Qt::TextAlignmentRole: {
      switch (section) {
        case COLUMN_EPISODE:
        case COLUMN_SIZE:
        case COLUMN_SEEDERS:
        case COLUMN_LEECHERS:
        case COLUMN_DOWNLOADS:
        case COLUMN_DATE:
          return QVariant(Qt::AlignRight | Qt::AlignVCenter);
      }
      break;
    }
  }

  return QAbstractListModel::headerData(section, orientation, role);
}

const track::FeedItem* TorrentModel::itemAt(const QModelIndex& index) const {
  if (!index.isValid()) return nullptr;
  return &track::aggregator()->feed().items.at(index.row());
}

}  // namespace gui
