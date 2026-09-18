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

#include "settings_torrents_page.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include "base/string.hpp"
#include "taiga/settings.hpp"
#include "track/feed_aggregator.hpp"

namespace gui {

namespace {

// The addresses v1 offers, so that a working feed is a click away.
const QStringList kSourceUrls{
    u"https://www.tokyotosho.info/rss.php?filter=1,11&zwnj=0"_s,
    u"https://nyaa.si/?page=rss&c=1_2&f=0"_s,
    u"https://subsplease.org/rss/?r=1080"_s,
};

const QStringList kSearchUrls{
    u"https://nyaa.si/?page=rss&c=1_2&f=0&q=%title%"_s,
    u"https://anidex.info/rss/?cat=1&lang_id=1&q=%title%"_s,
};

}  // namespace

TorrentsPage::TorrentsPage(QWidget* parent)
    : SettingsPage(parent),
      m_comboSource(new QComboBox(this)),
      m_comboSearch(new QComboBox(this)),
      m_checkAutoCheck(new QCheckBox(tr("Check new torrents automatically"), this)),
      m_spinInterval(new QSpinBox(this)),
      m_checkNotify(new QCheckBox(tr("Notify me when there are new episodes"), this)) {
  const auto layout = new QVBoxLayout(this);

  // Sources
  {
    const auto group = new QGroupBox(tr("Sources"), this);
    const auto groupLayout = new QVBoxLayout(group);

    m_comboSource->setEditable(true);
    m_comboSource->addItems(kSourceUrls);
    groupLayout->addWidget(new QLabel(tr("RSS feed for checking new releases:"), group));
    groupLayout->addWidget(m_comboSource);

    m_comboSearch->setEditable(true);
    m_comboSearch->addItems(kSearchUrls);
    groupLayout->addWidget(new QLabel(tr("RSS feed for searching releases for a title:"), group));
    groupLayout->addWidget(m_comboSearch);

    const auto note = new QLabel(tr("Note: %title% is replaced with the anime title."), group);
    note->setWordWrap(true);
    groupLayout->addWidget(note);

    layout->addWidget(group);
  }

  // Automation
  {
    const auto group = new QGroupBox(tr("Automation"), this);
    const auto groupLayout = new QVBoxLayout(group);
    const auto form = new QFormLayout();

    m_spinInterval->setRange(1, 1440);
    m_spinInterval->setSuffix(tr(" minutes"));
    form->addRow(tr("Interval:"), m_spinInterval);

    groupLayout->addWidget(m_checkAutoCheck);
    groupLayout->addLayout(form);
    groupLayout->addWidget(m_checkNotify);

    const auto note = new QLabel(
        tr("An episode counts as new when it belongs to an anime on your list and goes beyond what "
           "you have watched."),
        group);
    note->setWordWrap(true);
    groupLayout->addWidget(note);

    layout->addWidget(group);
  }

  layout->addStretch();

  connect(m_checkAutoCheck, &QCheckBox::toggled, m_spinInterval, &QWidget::setEnabled);
  connect(m_checkAutoCheck, &QCheckBox::toggled, m_checkNotify, &QWidget::setEnabled);
}

void TorrentsPage::load() {
  m_comboSource->setCurrentText(QString::fromStdString(taiga::settings.torrentDiscoveryUrl()));
  m_comboSearch->setCurrentText(QString::fromStdString(taiga::settings.torrentSearchUrl()));
  m_checkAutoCheck->setChecked(taiga::settings.torrentAutoCheckEnabled());
  m_spinInterval->setValue(static_cast<int>(taiga::settings.torrentAutoCheckInterval().count()));
  m_checkNotify->setChecked(taiga::settings.torrentNotifyNewEpisodes());
  m_spinInterval->setEnabled(m_checkAutoCheck->isChecked());
  m_checkNotify->setEnabled(m_checkAutoCheck->isChecked());
}

void TorrentsPage::save() {
  taiga::settings.setTorrentDiscoveryUrl(m_comboSource->currentText().trimmed().toStdString());
  taiga::settings.setTorrentSearchUrl(m_comboSearch->currentText().trimmed().toStdString());
  taiga::settings.setTorrentAutoCheckEnabled(m_checkAutoCheck->isChecked());
  taiga::settings.setTorrentAutoCheckInterval(std::chrono::minutes{m_spinInterval->value()});
  taiga::settings.setTorrentNotifyNewEpisodes(m_checkNotify->isChecked());

  track::aggregator()->applyAutoCheckSettings();
}

}  // namespace gui
