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

#include "recognition.hpp"

#include <QFileInfo>
#include <algorithm>
#include <anitomy.hpp>
#include <format>
#include <ranges>
#include <vector>

#include "base/string.hpp"
#include "media/anime.hpp"
#include "media/anime_db.hpp"
#include "track/episode.hpp"
#include "track/recognition_cache.hpp"
#include "track/recognition_normalize.hpp"
#include "track/recognition_path.hpp"
#include "track/recognition_relations.hpp"
#include "track/recognition_validate.hpp"

namespace track::recognition {

namespace {

QString formatEpisodeRange(const std::pair<int, int>& range) {
  if (range.second > range.first) return u"%1-%2"_s.arg(range.first).arg(range.second);
  return QString::number(range.first);
}

int identifyByTitle(Episode& episode, const std::string& normalizedTitle) {
  std::vector<Cache::Data::Match> matches;

  if (const auto data = cache()->find(normalizedTitle)) {
    matches.append_range(data->matches | std::views::values | std::ranges::to<std::vector>());
  }

  std::ranges::sort(matches, std::ranges::greater{}, &Cache::Data::Match::weight);

  for (const auto& match : matches) {
    const auto item = anime::db.item(match.id);
    if (!item) continue;

    if (!isValidEpisodeType(episode)) continue;

    if (!isValidEpisodeNumber(episode, *item)) {
      const auto range = episode.episodeNumberRange();
      if (!range) continue;

      const auto redirect = findRedirection(match.id, *range);
      if (!redirect) continue;

      qDebug() << u"Redirection: %1:%2 -> %3:%4"_s.arg(match.id)
                      .arg(formatEpisodeRange(*range))
                      .arg(redirect->id)
                      .arg(formatEpisodeRange(redirect->episode_range));
      episode.setEpisodeNumberRange(redirect->episode_range);
      return redirect->id;
    }

    return match.id;
  }

  return anime::kUnknownId;
}

// Sequels are separate entries with titles of their own (e.g. "Sousou no Frieren 2nd Season"),
// while file names only have the season number. Appending the season to the title is enough to
// tell them apart, because normalization reduces both forms to the same number.
std::vector<std::string> titleCandidates(const Episode& episode) {
  const auto title = episode.element(anitomy::ElementKind::Title);
  const auto season = toInt(episode.element(anitomy::ElementKind::Season));

  // Falling back to the bare title would match the first season instead, which is worse than not
  // recognizing the episode at all, because the list would be updated for the wrong entry.
  if (season > 1) {
    return {normalize(std::format("{} Season {}", title, season))};
  }

  return {normalize(title)};
}

}  // namespace

Episode parse(std::string_view input, const anitomy::Options options) {
  Episode episode;

  auto elements = anitomy::parse(input, options);
  episode.setElements(elements);

  return episode;
}

Episode parseFileInfo(const QFileInfo& info, const anitomy::Options options) {
  const auto fileName = info.fileName().toStdString();

  Episode episode = track::recognition::parse(fileName, options);

  if (!episode.contains(anitomy::ElementKind::Title)) {
    const auto parsed = parseParentDirectories(info);
    if (!parsed.title.empty()) {
      episode.addElement(anitomy::ElementKind::Title, parsed.title);
    }
    if (!parsed.season.empty() && !episode.contains(anitomy::ElementKind::Season)) {
      episode.addElement(anitomy::ElementKind::Season, parsed.season);
    }
  }

  return episode;
}

int identify(Episode& episode) {
  cache()->init();

  for (const auto& title : titleCandidates(episode)) {
    if (const auto id = identifyByTitle(episode, title); id != anime::kUnknownId) return id;
  }

  return anime::kUnknownId;
}

bool isVideoFile(const Episode& episode) {
  // This relies on Anitomy tagging `FileExtension` only for video
  // container extensions.
  return episode.contains(anitomy::ElementKind::FileExtension);
}

}  // namespace track::recognition
