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

#include "feed_filter_util.hpp"

#include <map>

namespace track::util {

namespace {

using namespace Qt::StringLiterals;

const std::map<FilterAction, QString> kActions{
    {FilterAction::Discard, u"discard"_s},
    {FilterAction::Select, u"select"_s},
    {FilterAction::Prefer, u"prefer"_s},
};

const std::map<FilterElement, QString> kElements{
    {FilterElement::MetaId, u"meta_id"_s},
    {FilterElement::MetaStatus, u"meta_status"_s},
    {FilterElement::MetaType, u"meta_type"_s},
    {FilterElement::MetaEpisodes, u"meta_episodes"_s},
    {FilterElement::MetaDateStart, u"meta_date_start"_s},
    {FilterElement::MetaDateEnd, u"meta_date_end"_s},
    // v1 named this one after the field it used to be, and its filters still say so.
    {FilterElement::UserNotes, u"user_tags"_s},
    {FilterElement::UserStatus, u"user_status"_s},
    {FilterElement::LocalEpisodeAvailable, u"local_episode_available"_s},
    {FilterElement::EpisodeTitle, u"episode_title"_s},
    {FilterElement::EpisodeNumber, u"episode_number"_s},
    {FilterElement::EpisodeVersion, u"episode_version"_s},
    {FilterElement::EpisodeGroup, u"episode_group"_s},
    {FilterElement::EpisodeVideoResolution, u"episode_video_resolution"_s},
    {FilterElement::EpisodeVideoType, u"episode_video_type"_s},
    {FilterElement::FileTitle, u"file_title"_s},
    {FilterElement::FileCategory, u"file_category"_s},
    {FilterElement::FileDescription, u"file_description"_s},
    {FilterElement::FileLink, u"file_link"_s},
    {FilterElement::FileSize, u"file_size"_s},
};

const std::map<FilterMatch, QString> kMatches{
    {FilterMatch::All, u"all"_s},
    {FilterMatch::Any, u"any"_s},
};

const std::map<FilterOperator, QString> kOperators{
    {FilterOperator::Equals, u"equals"_s},
    {FilterOperator::NotEquals, u"notequals"_s},
    {FilterOperator::IsGreaterThan, u"gt"_s},
    {FilterOperator::IsGreaterThanOrEqualTo, u"ge"_s},
    {FilterOperator::IsLessThan, u"lt"_s},
    {FilterOperator::IsLessThanOrEqualTo, u"le"_s},
    {FilterOperator::BeginsWith, u"beginswith"_s},
    {FilterOperator::EndsWith, u"endswith"_s},
    {FilterOperator::Contains, u"contains"_s},
    {FilterOperator::NotContains, u"notcontains"_s},
};

const std::map<FilterOption, QString> kOptions{
    {FilterOption::Default, u"default"_s},
    {FilterOption::Deactivate, u"deactivate"_s},
    {FilterOption::Hide, u"hide"_s},
};

template <typename T>
QString toShortcode(const std::map<T, QString>& map, const T value) {
  const auto it = map.find(value);
  return it != map.end() ? it->second : QString{};
}

template <typename T>
std::optional<T> fromShortcode(const std::map<T, QString>& map, const QString& shortcode) {
  for (const auto& [value, text] : map) {
    if (text.compare(shortcode, Qt::CaseInsensitive) == 0) return value;
  }
  return std::nullopt;
}

}  // namespace

QString shortcode(const FilterAction value) {
  return toShortcode(kActions, value);
}

QString shortcode(const FilterElement value) {
  return toShortcode(kElements, value);
}

QString shortcode(const FilterMatch value) {
  return toShortcode(kMatches, value);
}

QString shortcode(const FilterOperator value) {
  return toShortcode(kOperators, value);
}

QString shortcode(const FilterOption value) {
  return toShortcode(kOptions, value);
}

std::optional<FilterAction> filterAction(const QString& shortcode) {
  return fromShortcode(kActions, shortcode);
}

std::optional<FilterElement> filterElement(const QString& shortcode) {
  return fromShortcode(kElements, shortcode);
}

std::optional<FilterMatch> filterMatch(const QString& shortcode) {
  return fromShortcode(kMatches, shortcode);
}

std::optional<FilterOperator> filterOperator(const QString& shortcode) {
  return fromShortcode(kOperators, shortcode);
}

std::optional<FilterOption> filterOption(const QString& shortcode) {
  return fromShortcode(kOptions, shortcode);
}

}  // namespace track::util
