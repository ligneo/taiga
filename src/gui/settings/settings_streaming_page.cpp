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

#include "settings_streaming_page.hpp"

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>

#include "gui/settings/settings_player_list.hpp"
#include "gui/settings/settings_stream_list.hpp"
#include "taiga/settings.hpp"

namespace gui {

StreamingPage::StreamingPage(QWidget* parent)
    : SettingsPage(parent),
      m_checkEnabled(new QCheckBox(tr("Enable streaming media detection"), this)),
      m_listPlayers(new PlayerListWidget(true, this)),
      m_listProviders(new StreamListWidget(this)) {
  const auto layout = new QVBoxLayout(this);

  layout->addWidget(m_checkEnabled);

  const auto label =
      new QLabel(tr("Tip: Select the web browsers you use, deselect the others."), this);
  label->setWordWrap(true);
  layout->addWidget(label);

  layout->addWidget(m_listPlayers);

  const auto providersLabel = new QLabel(tr("Supported media providers:"), this);
  layout->addWidget(providersLabel);

  layout->addWidget(m_listProviders);

  connect(m_checkEnabled, &QCheckBox::toggled, m_listPlayers, &QWidget::setEnabled);
  connect(m_checkEnabled, &QCheckBox::toggled, providersLabel, &QWidget::setEnabled);
  connect(m_checkEnabled, &QCheckBox::toggled, m_listProviders, &QWidget::setEnabled);
}

void StreamingPage::load() {
  m_checkEnabled->setChecked(taiga::settings.streamingMediaEnabled());
  m_listPlayers->setEnabled(m_checkEnabled->isChecked());
  m_listPlayers->load();
  m_listProviders->setEnabled(m_checkEnabled->isChecked());
  m_listProviders->load();
}

void StreamingPage::save() {
  taiga::settings.setStreamingMediaEnabled(m_checkEnabled->isChecked());
  m_listPlayers->save();
  m_listProviders->save();
}

}  // namespace gui
