#include "Settings.h"

namespace
{
	const std::filesystem::path kPath{ "Data/SKSE/Plugins/NoKillmoves.ini" };
	std::mutex                  g_fileLock;

	std::string Trim(const std::string& a_text)
	{
		const auto first = a_text.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) {
			return {};
		}
		return a_text.substr(first, a_text.find_last_not_of(" \t\r\n") - first + 1);
	}

	bool ParseBool(const std::string& a_value)
	{
		return a_value == "1" || a_value == "true" || a_value == "True" || a_value == "TRUE";
	}

	const char* Bool(bool a_value) { return a_value ? "true" : "false"; }

	void LogSettings(const char* a_prefix)
	{
		logs::info("{}: disablePlayerThirdPerson={} disablePlayerFirstPerson={} disableNPC={} disableMagicKillCam={} disableBowKillCam={}",
			a_prefix, Settings::disablePlayerThirdPerson.load(), Settings::disablePlayerFirstPerson.load(), Settings::disableNPC.load(),
			Settings::disableMagicKillCam.load(), Settings::disableBowKillCam.load());
	}
}

void Settings::Load()
{
	std::scoped_lock lock{ g_fileLock };
	std::ifstream    file{ kPath };
	if (!file) {
		logs::warn("{} not found, using defaults", kPath.string());
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		line = Trim(line);
		if (line.empty() || line[0] == ';' || line[0] == '#' || line[0] == '[') {
			continue;
		}
		const auto eq = line.find('=');
		if (eq == std::string::npos) {
			continue;
		}
		const auto key = Trim(line.substr(0, eq));
		auto       value = Trim(line.substr(eq + 1));
		if (const auto comment = value.find(';'); comment != std::string::npos) {
			value = Trim(value.substr(0, comment));
		}

		if (key == "bDisablePlayerThirdPerson") {
			disablePlayerThirdPerson = ParseBool(value);
		} else if (key == "bDisablePlayerFirstPerson") {
			disablePlayerFirstPerson = ParseBool(value);
		} else if (key == "bDisableNPC") {
			disableNPC = ParseBool(value);
		} else if (key == "bDisableMagicKillCam") {
			disableMagicKillCam = ParseBool(value);
		} else if (key == "bDisableBowKillCam") {
			disableBowKillCam = ParseBool(value);
		}
	}
	LogSettings("Settings loaded");
}

void Settings::Save()
{
	std::scoped_lock lock{ g_fileLock };
	{
		std::ofstream out{ kPath, std::ios::trunc };
		if (!out) {
			logs::error("Could not write {}", kPath.string());
			return;
		}
		out << "; NoKillmoves - settings\n"
			   "; Also editable in game: Mod Control Panel (SKSE Menu Framework) > NoKillmoves.\n"
			   "; The in-game menu rewrites this file whenever you change a setting.\n"
			   "; true = switched off.\n"
			   "\n"
			   "[General]\n"
			   "; Your kill moves while in third person. Sneak kills (throat slit) always stay on.\n"
			<< "bDisablePlayerThirdPerson = " << Bool(disablePlayerThirdPerson) << "\n"
			<< "; Your kill moves while in first person. Sneak kills (throat slit) always stay on.\n"
			<< "bDisablePlayerFirstPerson = " << Bool(disablePlayerFirstPerson) << "\n"
			<< "; Kill moves by NPCs, creatures and dragons - on you and on each other.\n"
			<< "bDisableNPC = " << Bool(disableNPC) << "\n"
			<< "; The slow-motion kill cam when your spell kills the last enemy.\n"
			<< "bDisableMagicKillCam = " << Bool(disableMagicKillCam) << "\n"
			<< "; The slow-motion kill cam that follows your arrow or bolt.\n"
			<< "bDisableBowKillCam = " << Bool(disableBowKillCam) << "\n";
		if (!out) {
			logs::error("Write to {} failed", kPath.string());
			return;
		}
	}
	LogSettings("Settings saved");
}
