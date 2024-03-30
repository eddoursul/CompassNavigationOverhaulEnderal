#pragma once

#include "RE/H/HUDMarkerManager.h"

#include "Settings.h"

#include "Compass.h"
#include "QuestItemList.h"

namespace CNO
{
	class HUDMarkerManager
	{
	public:
		static HUDMarkerManager* GetSingleton()
		{
			static HUDMarkerManager singleton;

			return &singleton;
		}

		void ProcessQuestMarker(RE::TESQuest* a_quest, RE::BGSInstancedQuestObjective* a_questObjective,
								int a_questAgeIndex, RE::TESObjectREFR* a_marker, std::uint32_t a_markerIcon);

		void ProcessLocationMarker(RE::ExtraMapMarker* a_mapMarker, RE::TESObjectREFR* a_marker,
								   std::uint32_t a_markerIcon);

		void ProcessEnemyMarker(RE::Character* a_enemy, std::uint32_t a_markerIcon);

		void ProcessPlayerSetMarker(RE::TESObjectREFR* a_marker, std::uint32_t a_markerIcon);

		void SetMarkersExtraInfo();

	private:

		bool IsTheFocusedMarker(const RE::TESObjectREFR* a_marker) const
		{
			return focusedMarker && a_marker == focusedMarker->ref;
		}

		std::unique_ptr<Compass::Marker> GetMostCenteredMarker() const;

		bool UpdateFocusedMarker();

		float GetAngleBetween(const RE::PlayerCamera* a_playerCamera, const RE::TESObjectREFR* a_marker) const;

		bool IsPlayerAllyOfFaction(const RE::TESFaction* a_faction) const;

		bool IsPlayerOpponentOfFaction(const RE::TESFaction* a_faction) const;

		std::string GetSideInQuest(RE::QUEST_DATA::Type a_questType) const;

		Compass* compass = Compass::GetSingleton();
		QuestItemList* questItemList = QuestItemList::GetSingleton();

		float facingAngle = settings::display::angleToShowMarkerDetails;
		float keepFocusedAngle = settings::display::angleToKeepMarkerDetailsShown;

		float timePreFocusingMarker = 0.0F;
		float timeFocusingMarker = 0.0F;

		std::vector<Compass::Marker> facedMarkers;
		std::unique_ptr<Compass::Marker> preFocusedMarker;
		std::unique_ptr<Compass::Marker> focusedMarker;

		std::unordered_map<RE::TESObjectREFR*, std::unordered_map<RE::TESQuest*, QuestItem>> questItems;
		std::unordered_map<RE::TESObjectREFR*, QuestItem> miscQuestItem;

		RE::HUDMarkerManager* const hudMarkerManager = RE::HUDMarkerManager::GetSingleton();
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		RE::PlayerCamera* playerCamera = RE::PlayerCamera::GetSingleton();
		RE::BSTimer* timeManager = RE::BSTimer::GetTimeManager();

		// Factions to lookup
		// Reference: Creation Kit -> Skyrim.esm, Dawnguard.esm
		const RE::TESFaction* const imperialLegionFaction = RE::TESForm::LookupByID(0x0002BF9A)->As<RE::TESFaction>();
		const RE::TESFaction* const stormCloaksFaction = RE::TESForm::LookupByID(0x00028849)->As<RE::TESFaction>();
		const RE::TESFaction* const sonsOfSkyrimFaction = RE::TESForm::LookupByID(0x0002BF9B)->As<RE::TESFaction>();
		const RE::TESFaction* const dawnGuardFaction = RE::TESForm::LookupByID(0x02014217)->As<RE::TESFaction>();
		const RE::TESFaction* const vampireFaction = RE::TESForm::LookupByID(0x02003376)->As<RE::TESFaction>();
	};
}