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

#include "torrents_widget.hpp"

#include <QClipboard>
#include <QCursor>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QHeaderView>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QUrl>

#include "base/log.hpp"
#include "base/string.hpp"
#include "gui/main/main_window.hpp"
#include "gui/models/torrent_model.hpp"
#include "gui/utils/theme.hpp"
#include "track/feed.hpp"
#include "track/feed_aggregator.hpp"

namespace gui {

TorrentsWidget::TorrentsWidget(QWidget* parent)
    : PageWidget{parent},
      m_model(new TorrentModel(this)),
      m_proxyModel(new QSortFilterProxyModel(this)),
      m_view(new QTreeView(this)) {
  m_proxyModel->setSourceModel(m_model);
  m_proxyModel->setSortRole(static_cast<int>(TorrentItemDataRole::SortValue));
  m_proxyModel->setFilterKeyColumn(TorrentModel::COLUMN_TITLE);
  m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

  m_view->setObjectName("torrentsView");
  m_view->setFrameShape(QFrame::Shape::NoFrame);
  m_view->setModel(m_proxyModel);
  m_view->setAlternatingRowColors(true);
  m_view->setAllColumnsShowFocus(true);
  m_view->setContextMenuPolicy(Qt::CustomContextMenu);
  m_view->setRootIsDecorated(false);
  m_view->setSortingEnabled(true);
  m_view->setUniformRowHeights(true);
  m_view->sortByColumn(TorrentModel::COLUMN_DATE, Qt::SortOrder::DescendingOrder);

  m_view->header()->setSectionsMovable(false);
  m_view->header()->setStretchLastSection(false);
  m_view->header()->setTextElideMode(Qt::ElideRight);
  m_view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_view->header()->setSectionResizeMode(TorrentModel::COLUMN_TITLE, QHeaderView::Stretch);

  layout()->addWidget(m_view);

  initToolbar();

  connect(m_view, &QWidget::customContextMenuRequested, this, &TorrentsWidget::showContextMenu);
  connect(m_view, &QTreeView::doubleClicked, this, &TorrentsWidget::openItemLink);

  connect(mainWindow()->searchBox(), &QLineEdit::textChanged, this, &TorrentsWidget::setFilterText);
  setFilterText(mainWindow()->searchBox()->text());

  connect(track::aggregator(), &track::Aggregator::fetchingChanged, this,
          [this](bool fetching) { m_actionRefresh->setEnabled(!fetching); });
  connect(track::aggregator(), &track::Aggregator::errorOccurred, this,
          [](const QString& message) { qCritical() << message; });

  if (track::aggregator()->feed().items.empty()) track::aggregator()->fetch();
}

void TorrentsWidget::initToolbar() {
  m_actionRefresh = new QAction(theme.getIcon("sync"), tr("Refresh"), this);
  connect(m_actionRefresh, &QAction::triggered, this, []() { track::aggregator()->fetch(); });
  m_toolbar->addAction(m_actionRefresh);
}

void TorrentsWidget::setFilterText(const QString& text) {
  m_proxyModel->setFilterFixedString(text);
}

void TorrentsWidget::openItemLink(const QModelIndex& index) const {
  const auto item = m_model->itemAt(m_proxyModel->mapToSource(index));

  if (!item) return;

  // Opening the item in a browser is the only thing that can be done until downloads are handled.
  const auto link = !item->info_link.empty() ? item->info_link : item->link;

  if (!link.empty()) QDesktopServices::openUrl(QUrl{QString::fromStdString(link)});
}

void TorrentsWidget::showContextMenu() {
  const auto index = m_view->currentIndex();

  if (!index.isValid()) return;

  const auto item = m_model->itemAt(m_proxyModel->mapToSource(index));

  if (!item) return;

  auto* menu = new QMenu(m_view);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  menu->addAction(tr("Open in browser"), this, [this, index]() { openItemLink(index); });

  if (!item->magnet_link.empty()) {
    const auto link = QString::fromStdString(item->magnet_link);
    menu->addAction(tr("Copy magnet link"), this,
                    [link]() { QGuiApplication::clipboard()->setText(link); });
  }

  menu->popup(QCursor::pos());
}

}  // namespace gui
