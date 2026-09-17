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

#include "settings_library_page.hpp"

#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "taiga/settings.hpp"

namespace gui {

LibraryPage::LibraryPage(QWidget* parent)
    : SettingsPage(parent),
      m_listFolders(new QListWidget(this)),
      m_buttonRemove(new QPushButton(tr("Remove"), this)) {
  const auto layout = new QVBoxLayout(this);

  // Library folders
  {
    const auto group = new QGroupBox(tr("Library folders"), this);
    const auto groupLayout = new QVBoxLayout(group);

    groupLayout->addWidget(
        new QLabel(tr("These folders will be scanned for available episodes."), group));

    m_listFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
    groupLayout->addWidget(m_listFolders);

    const auto buttonLayout = new QHBoxLayout();
    const auto buttonAdd = new QPushButton(tr("Add new..."), group);
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(m_buttonRemove);
    buttonLayout->addStretch();
    groupLayout->addLayout(buttonLayout);

    connect(buttonAdd, &QPushButton::clicked, this, &LibraryPage::addFolder);
    connect(m_buttonRemove, &QPushButton::clicked, this, &LibraryPage::removeFolder);
    connect(m_listFolders, &QListWidget::itemSelectionChanged, this,
            [this]() { m_buttonRemove->setEnabled(!m_listFolders->selectedItems().isEmpty()); });

    layout->addWidget(group);
  }
}

void LibraryPage::load() {
  m_listFolders->clear();
  for (const auto& folder : taiga::settings.libraryFolders()) {
    m_listFolders->addItem(QString::fromStdString(folder));
  }
  m_buttonRemove->setEnabled(false);
}

void LibraryPage::save() {
  std::vector<std::string> folders;
  for (int i = 0; i < m_listFolders->count(); ++i) {
    folders.push_back(m_listFolders->item(i)->text().toStdString());
  }
  taiga::settings.setLibraryFolders(folders);
}

void LibraryPage::addFolder() {
  constexpr auto options =
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::ReadOnly;

  const auto directory =
      QFileDialog::getExistingDirectory(this, tr("Add New Folder"), QDir::homePath(), options);
  if (directory.isEmpty()) return;

  const auto path = QDir::toNativeSeparators(QDir::cleanPath(directory));
  if (!m_listFolders->findItems(path, Qt::MatchExactly).isEmpty()) return;

  m_listFolders->addItem(path);
}

void LibraryPage::removeFolder() {
  qDeleteAll(m_listFolders->selectedItems());
}

}  // namespace gui
