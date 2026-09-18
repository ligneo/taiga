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

#include "settings_recognition_page.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QVBoxLayout>

#include "taiga/settings.hpp"

namespace gui {

RecognitionPage::RecognitionPage(QWidget* parent)
    : SettingsPage(parent),
      m_checkOutOfRoot(new QCheckBox(tr("Ignore files outside of library folders"), this)),
      m_checkOutOfRange(
          new QCheckBox(tr("Ignore if episode number is greater than next episode"), this)),
      m_checkNotifyRecognized(new QCheckBox(tr("Notify me when an episode is recognized"), this)),
      m_checkNotifyNotRecognized(
          new QCheckBox(tr("Notify me when an episode is not recognized"), this)),
      m_spinDelay(new QSpinBox(this)),
      m_checkWaitPlayer(new QCheckBox(tr("Wait for media player to close"), this)),
      m_checkAskToConfirm(new QCheckBox(tr("Ask for confirmation"), this)),
      m_spinDetectionInterval(new QSpinBox(this)) {
  const auto layout = new QVBoxLayout(this);

  // Validation
  {
    const auto group = new QGroupBox(tr("Validation"), this);
    const auto groupLayout = new QVBoxLayout(group);
    groupLayout->addWidget(m_checkOutOfRoot);
    groupLayout->addWidget(m_checkOutOfRange);
    layout->addWidget(group);
  }

  // Behavior
  {
    const auto group = new QGroupBox(tr("Behavior"), this);
    const auto groupLayout = new QVBoxLayout(group);
    groupLayout->addWidget(m_checkNotifyRecognized);
    groupLayout->addWidget(m_checkNotifyNotRecognized);
    layout->addWidget(group);
  }

  // List updates
  {
    const auto group = new QGroupBox(tr("List updates"), this);
    const auto groupLayout = new QVBoxLayout(group);
    const auto form = new QFormLayout();

    m_spinDelay->setRange(10, 3600);
    m_spinDelay->setSingleStep(10);
    m_spinDelay->setSuffix(tr(" seconds"));
    form->addRow(tr("Delay:"), m_spinDelay);

    groupLayout->addLayout(form);
    groupLayout->addWidget(m_checkWaitPlayer);
    groupLayout->addWidget(m_checkAskToConfirm);
    layout->addWidget(group);
  }

  // Detection
  {
    const auto group = new QGroupBox(tr("Detection"), this);
    const auto form = new QFormLayout(group);

    m_spinDetectionInterval->setRange(1, 60);
    m_spinDetectionInterval->setSuffix(tr(" seconds"));
    form->addRow(tr("Check media players every:"), m_spinDetectionInterval);

    layout->addWidget(group);
  }

  layout->addStretch();
}

void RecognitionPage::load() {
  m_checkOutOfRoot->setChecked(taiga::settings.syncUpdateOutOfRoot());
  m_checkOutOfRange->setChecked(taiga::settings.syncUpdateOutOfRange());
  m_checkNotifyRecognized->setChecked(taiga::settings.syncNotifyRecognized());
  m_checkNotifyNotRecognized->setChecked(taiga::settings.syncNotifyNotRecognized());
  m_spinDelay->setValue(static_cast<int>(taiga::settings.syncUpdateDelay().count()));
  m_checkWaitPlayer->setChecked(taiga::settings.syncUpdateWaitPlayer());
  m_checkAskToConfirm->setChecked(taiga::settings.syncUpdateAskToConfirm());

  const auto interval =
      std::chrono::duration_cast<std::chrono::seconds>(taiga::settings.mediaDetectionInterval());
  m_spinDetectionInterval->setValue(static_cast<int>(interval.count()));
}

void RecognitionPage::save() {
  taiga::settings.setSyncUpdateOutOfRoot(m_checkOutOfRoot->isChecked());
  taiga::settings.setSyncUpdateOutOfRange(m_checkOutOfRange->isChecked());
  taiga::settings.setSyncNotifyRecognized(m_checkNotifyRecognized->isChecked());
  taiga::settings.setSyncNotifyNotRecognized(m_checkNotifyNotRecognized->isChecked());
  taiga::settings.setSyncUpdateDelay(std::chrono::seconds{m_spinDelay->value()});
  taiga::settings.setSyncUpdateWaitPlayer(m_checkWaitPlayer->isChecked());
  taiga::settings.setSyncUpdateAskToConfirm(m_checkAskToConfirm->isChecked());
  taiga::settings.setMediaDetectionInterval(
      std::chrono::seconds{m_spinDetectionInterval->value()});
}

}  // namespace gui
