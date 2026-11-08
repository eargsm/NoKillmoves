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

	const char* Name(Source a_source)
	{
		switch (a_source) {
		case Source::kMagic:
			return "magic";
		case Source::kBow:
			return "bow";
		default:
			return "other";
		}
	}

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

	std::optional<Source> AttackSource(const SKSE::ActionEvent& a_event)
	{
		using Type = SKSE::ActionEvent::Type;
		switch (a_event.type.get()) {
		case Type::kSpellCast:
		case Type::kSpellFire:
			return Source::kMagic;
		case Type::kBowDraw:
		case Type::kBowRelease:
			return Source::kBow;
		case Type::kWeaponSwing:
		case Type::kVoiceFire:
			return FromForm(a_event.sourceForm);
		default:
			return std::nullopt;
		}
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
			if (!a_event || !a_event->actor || !a_event->actor->IsPlayerRef()) {
				return RE::BSEventNotifyControl::kContinue;
			}
			if (const auto source = AttackSource(*a_event); source && g_lastAttack.exchange(*source) != *source) {
				KillCams::Update();
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
	bool       off = g_iniValue;
	if (attack == Source::kMagic) {
		off = Settings::disableMagicKillCam;
	} else if (attack == Source::kBow) {
		off = Settings::disableBowKillCam;
	}
	if (g_vatsDisable->data.b != off) {
		g_vatsDisable->data.b = off;
		logs::info("Kill cams {} (last attack: {})", off ? "off" : "on", Name(attack));
	}
}
