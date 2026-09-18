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

#pragma once

#include <QAbstractListModel>

namespace track {
struct FeedItem;
}

namespace gui {

enum class TorrentItemDataRole {
  FeedItem = Qt::UserRole,
  SortValue,
};

class TorrentModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TorrentModel)

public:
  enum Column {
    COLUMN_TITLE,
    COLUMN_EPISODE,
    COLUMN_GROUP,
    COLUMN_SIZE,
    COLUMN_VIDEO,
    COLUMN_SEEDERS,
    COLUMN_LEECHERS,
    COLUMN_DOWNLOADS,
    COLUMN_DESCRIPTION,
    COLUMN_FILENAME,
    COLUMN_DATE,
    NUM_COLUMNS,
  };

  TorrentModel(QObject* parent);
  ~TorrentModel() override = default;

  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  const track::FeedItem* itemAt(const QModelIndex& index) const;
};

}  // namespace gui
