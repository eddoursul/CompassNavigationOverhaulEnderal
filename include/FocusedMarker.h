#pragma once

#include "utils/Geometry.h"

struct FocusedMarker
{
	struct Data
	{
		Data(std::uint32_t a_gfxIndex, std::uint32_t a_gfxGotoFrame) :
			gfxIndex{ a_gfxIndex }, gfxGotoFrame{ a_gfxGotoFrame }
		{}

		virtual ~Data() = default;

		std::uint32_t gfxIndex;
		std::uint32_t gfxGotoFrame;
	};

	struct QuestData : Data
	{
		QuestData(std::uint32_t a_gfxIndex, std::uint32_t a_gfxGotoFrame, RE::TESObjectREFR* a_markerRef,
				  const RE::TESQuest* a_quest, const RE::BGSInstancedQuestObjective* a_instancedObjective);

		const RE::TESQuest* quest;
		const RE::BGSInstancedQuestObjective* instancedObjective;

		// cache
		RE::QUEST_DATA::Type type = quest->GetType();
		std::string name = (type == RE::QUEST_DATA::Type::kMiscellaneous) ? "$MISCELLANEOUS" : quest->GetName();
		std::string description = quest->GetCurrentDescriptionWithReplacedTags().c_str();
		std::string objective = instancedObjective->GetDisplayTextWithReplacedTags().c_str();
		std::string locationName;
		std::string characterName;
	};

	struct LocationData : Data
	{
		LocationData(std::uint32_t a_gfxIndex, std::uint32_t a_gfxGotoFrame, const RE::MapMarkerData* a_extraData) :
			Data{ a_gfxIndex, a_gfxGotoFrame }, extraData{ a_extraData }
		{}

		const RE::MapMarkerData* extraData;

		// cache
		std::string locationName = extraData->locationName.GetFullName();
	};

	struct EnemyData : Data
	{
		EnemyData(std::uint32_t a_gfxIndex, std::uint32_t a_gfxGotoFrame, const RE::Character* a_enemy) :
			Data{ a_gfxIndex, a_gfxGotoFrame }, enemy{ a_enemy }
		{}

		const RE::Character* enemy;

		// cache
		std::string enemyName = enemy->GetName();
	};

	struct PlayerSetData : Data
	{
		PlayerSetData(std::uint32_t a_gfxIndex, std::uint32_t a_gfxGotoFrame) :
			Data{ a_gfxIndex, a_gfxGotoFrame }
		{}

		std::string locationName = "";
	};

	FocusedMarker(const RE::TESObjectREFR* a_markerRef, float a_angleToPlayerCamera) :
		ref{ a_markerRef }, angleToPlayerCamera{ a_angleToPlayerCamera }
	{}

	const RE::TESObjectREFR* ref;
	float angleToPlayerCamera;
	float distanceToPlayer = util::GetDistanceBetween(RE::PlayerCharacter::GetSingleton(), ref);
	float heightDifference = util::GetHeightDifferenceBetween(RE::PlayerCharacter::GetSingleton(), ref);
	std::vector<std::shared_ptr<Data>> data;
};