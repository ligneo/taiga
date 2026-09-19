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

#include "library.hpp"

#include <QDirIterator>

#include "base/log.hpp"
#include "media/anime.hpp"
#include "media/anime_db.hpp"
#include "taiga/settings.hpp"
#include "track/episode.hpp"
#include "track/recognition.hpp"

namespace track {

Library::Library(QObject* parent) : QObject(parent) {}

void Library::scan() {
  decltype(episodes_) episodes;
  int episodeCount = 0;

  for (const auto& folder : taiga::settings.libraryFolders()) {
    QDirIterator it{QString::fromStdString(folder), QDir::Files, QDirIterator::Subdirectories};

    while (it.hasNext()) {
      const auto info = it.nextFileInfo();

      auto episode = recognition::parseFileInfo(info);

      if (!recognition::isVideoFile(episode)) continue;

      const auto animeId = recognition::identify(episode);

      if (animeId == anime::kUnknownId) continue;

      const auto range = episode.episodeNumberRange();

      // A file without an episode number is taken to be the first episode, as in v1.
      const int number = range ? range->second : 1;

      const auto item = anime::db.item(animeId);

      // Anything beyond the known episode count is a misparse rather than a new episode.
      if (item && item->episode_count > 0 && number > item->episode_count) continue;

      if (episodes[animeId].try_emplace(number, info.filePath()).second) ++episodeCount;
    }
  }

  episodes_ = std::move(episodes);

  qInfo() << "Found" << episodeCount << "episodes of" << episodes_.size() << "anime";

  emit availabilityChanged();
  emit scanCompleted(static_cast<int>(episodes_.size()), episodeCount);
}

int Library::availableEpisodeCount(const int animeId) const {
  const auto it = episodes_.find(animeId);
  return it != episodes_.end() ? static_cast<int>(it->second.size()) : 0;
}

bool Library::isEpisodeAvailable(const int animeId, const int number) const {
  const auto it = episodes_.find(animeId);
  return it != episodes_.end() && it->second.contains(number);
}

QString Library::episodePath(const int animeId, const int number) const {
  const auto it = episodes_.find(animeId);

  if (it == episodes_.end()) return {};

  const auto episode = it->second.find(number);

  return episode != it->second.end() ? episode->second : QString{};
}

}  // namespace track
