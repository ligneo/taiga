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

#include "media.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>

#include "media/anime.hpp"
#include "media/anime_db.hpp"
#include "taiga/settings.hpp"
#include "track/episode.hpp"
#include "track/media_player.hpp"
#include "track/media_stream.hpp"
#ifdef Q_OS_LINUX
#include "track/media_mpris.hpp"
#endif
#include "track/recognition.hpp"
#include "track/update.hpp"

namespace track::media {

namespace {

struct MediaFields {
  std::string file;
  std::string title;
  std::string url;
};

#if defined(Q_OS_WINDOWS)
namespace platform = anisthesia::win;
#elif defined(Q_OS_LINUX)
namespace platform = anisthesia::lin;
#endif

#if defined(Q_OS_WINDOWS) || defined(Q_OS_LINUX)
anisthesia::Media flattenMedia(const platform::Result& result) {
  anisthesia::Media media;

  for (const auto& item : result.media) {
    media.information.append_range(item.information);
  }

  return media;
}

bool isVideoFile(const std::string& path) {
  const auto fileName = QFileInfo{QString::fromStdString(path)}.fileName();
  return track::recognition::isVideoFile(track::recognition::parse(fileName.toStdString()));
}
#endif

MediaFields extractMediaFields(const anisthesia::Media& media) {
  MediaFields fields;

  for (const auto& information : media.information) {
    switch (information.type) {
      case anisthesia::MediaInfoType::File:
        if (fields.file.empty()) fields.file = information.value;
        break;
      case anisthesia::MediaInfoType::Url:
        fields.url = information.value;
        break;
      case anisthesia::MediaInfoType::Title:
        if (fields.title.empty()) fields.title = information.value;
        break;
      case anisthesia::MediaInfoType::Unknown:
        // Always prefer window title over page title reported via UI Automation.
        fields.title = information.value;
        break;
      case anisthesia::MediaInfoType::Tab:
        break;
    }
  }

  return fields;
}

std::optional<Episode> resolveEpisode(const MediaFields& fields) {
  if (!fields.file.empty()) {
    const QFileInfo fileInfo{QString::fromStdString(fields.file)};
    auto episode = track::recognition::parseFileInfo(fileInfo);

    if (!track::recognition::isVideoFile(episode)) return std::nullopt;

    return episode;
  }

  auto value = fields.title;

  if (value.empty()) return std::nullopt;

  if (!fields.url.empty()) {
    const auto title = track::recognition::titleFromStreamingProvider(fields.url, value);
    if (!title) return std::nullopt;

    value = *title;
  }

  return track::recognition::parse(value);
}

}  // namespace

Detection::Detection(QObject* parent) : QObject(parent) {
  pollTimer_ = new QTimer(this);
  connect(pollTimer_, &QTimer::timeout, this, &Detection::poll);
}

const std::optional<Episode> Detection::getCurrentEpisode() const {
  return currentEpisode_;
}

const std::optional<Detection::media_t> Detection::getCurrentMedia() const {
  return currentMedia_;
}

const std::optional<Detection::player_t> Detection::getCurrentPlayer() const {
  return currentPlayer_;
}

bool Detection::init() {
  if (!parsePlayersData(players_)) {
    return false;
  }

  setEnabled(taiga::settings.mediaDetectionEnabled());

  return true;
}

// v1's `program/general/enablerecognition`: detection can be turned off without quitting.
void Detection::setEnabled(const bool enabled) {
#if defined(Q_OS_WINDOWS) || defined(Q_OS_LINUX)
  if (enabled) {
    pollTimer_->start(taiga::settings.mediaDetectionInterval());
  } else {
    pollTimer_->stop();
    reset();
  }
#endif
}

void Detection::poll() {
#if defined(Q_OS_WINDOWS) || defined(Q_OS_LINUX)
  const auto players = getEnabledPlayers(players_);

  static const auto media_proc = [](const anisthesia::MediaInfo& info) {
    // Players may keep other files open as well (e.g. caches, databases)
    if (info.type == anisthesia::MediaInfoType::File) return isVideoFile(info.value);
    return true;
  };

  std::vector<platform::Result> results;
  if (!platform::GetResults(players, media_proc, results)) {
    results.clear();
  }
#ifdef Q_OS_LINUX
  std::ranges::move(getMprisResults(players), std::back_inserter(results));
#endif
  if (results.empty()) {
    reset();
    return;
  }

  static const auto getPlayerId = [](const platform::Result& result) -> player_id_t {
#ifdef Q_OS_WINDOWS
    return result.window.handle;
#else
    return result.process.id;
#endif
  };

  const auto resultIt = std::ranges::find_if(
      results, [this](const platform::Result& r) { return getPlayerId(r) == currentPlayerId_; });
  const auto& result = resultIt != results.end() ? *resultIt : results.front();

  currentPlayer_ = result.player;
  currentMedia_ = flattenMedia(result);
  currentPlayerId_ = getPlayerId(result);

  auto episode = resolveEpisode(extractMediaFields(*currentMedia_));
  if (!episode) {
    reset();
    return;
  }

  const auto animeId = track::recognition::identify(*episode);
  episode->setAnimeId(animeId);

  if (hasEpisodeChanged(*episode)) {
    currentEpisode_ = episode;
    episodeElapsed_ = {};
    episodeProcessed_ = false;
    emit currentEpisodeChanged(episode);
  } else {
    episodeElapsed_ += taiga::settings.mediaDetectionInterval();
  }

  if (!episodeProcessed_ && timeUntilUpdate() <= std::chrono::seconds{0} &&
      !taiga::settings.syncUpdateWaitPlayer()) {
    episodeProcessed_ = true;
    requestListEntryUpdate(*currentEpisode_);
  }
#endif
}

void Detection::requestListEntryUpdate(const Episode& episode) {
  if (!isUpdateAllowed(episode)) return;

  if (taiga::settings.syncUpdateAskToConfirm()) {
    emit listEntryUpdateRequested(episode);
  } else {
    updateListEntry(episode);
  }
}

std::chrono::seconds Detection::timeUntilUpdate() const {
  const auto delay = taiga::settings.syncUpdateDelay();
  const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(episodeElapsed_);
  return elapsed < delay ? delay - elapsed : std::chrono::seconds{0};
}

bool Detection::isMediaIdentified() const {
  return currentEpisode_.has_value() && currentEpisode_->animeId() != anime::kUnknownId;
}

void Detection::setCurrentEpisodeAnimeId(int animeId) {
  if (!currentEpisode_) return;

  currentEpisode_->setAnimeId(animeId);
  emit currentEpisodeChanged(currentEpisode_);
}

void Detection::reset() {
  if (currentEpisode_ && !episodeProcessed_ && timeUntilUpdate() <= std::chrono::seconds{0} &&
      taiga::settings.syncUpdateWaitPlayer()) {
    episodeProcessed_ = true;
    requestListEntryUpdate(*currentEpisode_);
  }

  currentPlayer_.reset();
  currentMedia_.reset();
  currentPlayerId_ = {};
  episodeElapsed_ = {};
  episodeProcessed_ = false;

  if (currentEpisode_) {
    currentEpisode_.reset();
    emit currentEpisodeChanged(std::nullopt);
  }
}

bool Detection::hasEpisodeChanged(const Episode& episode) const {
  if (!currentEpisode_) return true;
  if (currentEpisode_->animeId() != episode.animeId()) return true;

  return currentEpisode_->elements(anitomy::ElementKind::Episode) !=
         episode.elements(anitomy::ElementKind::Episode);
}

}  // namespace track::media
