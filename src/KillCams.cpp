#include "KillCams.h"
#include "Settings.h"

namespace
{
	enum class Source
	{
		kOther,
		kMagic,
		kBow
	};

	RE::Setting*        g_vatsDisable{ nullptr };
	bool                g_iniValue{ false };
	std::atomic<Source> g_lastAttack{ Source::kOther };

	Source FromForm(RE::TESForm* a_form)
	{
		if (!a_form) {
			return Source::kOther;
		}
		if (a_form->IsMagicItem()) {
			return Source::kMagic;
		}
		const auto weapon = a_form->As<RE::TESObjectWEAP>();
		if (!weapon) {
			return Source::kOther;
		}
		if (weapon->IsBow() || weapon->IsCrossbow()) {
			return Source::kBow;
		}
		return weapon->IsStaff() ? Source::kMagic : Source::kOther;
	}

	Source FromAction(const SKSE::ActionEvent& a_event)
	{
		switch (a_event.type.get()) {
		case SKSE::ActionEvent::Type::kSpellCast:
		case SKSE::ActionEvent::Type::kSpellFire:
			return Source::kMagic;
		case SKSE::ActionEvent::Type::kBowDraw:
		case SKSE::ActionEvent::Type::kBowRelease:
			return Source::kBow;
		default:
			return FromForm(a_event.sourceForm);
		}
	}

	bool IsAttack(SKSE::ActionEvent::Type a_type)
	{
		using Type = SKSE::ActionEvent::Type;
		return a_type == Type::kWeaponSwing || a_type == Type::kSpellCast || a_type == Type::kSpellFire || a_type == Type::kVoiceFire ||
		       a_type == Type::kBowDraw || a_type == Type::kBowRelease;
	}

	class ActionSink : public RE::BSTEventSink<SKSE::ActionEvent>
	{
	public:
		static ActionSink* Get()
		{
			static ActionSink sink;
			return &sink;
		}

		RE::BSEventNotifyControl ProcessEvent(const SKSE::ActionEvent* a_event, RE::BSTEventSource<SKSE::ActionEvent>*) override
		{
			if (a_event && a_event->actor && a_event->actor->IsPlayerRef() && IsAttack(a_event->type.get())) {
				const auto source = FromAction(*a_event);
				if (g_lastAttack.exchange(source) != source) {
					KillCams::Update();
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};
}

bool KillCams::Install()
{
	if (g_vatsDisable) {
		return true;
	}

	const auto collection = RE::INISettingCollection::GetSingleton();
	g_vatsDisable = collection ? collection->GetSetting("bVATSDisable:VATS") : nullptr;
	const auto events = SKSE::GetActionEventSource();
	if (!g_vatsDisable || !events) {
		g_vatsDisable = nullptr;
		logs::error("bVATSDisable or the action events are missing - kill cams are NOT being changed");
		return false;
	}

	g_iniValue = g_vatsDisable->GetBool();
	events->AddEventSink(ActionSink::Get());
	logs::info("Kill cam switch ready (bVATSDisable in your INI: {})", g_iniValue);
	Update();
	return true;
}

bool KillCams::IsInstalled()
{
	return g_vatsDisable != nullptr;
}

void KillCams::Reset()
{
	g_lastAttack = Source::kOther;
	Update();
}

void KillCams::Update()
{
	if (!g_vatsDisable) {
		return;
	}

	const auto attack = g_lastAttack.load();
	const bool off = g_iniValue || (attack == Source::kMagic && Settings::disableMagicKillCam) ||
	                 (attack == Source::kBow && Settings::disableBowKillCam);
	if (g_vatsDisable->data.b != off) {
		g_vatsDisable->data.b = off;
		logs::info("Kill cams {} (last attack: {})", off ? "off" : "on",
			attack == Source::kMagic ? "magic" : attack == Source::kBow ? "bow" : "other");
	}
}
