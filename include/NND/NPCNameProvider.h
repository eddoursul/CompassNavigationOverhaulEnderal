#pragma once

#include "NND/API.h"

namespace NND
{
	class NPCNameProvider
	{
	public:
		static NPCNameProvider* GetSingleton()
		{
			static NPCNameProvider singleton;
			return std::addressof(singleton);
		}

		const char* GetName(RE::Actor* actor) const;

		void RequestAPI();

	private:
		API::IVNND1* nnd = nullptr;

		NPCNameProvider() = default;
		NPCNameProvider(const NPCNameProvider&) = delete;
		NPCNameProvider(NPCNameProvider&&) = delete;

		~NPCNameProvider() = default;

		NPCNameProvider& operator=(const NPCNameProvider&) = delete;
		NPCNameProvider& operator=(NPCNameProvider&&) = delete;
	};
}