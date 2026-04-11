#include "Settings.h"

#include "util/EliteFileUtil.h"

#include <json.hpp>

#include <fstream>


Settings::Settings()
	: _appSettingsDirectory(EliteFileUtil::getConfigPath("EDWingHelper"))
{
	// Ensure app preference folder exists
	std::filesystem::create_directories(_appSettingsDirectory);

	_appMainSettingsFile = _appSettingsDirectory / "preferences.json";

	if (!std::filesystem::exists(_appMainSettingsFile)) {
		loadDefaultSettings();
	}
	else {
		loadSettings();
	}
}


Settings::~Settings()
{
	saveSettings();
}


void Settings::loadDefaultSettings()
{
	_mainWindowConfigFile = "mainwindow.ini";
	_overlayWindowConfigFile = "overlaywindow.ini";
	_playerProfileDirectory = EliteFileUtil::getUserProfile();
	saveSettings();
}


void Settings::loadSettings()
{
	std::ifstream in(_appMainSettingsFile);

	if (!in.is_open()) {
		// Silently fail in release, assert in debug
		assert(0);
		loadDefaultSettings();
		return;
	}

	nlohmann::json j;
	in >> j;
	_mainWindowConfigFile = j["mainWindowConfigFile"].get<std::filesystem::path>();
	_overlayWindowConfigFile = j["overlayWindowConfigFile"].get<std::filesystem::path>();
	_playerProfileDirectory = j["playerProfileDirectory"].get<std::filesystem::path>();
}


void Settings::saveSettings()
{
	nlohmann::json j;
	j["mainWindowConfigFile"] = _mainWindowConfigFile.string();
	j["overlayWindowConfigFile"] = _overlayWindowConfigFile.string();
	j["playerProfileDirectory"] = _playerProfileDirectory.string();
	
	std::ofstream out(_appMainSettingsFile);
	out << j.dump(4);
}