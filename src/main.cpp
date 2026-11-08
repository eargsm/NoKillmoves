#include "KillCams.h"
#include "Killmoves.h"
#include "Settings.h"
#include "UI.h"

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	logs::info("NoKillmoves loading");
	Settings::Load();

	SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* a_msg) {
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kPostLoad:
			UI::Register();
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			Killmoves::Install();
			KillCams::Install();
			break;
		case SKSE::MessagingInterface::kPostLoadGame:
		case SKSE::MessagingInterface::kNewGame:
			KillCams::Reset();
			break;
		default:
			break;
		}
	});

	return true;
}
