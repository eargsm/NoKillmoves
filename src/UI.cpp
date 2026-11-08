#include "UI.h"
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

	void SettingCheckbox(const char* a_label, std::atomic<bool>& a_value, const char* a_help)
	{
		bool v = a_value.load();
		if (ImGuiMCP::Checkbox(a_label, &v)) {
			a_value = v;
			Settings::Save();
		}
		HelpMarker(a_help);
	}

	void __stdcall RenderSettings()
	{
		ImGuiMCP::SeparatorText("NoKillmoves");

		SettingCheckbox("Disable player kill moves - third person", Settings::disablePlayerThirdPerson,
			"Your kill moves while the camera is in third person.\nSneak kills (throat slit) always stay on.");
		SettingCheckbox("Disable player kill moves - first person", Settings::disablePlayerFirstPerson,
			"Your kill moves while the camera is in first person.\nSneak kills (throat slit) always stay on.");
		SettingCheckbox("Disable player bow kill moves", Settings::disablePlayerBow,
			"Your kill moves with a bow or crossbow in hand, in either camera.\nThe two switches above do not cover these.");
		SettingCheckbox("Disable NPC kill moves", Settings::disableNPC,
			"Kill moves by NPCs, creatures and dragons - on you and on each other.");

		if (!Killmoves::IsInstalled()) {
			ImGuiMCP::Spacing();
			ImGuiMCP::TextColored({ 1.0f, 0.4f, 0.4f, 1.0f },
				"Could not hook the kill move check - these switches do nothing. See NoKillmoves.log.");
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
