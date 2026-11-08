#include "Killmoves.h"
#include "Settings.h"

namespace
{
	RE::SCRIPT_FUNCTION::Condition_t* g_original{ nullptr };

	RE::TESObjectWEAP* AsWeapon(RE::TESForm* a_form)
	{
		return a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;
	}

	bool IsBowOrCrossbow(RE::TESForm* a_form)
	{
		const auto weapon = AsWeapon(a_form);
		return weapon && (weapon->IsBow() || weapon->IsCrossbow());
	}

	bool IsSneakKillSetup(RE::PlayerCharacter* a_player)
	{
		const auto weapon = AsWeapon(a_player->GetEquippedObject(false));
		return a_player->IsSneaking() && weapon && (weapon->IsOneHandedDagger() || weapon->IsOneHandedSword());
	}

	bool KillMoveAllowed(RE::TESObjectREFR* a_attacker)
	{
		if (!a_attacker) {
			return true;
		}
		if (!a_attacker->IsPlayerRef()) {
			return !Settings::disableNPC;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (IsSneakKillSetup(player)) {
			return true;
		}
		if (IsBowOrCrossbow(player->GetEquippedObject(false)) || IsBowOrCrossbow(player->GetEquippedObject(true))) {
			return !Settings::disablePlayerBow;
		}

		const auto camera = RE::PlayerCamera::GetSingleton();
		const bool firstPerson = camera && camera->IsInFirstPerson();
		return firstPerson ? !Settings::disablePlayerFirstPerson : !Settings::disablePlayerThirdPerson;
	}

	bool ShouldAttackKill(RE::TESObjectREFR* a_thisObj, void* a_param1, void* a_param2, double& a_result)
	{
		const bool ok = g_original(a_thisObj, a_param1, a_param2, a_result);
		if (ok && a_result != 0.0 && !KillMoveAllowed(a_thisObj)) {
			a_result = 0.0;
		}
		return ok;
	}
}

bool Killmoves::Install()
{
	if (g_original) {
		return true;
	}

	const auto function = RE::SCRIPT_FUNCTION::LocateScriptCommand("ShouldAttackKill");
	if (!function || !function->conditionFunction) {
		logs::error("ShouldAttackKill condition not found - kill moves are NOT being changed");
		return false;
	}

	const auto index = function - RE::SCRIPT_FUNCTION::GetFirstScriptCommand();
	if (index != std::to_underlying(RE::FUNCTION_DATA::FunctionID::kShouldAttackKill)) {
		logs::error("ShouldAttackKill found at table index {} instead of {} - kill moves are NOT being changed", index,
			std::to_underlying(RE::FUNCTION_DATA::FunctionID::kShouldAttackKill));
		return false;
	}

	g_original = function->conditionFunction;
	function->conditionFunction = ShouldAttackKill;
	logs::info("ShouldAttackKill wrapped (table index {})", index);
	return true;
}

bool Killmoves::IsInstalled()
{
	return g_original != nullptr;
}
