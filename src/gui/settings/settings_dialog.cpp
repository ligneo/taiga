/**
 * Taiga
 * Copyright (C) 2010-2024, Eren Okka
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

#include "settings_dialog.hpp"

#include "base/string.hpp"
#include "gui/settings/settings_accounts_page.hpp"
#include "gui/settings/settings_advanced_page.hpp"
#include "gui/settings/settings_anime_list_page.hpp"
#include "gui/settings/settings_application_page.hpp"
#include "gui/settings/settings_library_page.hpp"
#include "gui/settings/settings_media_players_page.hpp"
#include "gui/settings/settings_recognition_page.hpp"
#include "gui/settings/settings_streaming_page.hpp"
#include "gui/utils/theme.hpp"
#include "ui_settings_dialog.h"

#ifdef Q_OS_WINDOWS
#include "gui/platforms/windows.hpp"
#endif

namespace gui {

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), ui_(new Ui::SettingsDialog) {
  ui_->setupUi(this);

#ifdef Q_OS_WINDOWS
  enableMicaBackground(this);
#endif

  ui_->treeWidget->setIndentation(22);

  const auto add_item = [this](QString icon, QString text) {
    auto item = new QTreeWidgetItem(ui_->treeWidget, QStringList(text));
    item->setIcon(0, theme.getIcon(icon));
    item->setSizeHint(0, QSize{0, 24});
    return item;
  };

  const auto add_child = [this](QTreeWidgetItem* parent, QString text) {
    return new QTreeWidgetItem(parent, QStringList(text));
  };

  const auto accountsItem = add_item("account_circle", "Accounts");
  const auto applicationItem = add_item("web_asset", "Application");
  const auto animeListItem = add_item("list_alt", "Anime List");
  const auto libraryItem = add_item("folder", "Library");
  QTreeWidgetItem* recognitionItem = nullptr;
  QTreeWidgetItem* mediaPlayersItem = nullptr;
  QTreeWidgetItem* streamingItem = nullptr;
  {
    recognitionItem = add_item("check_circle", "Recognition");
    mediaPlayersItem = add_child(recognitionItem, "Media players");
    streamingItem = add_child(recognitionItem, "Streaming");
  }
  {
    auto item = add_item("share", "Sharing");
    add_child(item, "Discord");
    add_child(item, "HTTP");
    add_child(item, "mIRC");
  }
  {
    auto item = add_item("rss_feed", "Torrents");
    add_child(item, "Downloads");
    add_child(item, "Filters");
  }
  QTreeWidgetItem* advancedItem = nullptr;
  {
    advancedItem = add_item("warning", "Advanced");
    add_child(advancedItem, "Cache");
  }

  ui_->treeWidget->expandAll();

  addPage(accountsItem, new AccountsPage(this));
  addPage(applicationItem, new ApplicationPage(this));
  addPage(animeListItem, new AnimeListPage(this));
  addPage(libraryItem, new LibraryPage(this));
  addPage(recognitionItem, new RecognitionPage(this));
  addPage(mediaPlayersItem, new MediaPlayersPage(this));
  addPage(streamingItem, new StreamingPage(this));
  addPage(advancedItem, new AdvancedPage(this));

  connect(ui_->treeWidget, &QTreeWidget::currentItemChanged, this,
          [this](QTreeWidgetItem* current, QTreeWidgetItem*) {
            if (current) {
              auto text = current->text(0);
              if (current->parent()) {
                text = u"%1 / %2"_s.arg(current->parent()->text(0), text);
              }
              ui_->titleLabel->setText(text);

              // Pages that are not implemented yet share the placeholder at index 0
              ui_->stackedWidget->setCurrentIndex(current->data(0, Qt::UserRole).toInt());
            }
          });

  ui_->treeWidget->setCurrentItem(accountsItem);
}

void SettingsDialog::accept() {
  for (const auto page : pages_) {
    page->save();
  }

  QDialog::accept();
}

void SettingsDialog::addPage(QTreeWidgetItem* item, SettingsPage* page) {
  const auto index = ui_->stackedWidget->addWidget(page);
  item->setData(0, Qt::UserRole, index);
  page->load();
  pages_.push_back(page);
}

void SettingsDialog::show(QWidget* parent) {
  auto dlg = new SettingsDialog(parent);
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  dlg->setModal(true);
  dlg->QDialog::show();
}

}  // namespace gui
