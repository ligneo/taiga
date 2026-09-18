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

#include "update.hpp"

#include <chrono>

#include "media/anime_db.hpp"
#include "media/anime_list.hpp"
#include "media/anime_list_utils.hpp"
#include "sync/queue.hpp"
#include "taiga/settings.hpp"
#include "track/episode.hpp"
#include "track/recognition_validate.hpp"

namespace track {

namespace {

FuzzyDate today() {
  const auto now = std::chrono::system_clock::now();
  return FuzzyDate{Date{std::chrono::floor<std::chrono::days>(now)}};
}

int episodeNumber(const Episode& episode) {
  const auto range = episode.episodeNumberRange();
  return range ? range->second : 0;
}

}  // namespace

bool isUpdateAllowed(const Episode& episode) {
  if (!taiga::settings.syncEnabled()) return false;

  const auto item = anime::db.item(episode.animeId());
  if (!item) return false;

  if (!recognition::isValidEpisodeNumber(episode, *item)) return false;

  const auto number = episodeNumber(episode);
  if (number < 1) return false;

  const auto entry = anime::db.entry(episode.animeId());
  if (!entry) return true;  // the anime is not in the list yet

  if (entry->status == anime::list::Status::Completed && !entry->rewatching) return false;

  // Progress only moves forward, so that seeking back to an earlier episode does not undo it.
  return number > entry->watched_episodes;
}

void updateListEntry(const Episode& episode) {
  if (!isUpdateAllowed(episode)) return;

  const auto item = anime::db.item(episode.animeId());
  const auto previous = anime::db.entry(episode.animeId());
  const auto number = episodeNumber(episode);

  auto entry = previous ? *previous : anime::list::Entry{.anime_id = episode.animeId()};
  entry.watched_episodes = number;

  if (number == 1 && entry.date_started.empty()) {
    entry.date_started = today();
  }

  if (item->episode_count > 0 && number == item->episode_count) {
    if (entry.rewatching) {
      entry.rewatching = false;
      entry.rewatched_times += 1;
    } else {
      entry.status = anime::list::Status::Completed;
    }
    if (entry.date_completed.empty()) {
      entry.date_completed = today();
    }
  } else if (!entry.rewatching) {
    entry.status = anime::list::Status::Watching;
  }

  anime::list::save(entry);

  sync::queue.process();
}

}  // namespace track
