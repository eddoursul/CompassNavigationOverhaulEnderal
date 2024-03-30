#include "Hooks.h"

#include "Settings.h"

#include "HUDMarkerManager.h"

namespace hooks
{
	bool UpdateQuests(const RE::HUDMarkerManager* a_hudMarkerManager, RE::HUDMarker::ScaleformData* a_markerData,
					  RE::NiPoint3* a_pos, const RE::RefHandle& a_refHandle, std::uint32_t a_markerGotoFrame,
					  RE::TESQuestTarget* a_questTarget)
	{
		// The game loops through active quest targets, and calls `AddMarker` for each one.
		// If multiple targets correspond to the same marker, `AddMarker` returns the 
		// previously-created marker (via `a_refHandle`), so we can iteratively
		// build the structure containing all the targets, objectives, etc. corresponding
		// to the marker.
		if (HUDMarkerManager::AddMarker(a_hudMarkerManager, a_markerData, a_pos, a_refHandle, a_markerGotoFrame))
		{
			RE::TESObjectREFR* marker = RE::TESObjectREFR::LookupByHandle(a_refHandle).get();

			auto player = RE::PlayerCharacter::GetSingleton();

			RE::BSTArray<RE::BGSInstancedQuestObjective>& playerObjectives = REL::Module::IsVR() ?
																				player->GetVRPlayerRuntimeData().objectives :
																				player->GetPlayerRuntimeData().objectives;

			// The objectives are in oldest-to-newest order, so we iterate from newest-to-oldest
			// to have it in the same order as in the journal
			for (int ageIndex = playerObjectives.size() - 1; ageIndex >= 0; ageIndex--)
			{
				RE::BGSInstancedQuestObjective* playerObjective = &playerObjectives[ageIndex];
				RE::BGSQuestObjective* questObjective = playerObjective->objective;

				for (int j = 0; j < questObjective->numTargets; j++)
				{
					auto questObjectiveTarget = reinterpret_cast<RE::TESQuestTarget*>(a_questTarget->unk00);

					if (questObjectiveTarget == questObjective->targets[j])
					{
						RE::TESQuest* quest = questObjective->ownerQuest;
						CNO::HUDMarkerManager::GetSingleton()->ProcessQuestMarker(quest, playerObjective, ageIndex,
																				  marker, a_markerGotoFrame);
						return true;
					}
				}
			}

			return true;
		}

		return false;
	}

	RE::TESWorldSpace* AllowedToShowMapMarker(const RE::TESObjectREFR* a_marker)
	{
		RE::TESWorldSpace* markerWorldspace = a_marker->GetWorldspace();

		if (settings::display::showInteriorMarkers)
		{
			auto player = RE::PlayerCharacter::GetSingleton();

			RE::TESWorldSpace* playerWorldspace = player->GetWorldspace();

			if (playerWorldspace && markerWorldspace && playerWorldspace != markerWorldspace)
			{
				if (!playerWorldspace->parentWorld && markerWorldspace->parentWorld == playerWorldspace) 
				{
					return playerWorldspace;
				}
			}
		}
			
		return markerWorldspace;
	}

	bool UpdateLocations(const RE::HUDMarkerManager* a_hudMarkerManager, RE::HUDMarker::ScaleformData* a_markerData,
							   RE::NiPoint3* a_pos, const RE::RefHandle& a_refHandle, std::uint32_t a_markerGotoFrame)
	{
		RE::TESObjectREFR* marker = RE::TESObjectREFR::LookupByHandle(a_refHandle).get();
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

		RE::NiPoint3 markerPos = util::GetRealPosition(marker);
		RE::NiPoint3 playerPos = util::GetRealPosition(player);

		float sqDistanceToMarker = playerPos.GetSquaredDistance(markerPos);

		if (sqDistanceToMarker < RE::HUDMarkerManager::GetSingleton()->sqRadiusToAddLocation)
		{
			auto mapMarker = marker->extraList.GetByType<RE::ExtraMapMarker>();

			// Unvisited markers keep being shown in any case
			if (settings::display::showUndiscoveredLocationMarkers || mapMarker->mapData->flags.all(RE::MapMarkerData::Flag::kVisible))
			{
				if (HUDMarkerManager::AddMarker(a_hudMarkerManager, a_markerData, a_pos, a_refHandle, a_markerGotoFrame)) 
				{
					CNO::HUDMarkerManager::GetSingleton()->ProcessLocationMarker(mapMarker, marker, a_markerGotoFrame);

					return true;
				}
			}
		}

		return false;
	}

	bool UpdateEnemies(const RE::HUDMarkerManager* a_hudMarkerManager, RE::HUDMarker::ScaleformData* a_markerData,
							RE::NiPoint3* a_pos, const RE::RefHandle& a_refHandle, std::uint32_t a_markerGotoFrame)
	{
		if (settings::display::showEnemyMarkers)
		{
			if (HUDMarkerManager::AddMarker(a_hudMarkerManager, a_markerData, a_pos, a_refHandle, a_markerGotoFrame)) 
			{
				RE::TESObjectREFR* marker = RE::TESObjectREFR::LookupByHandle(a_refHandle).get();

				CNO::HUDMarkerManager::GetSingleton()->ProcessEnemyMarker(marker->As<RE::Character>(), a_markerGotoFrame);

				return true;
			}
		}

		return false;
	}

	bool UpdatePlayerSetMarker(const RE::HUDMarkerManager* a_hudMarkerManager, RE::HUDMarker::ScaleformData* a_markerData,
								RE::NiPoint3* a_pos, const RE::RefHandle& a_refHandle, std::uint32_t a_markerGotoFrame)
	{
		if (HUDMarkerManager::AddMarker(a_hudMarkerManager, a_markerData, a_pos, a_refHandle, a_markerGotoFrame)) 
		{
			RE::TESObjectREFR* marker = RE::TESObjectREFR::LookupByHandle(a_refHandle).get();

			CNO::HUDMarkerManager::GetSingleton()->ProcessPlayerSetMarker(marker, a_markerGotoFrame);

			return true;
		}

		return false;
	}

	void UpdateCompass(RE::Compass* a_compass)
	{
		hooks::Compass::Update(a_compass);

		CNO::HUDMarkerManager::GetSingleton()->SetMarkersExtraInfo();
	}

	namespace compat
	{
		RE::GFxMovieDef* MapMarkerFramework::GetCompassMovieDef()
		{
			return compassMovieDef;
		}
	}
}
