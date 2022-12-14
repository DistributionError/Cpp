#include "memory.h"
#include "vector.h"
#include <thread>

namespace offset {
	constexpr ::std::ptrdiff_t dwLocalPlayer = outdated;
	constexpr ::std::ptrdiff_t dwEntityList = outdated;

	constexpr ::std::ptrdiff_t dwClientState = outdated;
	constexpr ::std::ptrdiff_t dwClientState_ViewAngles = outdated;


	constexpr ::std::ptrdiff_t m_dwBoneMatrix = outdated;
	constexpr ::std::ptrdiff_t m_bDormant = outdated;
	constexpr ::std::ptrdiff_t m_iTeamNum = outdated;
	constexpr ::std::ptrdiff_t m_iHealth = outdated;
	constexpr ::std::ptrdiff_t m_vecOrigin = outdated;
	constexpr ::std::ptrdiff_t m_vecViewOffset = outdated;
	constexpr ::std::ptrdiff_t m_aimPunchAngle = outdated;
	constexpr ::std::ptrdiff_t m_bSpottedByMask = outdated;

}

constexpr Vector3 CalcuateAngle(
	const Vector3& localPosition,
	const Vector3& enemyPosition,
	const Vector3& viewAngles) noexcept

{
	return ((enemyPosition - localPosition).ToAngle() - viewAngles);
}

int main()
{
	const auto memory = Memory{ "csgo.exe" };

	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (!GetAsyncKeyState(VK_RBUTTON))
			continue;

		const auto& localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
		const auto& localTeam = memory.Read<std::int32_t>(localPlayer + offset::m_iTeamNum);

		const auto localEyePosition = memory.Read<Vector3>(localPlayer + offset::m_vecOrigin) +
			memory.Read<Vector3>(localPlayer + offset::m_vecViewOffset);

		const auto& clientState = memory.Read<std::uintptr_t>(engine + offset::dwClientState);

		const auto& viewAngles = memory.Read<Vector3>(clientState + offset::dwClientState_ViewAngles);
		const auto& aimPunch = memory.Read<Vector3>(localPlayer + offset::m_aimPunchAngle * 2);

		auto bestFov = 5.f;
		auto bestAngle = Vector3{ };

		for (auto i = 1; i <= 32; ++i)
		{
			const auto& player = memory.Read<std::uintptr_t>(client + offset::dwEntityList + i * 0x10);

			if (memory.Read<std::int32_t>(player + offset::m_iTeamNum) == localTeam)
				continue;

			if (memory.Read<bool>(player + offset::m_bDormant))
				continue;

			if (!memory.Read<std::int32_t>(player + offset::m_iHealth))
				continue;

			if (!memory.Read<bool>(player + offset::m_bSpottedByMask))
				continue;

			const auto boneMatrix = memory.Read<std::uintptr_t>(player + offset::m_dwBoneMatrix);


			const auto playerHeadPostion = Vector3{
			 memory.Read<float>(boneMatrix + 0x30 * 8 + 0x0c),
			 memory.Read<float>(boneMatrix + 0x30 * 8 + 0x1c),
			 memory.Read<float>(boneMatrix + 0x30 * 8 + 0x2c),
			};

			const auto angle = CalcuateAngle(
				localEyePosition,
				playerHeadPostion,
				viewAngles + aimPunch
			);

			const auto fov = std::hypot(angle.x, angle.y);

			if (fov < bestFov)
			{
				bestFov = fov;
				bestAngle = angle;
			}


			if (!bestAngle.IsZero())
				memory.Write<Vector3>(clientState + offset::dwClientState_ViewAngles, viewAngles + bestAngle / 3.f);

		}

	}

	return 0;
}
