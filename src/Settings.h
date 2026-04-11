#pragma once

#include <filesystem>


class Settings
{
public:
	Settings();
	~Settings();

	std::filesystem::path mainWindowConfigFile() const { return _appSettingsDirectory / _mainWindowConfigFile; }
	std::filesystem::path overlayWindowConfigFile() const { return _appSettingsDirectory / _overlayWindowConfigFile; }
	const std::filesystem::path& playerProfileDirectory() const { return _playerProfileDirectory; }

private:
	void loadDefaultSettings();
	void loadSettings();

	void saveSettings();

	std::filesystem::path _appSettingsDirectory;

	std::filesystem::path _appMainSettingsFile;

	std::filesystem::path _mainWindowConfigFile;
	std::filesystem::path _overlayWindowConfigFile;
	std::filesystem::path _playerProfileDirectory;
};