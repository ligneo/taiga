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

#include "settings.hpp"

#include <QFile>
#include <QJsonArray>
#include <ranges>

#include "base/string.hpp"
#include "compat/settings.hpp"
#include "sync/service.hpp"
#include "taiga/accounts.hpp"
#include "taiga/path.hpp"
#include "taiga/version.hpp"

namespace taiga {

void Settings::init() const {
  const auto appVersion = taiga::version().to_string();

  // v1 to v2
  if (!QFile::exists(fileName())) {
    compat::v1::readSettings(std::format("{}/v1/settings.xml", get_data_path()), *this, accounts);
    setValue("meta.version", appVersion);
    return;
  }

  // v2.x
  const auto fileVersion = value("meta.version").toString().toStdString();
  if (fileVersion != appVersion) {
    setValue("meta.version", appVersion);
  }
}

QString Settings::fileName() const {
  return u"%1/settings.json"_s.arg(QString::fromStdString(get_data_path()));
}

////////////////////////////////////////////////////////////////////////////////

Qt::ColorScheme Settings::appColorScheme() const {
  return value("app.colorScheme", static_cast<int>(Qt::ColorScheme::Unknown))
      .value<Qt::ColorScheme>();
}

std::vector<std::string> Settings::disabledMediaPlayers() const {
  return value("recognition.mediaPlayers.disabled").toJsonArray().toVariantList() |
         std::views::transform([](const QVariant& v) { return v.toString().toStdString(); }) |
         std::ranges::to<std::vector>();
}

std::string Settings::service() const {
  return value("v1.service", sync::serviceSlug(sync::ServiceId::AniList)).toString().toStdString();
}

std::vector<std::string> Settings::libraryFolders() const {
  return value("library.folders").toJsonArray().toVariantList() |
         std::views::transform([](const QVariant& v) { return v.toString().toStdString(); }) |
         std::ranges::to<std::vector>();
}

std::chrono::milliseconds Settings::mediaDetectionInterval() const {
  const auto interval = value("track.detection.interval", 3000).toInt();
  return std::chrono::milliseconds{interval};
}

QNetworkProxy::ProxyType Settings::proxyType() const {
  const auto type = value("network.proxy.type", u"http"_s).toString();
  if (type == u"socks5") return QNetworkProxy::Socks5Proxy;
  return QNetworkProxy::HttpProxy;
}

std::string Settings::proxyHost() const {
  return value("network.proxy.host").toString().toStdString();
}

int Settings::proxyPort() const {
  return value("network.proxy.port", -1).toInt();
}

std::string Settings::proxyUsername() const {
  return value("network.proxy.username").toString().toStdString();
}

std::string Settings::proxyPassword() const {
  return value("network.proxy.password").toString().toStdString();
}

bool Settings::streamingMediaEnabled() const {
  return value("recognition.streaming.enabled", false).toBool();
}

bool Settings::syncEnabled() const {
  return value("sync.enabled", true).toBool();
}

bool Settings::syncNotifyNotRecognized() const {
  return value("sync.notify.notRecognized", true).toBool();
}

bool Settings::syncNotifyRecognized() const {
  return value("sync.notify.recognized", true).toBool();
}

bool Settings::syncUpdateAskToConfirm() const {
  return value("sync.update.askToConfirm", true).toBool();
}

std::chrono::seconds Settings::syncUpdateDelay() const {
  const auto delay = value("sync.update.delay", 120).toInt();
  return std::chrono::seconds{delay};
}

bool Settings::syncUpdateOutOfRange() const {
  return value("sync.update.outOfRange", false).toBool();
}

bool Settings::syncUpdateOutOfRoot() const {
  return value("sync.update.outOfRoot", false).toBool();
}

bool Settings::syncUpdateWaitPlayer() const {
  return value("sync.update.waitPlayer", false).toBool();
}

anime::TitleLanguage Settings::titleLanguage() const {
  if (!titleLanguageCache_) {
    const auto language = value("library.titleLanguage", u"romaji"_s).toString();
    if (language == u"english") {
      titleLanguageCache_ = anime::TitleLanguage::English;
    } else if (language == u"native") {
      titleLanguageCache_ = anime::TitleLanguage::Native;
    } else {
      titleLanguageCache_ = anime::TitleLanguage::Romaji;
    }
  }
  return *titleLanguageCache_;
}

////////////////////////////////////////////////////////////////////////////////

void Settings::setAppColorScheme(const Qt::ColorScheme scheme) const {
  setValue("app.colorScheme", static_cast<int>(scheme));
}

std::vector<std::string> Settings::disabledStreamingProviders() const {
  return value("recognition.streaming.disabledProviders").toJsonArray().toVariantList() |
         std::views::transform([](const QVariant& v) { return v.toString().toStdString(); }) |
         std::ranges::to<std::vector>();
}

std::string Settings::torrentDiscoveryUrl() const {
  static const auto defaultUrl = u"https://www.tokyotosho.info/rss.php?filter=1,11&zwnj=0"_s;
  return value("torrents.discovery.url", defaultUrl).toString().toStdString();
}

std::string Settings::torrentSearchUrl() const {
  static const auto defaultUrl = u"https://nyaa.si/?page=rss&c=1_2&f=0&q=%title%"_s;
  return value("torrents.discovery.searchUrl", defaultUrl).toString().toStdString();
}

bool Settings::torrentAutoCheckEnabled() const {
  return value("torrents.discovery.autoCheck", true).toBool();
}

std::chrono::minutes Settings::torrentAutoCheckInterval() const {
  return std::chrono::minutes{value("torrents.discovery.interval", 60).toInt()};
}

void Settings::setDisabledMediaPlayers(std::vector<std::string> players) const {
  const auto list =
      players |
      std::views::transform([](const std::string& s) { return QString::fromStdString(s); }) |
      std::ranges::to<QList>();
  setValue("recognition.mediaPlayers.disabled", QJsonArray::fromStringList(list));
}

void Settings::setTorrentDiscoveryUrl(const std::string& url) const {
  setValue("torrents.discovery.url", url);
}

void Settings::setTorrentSearchUrl(const std::string& url) const {
  setValue("torrents.discovery.searchUrl", url);
}

void Settings::setTorrentAutoCheckEnabled(const bool enabled) const {
  setValue("torrents.discovery.autoCheck", enabled);
}

void Settings::setTorrentAutoCheckInterval(const std::chrono::minutes interval) const {
  setValue("torrents.discovery.interval", static_cast<int>(interval.count()));
}

void Settings::setDisabledStreamingProviders(std::vector<std::string> providers) const {
  const auto list =
      providers |
      std::views::transform([](const std::string& s) { return QString::fromStdString(s); }) |
      std::ranges::to<QList>();
  setValue("recognition.streaming.disabledProviders", QJsonArray::fromStringList(list));
}

void Settings::setService(const std::string& service) const {
  setValue("v1.service", service);
}

void Settings::setLibraryFolders(std::vector<std::string> folders) const {
  const auto list =
      folders |
      std::views::transform([](const std::string& s) { return QString::fromStdString(s); }) |
      std::ranges::to<QList>();
  setValue("library.folders", QJsonArray::fromStringList(list));
}

void Settings::setMediaDetectionInterval(const std::chrono::milliseconds interval) const {
  setValue("track.detection.interval", static_cast<qint64>(interval.count()));
}

void Settings::setProxyType(const QNetworkProxy::ProxyType type) const {
  const std::string slug = type == QNetworkProxy::Socks5Proxy ? "socks5" : "http";
  setValue("network.proxy.type", slug);
}

void Settings::setProxyHost(const std::string& host) const {
  setValue("network.proxy.host", host);
}

void Settings::setProxyPort(const int port) const {
  setValue("network.proxy.port", port);
}

void Settings::setProxyUsername(const std::string& username) const {
  setValue("network.proxy.username", username);
}

void Settings::setProxyPassword(const std::string& password) const {
  setValue("network.proxy.password", password);
}

void Settings::setStreamingMediaEnabled(const bool enabled) const {
  setValue("recognition.streaming.enabled", enabled);
}

void Settings::setSyncEnabled(const bool enabled) const {
  setValue("sync.enabled", enabled);
}

void Settings::setSyncNotifyNotRecognized(const bool enabled) const {
  setValue("sync.notify.notRecognized", enabled);
}

void Settings::setSyncNotifyRecognized(const bool enabled) const {
  setValue("sync.notify.recognized", enabled);
}

void Settings::setSyncUpdateAskToConfirm(const bool enabled) const {
  setValue("sync.update.askToConfirm", enabled);
}

void Settings::setSyncUpdateDelay(const std::chrono::seconds delay) const {
  setValue("sync.update.delay", static_cast<int>(delay.count()));
}

void Settings::setSyncUpdateOutOfRange(const bool enabled) const {
  setValue("sync.update.outOfRange", enabled);
}

void Settings::setSyncUpdateOutOfRoot(const bool enabled) const {
  setValue("sync.update.outOfRoot", enabled);
}

void Settings::setSyncUpdateWaitPlayer(const bool enabled) const {
  setValue("sync.update.waitPlayer", enabled);
}

void Settings::setTitleLanguage(const anime::TitleLanguage language) const {
  const auto slug = [language]() -> std::string {
    switch (language) {
      default:
      case anime::TitleLanguage::Romaji:
        return "romaji";
      case anime::TitleLanguage::English:
        return "english";
      case anime::TitleLanguage::Native:
        return "native";
    }
  }();
  setValue("library.titleLanguage", slug);
  titleLanguageCache_ = language;
}

}  // namespace taiga
