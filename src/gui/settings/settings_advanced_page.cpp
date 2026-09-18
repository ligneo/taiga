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

#include "settings_advanced_page.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QNetworkProxy>
#include <QSpinBox>
#include <QVBoxLayout>

#include "taiga/network.hpp"
#include "taiga/settings.hpp"

namespace gui {

AdvancedPage::AdvancedPage(QWidget* parent)
    : SettingsPage(parent),
      m_comboProxyType(new QComboBox(this)),
      m_editProxyHost(new QLineEdit(this)),
      m_spinProxyPort(new QSpinBox(this)),
      m_editProxyUsername(new QLineEdit(this)),
      m_editProxyPassword(new QLineEdit(this)) {
  const auto layout = new QVBoxLayout(this);

  // Proxy
  {
    const auto group = new QGroupBox(tr("Proxy"), this);
    const auto form = new QFormLayout(group);

    m_comboProxyType->addItem(tr("HTTP"), static_cast<int>(QNetworkProxy::HttpProxy));
    m_comboProxyType->addItem(tr("SOCKS5"), static_cast<int>(QNetworkProxy::Socks5Proxy));
    form->addRow(tr("Type:"), m_comboProxyType);

    form->addRow(tr("Host:"), m_editProxyHost);

    m_spinProxyPort->setRange(0, 65535);
    m_spinProxyPort->setSpecialValueText(tr("Default"));
    form->addRow(tr("Port:"), m_spinProxyPort);

    form->addRow(tr("Username:"), m_editProxyUsername);

    m_editProxyPassword->setEchoMode(QLineEdit::Password);
    form->addRow(tr("Password:"), m_editProxyPassword);

    layout->addWidget(group);
  }

  layout->addStretch();
}

void AdvancedPage::load() {
  const auto type = static_cast<int>(taiga::settings.proxyType());
  m_comboProxyType->setCurrentIndex(m_comboProxyType->findData(type));

  m_editProxyHost->setText(QString::fromStdString(taiga::settings.proxyHost()));
  m_spinProxyPort->setValue(std::max(taiga::settings.proxyPort(), 0));
  m_editProxyUsername->setText(QString::fromStdString(taiga::settings.proxyUsername()));
  m_editProxyPassword->setText(QString::fromStdString(taiga::settings.proxyPassword()));
}

void AdvancedPage::save() {
  taiga::settings.setProxyType(
      static_cast<QNetworkProxy::ProxyType>(m_comboProxyType->currentData().toInt()));
  taiga::settings.setProxyHost(m_editProxyHost->text().trimmed().toStdString());

  const auto port = m_spinProxyPort->value();
  taiga::settings.setProxyPort(port > 0 ? port : -1);

  taiga::settings.setProxyUsername(m_editProxyUsername->text().toStdString());
  taiga::settings.setProxyPassword(m_editProxyPassword->text().toStdString());

  taiga::network()->applyProxySettings();
}

}  // namespace gui
