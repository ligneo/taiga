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

#include "settings_anime_list_page.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>

#include "media/anime.hpp"
#include "taiga/settings.hpp"

namespace gui {

AnimeListPage::AnimeListPage(QWidget* parent)
    : SettingsPage(parent), m_comboTitleLanguage(new QComboBox(this)) {
  const auto layout = new QVBoxLayout(this);

  // Appearance
  {
    const auto group = new QGroupBox(tr("Appearance"), this);
    const auto form = new QFormLayout(group);

    m_comboTitleLanguage->addItem(tr("Romaji"), static_cast<int>(anime::TitleLanguage::Romaji));
    m_comboTitleLanguage->addItem(tr("English"), static_cast<int>(anime::TitleLanguage::English));
    m_comboTitleLanguage->addItem(tr("Native"), static_cast<int>(anime::TitleLanguage::Native));
    form->addRow(tr("Title language preference:"), m_comboTitleLanguage);

    layout->addWidget(group);
  }

  layout->addStretch();
}

void AnimeListPage::load() {
  const auto language = static_cast<int>(taiga::settings.titleLanguage());
  m_comboTitleLanguage->setCurrentIndex(m_comboTitleLanguage->findData(language));
}

void AnimeListPage::save() {
  const auto language =
      static_cast<anime::TitleLanguage>(m_comboTitleLanguage->currentData().toInt());
  taiga::settings.setTitleLanguage(language);
}

}  // namespace gui
