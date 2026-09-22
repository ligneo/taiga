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

#pragma once

#include <QJsonArray>
#include <QNetworkProxy>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "base/settings.hpp"
#include "media/anime.hpp"

namespace taiga {

class Settings final : public base::Settings {
public:
  static constexpr QLatin1StringView kAppStyleSystem{"system"};

  void init() const;

  Qt::ColorScheme appColorScheme() const;
  std::string appStyle() const;
  std::vector<std::string> externalLinks() const;
  std::vector<std::string> disabledMediaPlayers() const;
  std::vector<std::string> disabledStreamingProviders() const;
  std::string torrentDiscoveryUrl() const;
  std::string torrentSearchUrl() const;
  bool torrentAutoCheckEnabled() const;
  std::chrono::minutes torrentAutoCheckInterval() const;
  bool torrentNotifyNewEpisodes() const;
  bool torrentDownloadNewEpisodes() const;
  bool torrentFilterEnabled() const;
  int torrentArchiveMaxCount() const;
  std::string torrentDownloadSortBy() const;
  Qt::SortOrder torrentDownloadSortOrder() const;
  std::string torrentDownloadLocation() const;
  bool torrentDownloadOpen() const;
  bool torrentDownloadUseMagnet() const;
  std::string torrentDownloadAppMode() const;
  std::string torrentDownloadAppPath() const;
  std::string torrentDownloadFileLocation() const;
  std::optional<QJsonArray> torrentFilters() const;
  std::string service() const;
  std::vector<std::string> libraryFolders() const;
  bool libraryWatchFolders() const;
  bool libraryScanOnStartup() const;
  qint64 libraryMinimumFileSize() const;
  bool mediaDetectionEnabled() const;
  std::chrono::milliseconds mediaDetectionInterval() const;
  std::vector<std::string> recognitionIgnoredStrings() const;
  bool recognitionLookupParentDirectories() const;
  QNetworkProxy::ProxyType proxyType() const;
  std::string proxyHost() const;
  int proxyPort() const;
  std::string proxyUsername() const;
  std::string proxyPassword() const;
  bool sidebarVisible() const;
  bool appAutoStart() const;
  bool appCloseToTray() const;
  bool appMinimizeToTray() const;
  bool appStartMinimized() const;
  std::string listDoubleClickAction() const;
  std::string listMiddleClickAction() const;
  bool listHighlightNewEpisodes() const;
  bool listShowAiredEpisodes() const;
  bool listShowAvailableEpisodes() const;
  bool streamingMediaEnabled() const;
  bool httpShareEnabled() const;
  std::string httpShareUrl() const;
  std::string httpShareFormat() const;
  bool discordEnabled() const;
  std::string discordApplicationId() const;
  bool discordTimeEnabled() const;
  bool discordUsernameEnabled() const;
  bool syncEnabled() const;
  bool syncNotifyNotRecognized() const;
  bool syncNotifyRecognized() const;
  bool syncUpdateAskToConfirm() const;
  bool syncUpdateOutOfRange() const;
  bool syncUpdateOutOfRoot() const;
  bool syncUpdateWaitPlayer() const;
  std::chrono::seconds syncUpdateDelay() const;
  anime::TitleLanguage titleLanguage() const;

  void setAppColorScheme(const Qt::ColorScheme scheme) const;
  void setAppStyle(const std::string& style) const;
  void setExternalLinks(std::vector<std::string> links) const;
  void setDisabledMediaPlayers(std::vector<std::string> players) const;
  void setDisabledStreamingProviders(std::vector<std::string> providers) const;
  void setTorrentDiscoveryUrl(const std::string& url) const;
  void setTorrentSearchUrl(const std::string& url) const;
  void setTorrentAutoCheckEnabled(const bool enabled) const;
  void setTorrentAutoCheckInterval(const std::chrono::minutes interval) const;
  void setTorrentNotifyNewEpisodes(const bool enabled) const;
  void setTorrentDownloadNewEpisodes(const bool enabled) const;
  void setTorrentFilterEnabled(const bool enabled) const;
  void setTorrentArchiveMaxCount(const int count) const;
  void setTorrentDownloadSortBy(const std::string& sortBy) const;
  void setTorrentDownloadSortOrder(const Qt::SortOrder order) const;
  void setTorrentDownloadLocation(const std::string& path) const;
  void setTorrentDownloadOpen(const bool open) const;
  void setTorrentDownloadUseMagnet(const bool use) const;
  void setTorrentDownloadAppMode(const std::string& mode) const;
  void setTorrentDownloadAppPath(const std::string& path) const;
  void setTorrentDownloadFileLocation(const std::string& path) const;
  void setTorrentFilters(const QJsonArray& filters) const;
  void setService(const std::string& service) const;
  void setLibraryFolders(std::vector<std::string> folders) const;
  void setLibraryWatchFolders(const bool watch) const;
  void setLibraryScanOnStartup(const bool scan) const;
  void setLibraryMinimumFileSize(const qint64 bytes) const;
  void setMediaDetectionEnabled(const bool enabled) const;
  void setMediaDetectionInterval(const std::chrono::milliseconds interval) const;
  void setRecognitionIgnoredStrings(std::vector<std::string> strings) const;
  void setRecognitionLookupParentDirectories(const bool lookup) const;
  void setProxyType(const QNetworkProxy::ProxyType type) const;
  void setProxyHost(const std::string& host) const;
  void setProxyPort(const int port) const;
  void setProxyUsername(const std::string& username) const;
  void setProxyPassword(const std::string& password) const;
  void setSidebarVisible(const bool visible) const;
  void setAppAutoStart(const bool enabled) const;
  void setAppCloseToTray(const bool enabled) const;
  void setAppMinimizeToTray(const bool enabled) const;
  void setAppStartMinimized(const bool enabled) const;
  void setListDoubleClickAction(const std::string& action) const;
  void setListMiddleClickAction(const std::string& action) const;
  void setListHighlightNewEpisodes(const bool highlight) const;
  void setListShowAiredEpisodes(const bool show) const;
  void setListShowAvailableEpisodes(const bool show) const;
  void setStreamingMediaEnabled(const bool enabled) const;
  void setHttpShareEnabled(const bool enabled) const;
  void setHttpShareUrl(const std::string& url) const;
  void setHttpShareFormat(const std::string& format) const;
  void setDiscordEnabled(const bool enabled) const;
  void setDiscordApplicationId(const std::string& id) const;
  void setDiscordTimeEnabled(const bool enabled) const;
  void setDiscordUsernameEnabled(const bool enabled) const;
  void setSyncEnabled(const bool enabled) const;
  void setSyncNotifyNotRecognized(const bool enabled) const;
  void setSyncNotifyRecognized(const bool enabled) const;
  void setSyncUpdateAskToConfirm(const bool enabled) const;
  void setSyncUpdateOutOfRange(const bool enabled) const;
  void setSyncUpdateOutOfRoot(const bool enabled) const;
  void setSyncUpdateWaitPlayer(const bool enabled) const;
  void setSyncUpdateDelay(const std::chrono::seconds delay) const;
  void setTitleLanguage(const anime::TitleLanguage language) const;

private:
  QString fileName() const override;

  mutable std::optional<anime::TitleLanguage> titleLanguageCache_;
};

inline Settings settings;

}  // namespace taiga
