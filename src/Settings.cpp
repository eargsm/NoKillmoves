#include "Settings.h"

namespace
{
	struct Entry
	{
		const char*        key;
		std::atomic<bool>& value;
		const char*        help;
	};

	const std::array<Entry, 5> kEntries{ {
		{ "bDisablePlayerThirdPerson", Settings::disablePlayerThirdPerson,
			"Your kill moves while in third person. Sneak kills (throat slit) always stay on." },
		{ "bDisablePlayerFirstPerson", Settings::disablePlayerFirstPerson,
			"Your kill moves while in first person. Sneak kills (throat slit) always stay on." },
		{ "bDisableNPC", Settings::disableNPC, "Kill moves by NPCs, creatures and dragons - on you and on each other." },
		{ "bDisableMagicKillCam", Settings::disableMagicKillCam, "The slow-motion kill cam when your spell kills the last enemy." },
		{ "bDisableBowKillCam", Settings::disableBowKillCam, "The slow-motion kill cam that follows your arrow or bolt." },
	} };

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
		std::string values;
		for (const auto& entry : kEntries) {
			values += std::format(" {}={}", entry.key, Bool(entry.value));
		}
		logs::info("{}:{}", a_prefix, values);
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
		const auto eq = line.find('=');
		if (line.empty() || line[0] == ';' || line[0] == '#' || line[0] == '[' || eq == std::string::npos) {
			continue;
		}
		const auto key = Trim(line.substr(0, eq));
		auto       value = Trim(line.substr(eq + 1));
		if (const auto comment = value.find(';'); comment != std::string::npos) {
			value = Trim(value.substr(0, comment));
		}

		const auto entry = std::ranges::find(kEntries, key, &Entry::key);
		if (entry != kEntries.end()) {
			entry->value = ParseBool(value);
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
			   "[General]\n";
		for (const auto& entry : kEntries) {
			out << "; " << entry.help << "\n" << entry.key << " = " << Bool(entry.value) << "\n";
		}
		if (!out) {
			logs::error("Write to {} failed", kPath.string());
			return;
		}
	}
	LogSettings("Settings saved");
}
