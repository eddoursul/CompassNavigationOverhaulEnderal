#include "HUDMarkerManager.h"

#include "RE/B/BSTimer.h"

#include "RE/I/IMenu.h"

#include "NND/NPCNameProvider.h"

namespace CNO
{
	void HUDMarkerManager::ProcessQuestMarker(RE::TESQuest* a_quest, RE::BGSInstancedQuestObjective* a_questObjective,
											  int a_questAgeIndex, RE::TESObjectREFR* a_marker, std::uint32_t a_markerIcon)
	{
		float angleToPlayerCamera = GetAngleBetween(playerCamera, a_marker);

		if ((IsTheFocusedMarker(a_marker) && angleToPlayerCamera < keepFocusedAngle) ||
			angleToPlayerCamera < facingAngle)
		{
			std::string description;

			if (settings::display::showObjectiveAsTarget)
			{
				description = a_questObjective->GetDisplayTextWithReplacedTags().c_str();
			}
			else
			{
				// A quest marker can reference to a character or a location
				switch (a_marker->GetFormType())
				{
				case RE::FormType::Reference:
					if (auto teleportDoor = a_marker->As<RE::TESObjectREFR>())
					{
						// If it is a teleport door, we can get the door at the other side
						if (auto teleportLinkedDoor = teleportDoor->extraList.GetTeleportLinkedDoor().get())
						{
							// First, try interior cell
							if (RE::TESObjectCELL* cell = teleportLinkedDoor->GetParentCell())
							{
								description = cell->GetName();
							}
							// Exterior cell
							else if (RE::TESWorldSpace* worldSpace = teleportLinkedDoor->GetWorldspace())
							{
								description = worldSpace->GetName();
							}
						}
					}
					break;
				case RE::FormType::ActorCharacter:
					if (auto character = a_marker->As<RE::Character>())
					{
						description = NND::NPCNameProvider::GetSingleton()->GetName(character);
					}
					break;
				}
			}

			facedMarkers.emplace_back(a_marker, angleToPlayerCamera,
									  hudMarkerManager->currentMarkerIndex - 1,
									  a_markerIcon, description);

			RE::QUEST_DATA::Type questType = a_quest->GetType();

			bool isInSameLocation = a_markerIcon == RE::HUDMarker::FrameOffsets::GetSingleton()->quest;

			QuestItem* questItem;

			if (questType == RE::QUEST_DATA::Type::kMiscellaneous)
			{
				if (!miscQuestItem.contains(a_marker))
				{
					miscQuestItem[a_marker] = QuestItem{ a_marker, questType, "$MISCELLANEOUS", isInSameLocation, a_questAgeIndex };
				}

				questItem = &miscQuestItem[a_marker];
			}
			else
			{
				RE::BSString questFullName = a_quest->GetFullName();
				ReplaceTagsInQuestText(&questFullName, a_quest, a_quest->currentInstanceID);

				questItems[a_marker][a_quest] = QuestItem{ a_marker, questType, questFullName.c_str(), isInSameLocation, a_questAgeIndex };

				questItem = &questItems[a_marker][a_quest];
			}

			if (std::ranges::find(questItem->objectives, a_questObjective) == questItem->objectives.end())
			{
				questItem->objectives.push_back(a_questObjective);
			}
		}
	}

	void HUDMarkerManager::ProcessLocationMarker(RE::ExtraMapMarker* a_mapMarker, RE::TESObjectREFR* a_marker,
												 std::uint32_t a_markerIcon)
	{
		float angleToPlayerCamera = GetAngleBetween(playerCamera, a_marker);

		bool isDiscoveredLocation = a_mapMarker->mapData->flags.all(RE::MapMarkerData::Flag::kVisible);

		if (isDiscoveredLocation || !settings::display::undiscoveredMeansUnknownInfo)
		{
			if ((IsTheFocusedMarker(a_marker) && angleToPlayerCamera < keepFocusedAngle) ||
				angleToPlayerCamera < facingAngle)
			{
				std::string_view locationFullName = a_mapMarker->mapData->locationName.GetFullName();

				facedMarkers.emplace_back(a_marker, angleToPlayerCamera,
										  hudMarkerManager->currentMarkerIndex - 1,
										  a_markerIcon, locationFullName);
			}
		}

		if (!isDiscoveredLocation && settings::display::undiscoveredMeansUnknownMarkers)
		{
			hudMarkerManager->scaleformMarkerData[hudMarkerManager->currentMarkerIndex - 1].icon.SetNumber(0);
		}
	}

	void HUDMarkerManager::ProcessEnemyMarker(RE::Character* a_enemy, std::uint32_t a_markerIcon)
	{
		float angleToPlayerCamera = GetAngleBetween(playerCamera, a_enemy);

		if ((IsTheFocusedMarker(a_enemy) && angleToPlayerCamera < keepFocusedAngle) ||
			angleToPlayerCamera < facingAngle)
		{
			std::string enemyName = NND::NPCNameProvider::GetSingleton()->GetName(a_enemy);

			facedMarkers.emplace_back(a_enemy, angleToPlayerCamera,
									hudMarkerManager->currentMarkerIndex - 1,
									a_markerIcon, enemyName);
		}
	}

	void HUDMarkerManager::ProcessPlayerSetMarker(RE::TESObjectREFR* a_marker, std::uint32_t a_markerIcon)
	{
		float angleToPlayerCamera = GetAngleBetween(playerCamera, a_marker);

		if ((IsTheFocusedMarker(a_marker) && angleToPlayerCamera < keepFocusedAngle) ||
			angleToPlayerCamera < facingAngle)
		{
			facedMarkers.emplace_back(a_marker, angleToPlayerCamera,
										hudMarkerManager->currentMarkerIndex - 1,
										a_markerIcon, "");
		}
	}

	void HUDMarkerManager::SetMarkersExtraInfo()
	{
		bool focusChanged = UpdateFocusedMarker();

		if (focusChanged)
		{
			compass->UnfocusMarker();
			timeFocusingMarker = 0.0F;
		}
		else if (focusedMarker)
		{
			timeFocusingMarker += timeManager->realTimeDelta;
		}

		bool isFocusedQuestMarker = false;

		if (focusedMarker)
		{
			std::string focusedMarkerDescription = focusedMarker->description;

			if (settings::display::showObjectiveAsTarget && settings::display::showOtherObjectivesCount)
			{
				int objectivesCount = 0;

				if (questItems.contains(focusedMarker->ref))
				{
					isFocusedQuestMarker = true;

					std::unordered_map<RE::TESQuest*, QuestItem>& questItemMap = questItems[focusedMarker->ref];

					for (auto& [quest, questItem] : questItemMap)
					{
						objectivesCount += questItem.objectives.size();
					}
				}

				if (miscQuestItem.contains(focusedMarker->ref))
				{
					isFocusedQuestMarker = true;

					QuestItem& questItem = miscQuestItem[focusedMarker->ref];

					objectivesCount += questItem.objectives.size();
				}

				if (objectivesCount > 1)
				{
					focusedMarkerDescription += " (+" + std::to_string(objectivesCount - 1) + ")";
				}
			}

			compass->SetFocusedMarkerInfo(focusedMarkerDescription, focusedMarker->distanceToPlayer,
										  focusedMarker->heightDifference, focusedMarker->index);

			if (focusChanged)
			{
				compass->FocusMarker();
			}

			compass->UpdateFocusedMarker();
		}

		RE::ActorState* playerState = player->AsActorState();

		bool canQuestItemListBeDisplayed = questItemList->CanBeDisplayed(player->GetParentCell(), playerState->IsWeaponDrawn());

		if (!canQuestItemListBeDisplayed || focusChanged)
		{
			questItemList->RemoveAllQuests();
		}

		if (canQuestItemListBeDisplayed && isFocusedQuestMarker)
		{
			if (focusChanged)
			{
				if (questItems.contains(focusedMarker->ref))
				{
					std::unordered_map<RE::TESQuest*, QuestItem>& questItemMap = questItems[focusedMarker->ref];

					for (auto& [quest, questItem] : questItemMap)
					{
						questItemList->AddQuest(questItem);
						questItemList->SetQuestSide(GetSideInQuest(questItem.type));

						// If we call a function more than once per frame (like in this for-loop)
						// we need to update the stage with `GFxMovieView::Advance`, otherwise graphical
						// glitches occur to the element when showing up
						questItemList->GetMovieView()->Advance(0.0F);
					}
				}
				
				if (miscQuestItem.contains(focusedMarker->ref))
				{
					QuestItem& questItem = miscQuestItem[focusedMarker->ref];

					questItemList->AddQuest(questItem);

					// If we call a function more than once per frame (like in this for-loop)
					// we need to update the stage with `GFxMovieView::Advance`, otherwise graphical
					// glitches occur to the element when showing up
					questItemList->GetMovieView()->Advance(0.0F);
				}
			}

			questItemList->SetHiddenByForce(false);

			float playerSpeed = playerState->DoGetMovementSpeed();

			float delayToShow = (playerSpeed < player->GetWalkSpeed()) ? settings::questlist::walkingDelayToShow :
								(playerSpeed < player->GetJogSpeed())  ? settings::questlist::joggingDelayToShow :
																		 settings::questlist::sprintingDelayToShow;

			if (timeFocusingMarker > delayToShow)
			{
				questItemList->ShowAllQuests();
				questItemList->Update();
			}
		}

		facedMarkers.clear();
		questItems.clear();
		miscQuestItem.clear();
	}

	std::unique_ptr<Compass::Marker> HUDMarkerManager::GetMostCenteredMarker() const
	{
		std::unique_ptr<Compass::Marker> mostCenteredMarker = nullptr;

		float closestAngleToPlayerCamera = std::numeric_limits<float>::max();

		int mostCenteredMarkerIndex = -1;

		for (int i = 0; i < facedMarkers.size(); i++)
		{
			const Compass::Marker& facedMarker = facedMarkers[i];

			if (facedMarker.angleToPlayerCamera < closestAngleToPlayerCamera)
			{
				mostCenteredMarkerIndex = i;
				closestAngleToPlayerCamera = facedMarker.angleToPlayerCamera;
			}
		}

		if (mostCenteredMarkerIndex >= 0)
		{
			mostCenteredMarker = std::make_unique<Compass::Marker>(facedMarkers[mostCenteredMarkerIndex]);
		}

		return mostCenteredMarker;
	}

	bool HUDMarkerManager::UpdateFocusedMarker()
	{
		std::unique_ptr<Compass::Marker> mostCenteredMarker = GetMostCenteredMarker();

		static auto IsMarkerDifferent = [](const std::unique_ptr<Compass::Marker>& a_lhs, const std::unique_ptr<Compass::Marker>& a_rhs) -> bool
		{
			if (a_lhs && a_rhs)
			{
				return a_lhs->ref != a_rhs->ref;
			}
			else if (!a_lhs && !a_rhs)
			{
				return false;
			}

			return true;
		};

		if (IsMarkerDifferent(mostCenteredMarker, preFocusedMarker))
		{
			timePreFocusingMarker = 0.0F;
		}

		if (preFocusedMarker || mostCenteredMarker)
		{
			preFocusedMarker = std::move(mostCenteredMarker);
		}

		if (IsMarkerDifferent(preFocusedMarker, focusedMarker))
		{
			if (preFocusedMarker)
			{
				if (timePreFocusingMarker > settings::display::focusingDelayToShow)
				{	
					focusedMarker = std::move(preFocusedMarker);
					return true;
				}
				else
				{
					timePreFocusingMarker += timeManager->realTimeDelta;
				}
			}
			else
			{
				focusedMarker = nullptr;
				return true;
			}
		}
		else if (preFocusedMarker && focusedMarker)
		{
			focusedMarker = std::move(preFocusedMarker);
		}

		return false;
	}

	float HUDMarkerManager::GetAngleBetween(const RE::PlayerCamera* a_playerCamera,
											const RE::TESObjectREFR* a_marker) const
	{
		float angleToPlayerCameraInRadians = util::GetAngleBetween(a_playerCamera, a_marker);
		float angleToPlayerCamera = util::RadiansToDegrees(angleToPlayerCameraInRadians);

		if (angleToPlayerCamera > 180.0F)
			angleToPlayerCamera = 360.0F - angleToPlayerCamera;

		return angleToPlayerCamera;
	}

	bool HUDMarkerManager::IsPlayerAllyOfFaction(const RE::TESFaction* a_faction) const
	{
		if (player->IsInFaction(a_faction)) 
		{
			return true;
		}

		return player->VisitFactions([a_faction](RE::TESFaction* a_visitedFaction, std::int8_t a_rank) -> bool
		{
			if (a_visitedFaction == a_faction && a_rank > 0)
			{
				return true;
			}

			for (RE::GROUP_REACTION* reactionToFaction : a_visitedFaction->reactions)
			{
				auto relatedFaction = reactionToFaction->form->As<RE::TESFaction>();
				if (relatedFaction == a_faction && reactionToFaction->fightReaction >= RE::FIGHT_REACTION::kAlly)
				{
					return true;
				}
			}

			return false;
		});
	}

	bool HUDMarkerManager::IsPlayerOpponentOfFaction(const RE::TESFaction* a_faction) const
	{
		return player->VisitFactions([a_faction](RE::TESFaction* a_visitedFaction, std::int8_t a_rank) -> bool
		{
			if (a_visitedFaction == a_faction && a_rank < 0)
			{
				return true;
			}

			for (RE::GROUP_REACTION* reactionToFaction : a_visitedFaction->reactions)
			{
				auto relatedFaction = reactionToFaction->form->As<RE::TESFaction>();
				if (relatedFaction == a_faction && reactionToFaction->fightReaction == RE::FIGHT_REACTION::kEnemy)
				{
					return true;
				}
			}

			return false;
		});
	}

	std::string HUDMarkerManager::GetSideInQuest(RE::QUEST_DATA::Type a_questType) const
	{
		switch (a_questType)
		{
		case RE::QUEST_DATA::Type::kCivilWar:
			if (IsPlayerAllyOfFaction(sonsOfSkyrimFaction) || IsPlayerAllyOfFaction(stormCloaksFaction) ||
				IsPlayerOpponentOfFaction(imperialLegionFaction))
			{
				return "StormCloaks";
			}
			else
			{
				return "ImperialLegion";
			}
		case RE::QUEST_DATA::Type::kDLC01_Vampire:
			if (player->HasKeywordString("Vampire") || IsPlayerAllyOfFaction(vampireFaction) ||
				IsPlayerOpponentOfFaction(dawnGuardFaction))
			{
				return "Vampires";
			}
			else
			{
				return "Dawnguard"; 
			}
		}

		return { };
	}
}
