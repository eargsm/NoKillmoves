#pragma once

namespace Settings
{
	inline std::atomic<bool> disablePlayerThirdPerson{ true };
	inline std::atomic<bool> disablePlayerFirstPerson{ true };
	inline std::atomic<bool> disablePlayerBow{ false };
	inline std::atomic<bool> disableNPC{ true };

	void Load();
	void Save();
}
