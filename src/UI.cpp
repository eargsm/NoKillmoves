#include "UI.h"
#include "KillCams.h"
#include "Killmoves.h"
#include "Settings.h"

#include "SKSEMenuFramework.h"

namespace
{
	void HelpMarker(const char* a_text)
	{
		ImGuiMCP::SameLine();
		ImGuiMCP::TextDisabled("(?)");
		if (ImGuiMCP::IsItemHovered()) {
			ImGuiMCP::SetTooltip("%s", a_text);
		}
	}

	bool SettingCheckbox(const char* a_label, std::atomic<bool>& a_value, const char* a_help)
	{
		bool       v = a_value.load();
		const bool changed = ImGuiMCP::Checkbox(a_label, &v);
		if (changed) {
			a_value = v;
			Settings::Save();
		}
		HelpMarker(a_help);
		return changed;
	}

	void __stdcall RenderSettings()
	{
		ImGuiMCP::SeparatorText("NoKillmoves");

		SettingCheckbox("Disable player kill moves - third person", Settings::disablePlayerThirdPerson,
			"Your kill moves while the camera is in third person.\nSneak kills (throat slit) always stay on.");
		SettingCheckbox("Disable player kill moves - first person", Settings::disablePlayerFirstPerson,
			"Your kill moves while the camera is in first person.\nSneak kills (throat slit) always stay on.");
		SettingCheckbox("Disable NPC kill moves", Settings::disableNPC,
			"Kill moves by NPCs, creatures and dragons - on you and on each other.");

		ImGuiMCP::Spacing();
		const bool magicChanged = SettingCheckbox("Disable magic kill cam", Settings::disableMagicKillCam,
			"The slow-motion kill cam when your spell kills the last enemy.");
		const bool bowChanged = SettingCheckbox("Disable bow kill cam", Settings::disableBowKillCam,
			"The slow-motion kill cam that follows your arrow or bolt.");

		if (magicChanged || bowChanged) {
			SKSE::GetTaskInterface()->AddTask(KillCams::Update);
		}

		if (!Killmoves::IsInstalled() || !KillCams::IsInstalled()) {
			ImGuiMCP::Spacing();
			ImGuiMCP::TextColored({ 1.0f, 0.4f, 0.4f, 1.0f },
				"Some switches could not hook into the game and do nothing. See NoKillmoves.log.");
		}
	}
}

void UI::Register()
{
	if (!SKSEMenuFramework::IsInstalled()) {
		logs::info("SKSE Menu Framework not installed - settings are INI-only");
		return;
	}
	SKSEMenuFramework::SetSection("NoKillmoves");
	SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);
	logs::info("Registered SKSE Menu Framework page");
}
