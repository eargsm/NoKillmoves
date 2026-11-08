#pragma once

namespace Settings
{
	inline std::atomic<bool> disablePlayerThirdPerson{ true };
	inline std::atomic<bool> disablePlayerFirstPerson{ true };
	inline std::atomic<bool> disableNPC{ true };
	inline std::atomic<bool> disableKillMoveCam{ false };
	inline std::atomic<bool> disableMagicKillCam{ true };
	inline std::atomic<bool> disableBowKillCam{ false };

	void Load();
	void Save();
}
