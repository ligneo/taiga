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

#include "settings_application_page.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>

#include "gui/utils/theme.hpp"
#include "taiga/settings.hpp"

namespace gui {

ApplicationPage::ApplicationPage(QWidget* parent)
    : SettingsPage(parent), m_comboColorScheme(new QComboBox(this)) {
  const auto layout = new QVBoxLayout(this);

  // Appearance
  {
    const auto group = new QGroupBox(tr("Appearance"), this);
    const auto form = new QFormLayout(group);

    m_comboColorScheme->addItem(tr("System default"), static_cast<int>(Qt::ColorScheme::Unknown));
    m_comboColorScheme->addItem(tr("Light"), static_cast<int>(Qt::ColorScheme::Light));
    m_comboColorScheme->addItem(tr("Dark"), static_cast<int>(Qt::ColorScheme::Dark));
    form->addRow(tr("Color scheme:"), m_comboColorScheme);

    layout->addWidget(group);
  }

  layout->addStretch();
}

void ApplicationPage::load() {
  const auto scheme = static_cast<int>(taiga::settings.appColorScheme());
  m_comboColorScheme->setCurrentIndex(m_comboColorScheme->findData(scheme));
}

void ApplicationPage::save() {
  const auto scheme = m_comboColorScheme->currentData().value<Qt::ColorScheme>();
  if (scheme == taiga::settings.appColorScheme()) return;

  taiga::settings.setAppColorScheme(scheme);
  theme.applyStyle();
}

}  // namespace gui
