#include "NND/NPCNameProvider.h"

namespace logger = SKSE::log;

namespace NND
{
	const char* NPCNameProvider::GetName(RE::Actor* actor) const
	{
		if (nnd) {
			if (auto name = nnd->GetName(actor, API::NameContext::kEnemyHUD); !name.empty()) {
				return name.data();
			}
		}

		return actor->GetDisplayFullName();
	}

	void NPCNameProvider::RequestAPI()
	{
		if (!nnd) {
			nnd = static_cast<API::IVNND1*>(API::RequestPluginAPI(API::InterfaceVersion::kV1));
			if (nnd) {
				logger::info("Obtained NND API - {0:x}", reinterpret_cast<uintptr_t>(nnd));
			} else {
				logger::warn("Failed to obtain NND API");
			}
		}
}
}
