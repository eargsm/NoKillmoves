#include "UI.h"
#include "KillCams.h"
#include "Killmoves.h"
#include "Settings.h"

#include "SKSEMenuFramework.h"

namespace
{
	bool SettingCheckbox(const char* a_label, std::atomic<bool>& a_value, const char* a_help)
	{
		bool       v = a_value.load();
		const bool changed = ImGuiMCP::Checkbox(a_label, &v);
		if (changed) {
			a_value = v;
			Settings::Save();
		}
		ImGuiMCP::Indent();
		ImGuiMCP::TextColored({ 1.0f, 0.85f, 0.35f, 1.0f }, "%s", a_help);
		ImGuiMCP::Unindent();
		return changed;
	}

	void __stdcall RenderSettings()
	{
		ImGuiMCP::SeparatorText("Kill moves");

		SettingCheckbox("Disable player kill moves - third person", Settings::disablePlayerThirdPerson,
			"Your kill moves in third person. Sneak kills stay on.");
		SettingCheckbox("Disable player kill moves - first person", Settings::disablePlayerFirstPerson,
			"Your kill moves in first person. Sneak kills stay on.");
		SettingCheckbox("Disable NPC kill moves", Settings::disableNPC,
			"Kill moves by NPCs, creatures and dragons.");
		const bool killMoveCamChanged = SettingCheckbox("Disable kill move camera", Settings::disableKillMoveCam,
			"The camera during your melee kill moves. The kill move still plays.");

		ImGuiMCP::Spacing();
		ImGuiMCP::SeparatorText("Kill cams");

		const bool magicChanged = SettingCheckbox("Disable magic kill cam", Settings::disableMagicKillCam,
			"The slow-motion when your spell kills the last enemy.");
		const bool bowChanged = SettingCheckbox("Disable bow kill cam", Settings::disableBowKillCam,
			"The slow-motion camera that follows your arrow.");

		if (magicChanged || bowChanged || killMoveCamChanged) {
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
