#include "framework.h"
#include "C_Entity.hpp"

class C_ClientEntityList {
	public:
		DWORD p_Base{ 0 };

		std::vector<C_Entity> Survivor;
		std::vector<C_Entity> Special;
		std::vector<C_Entity> Common;
        std::vector<C_Entity> Boss;

		void Update() {
            Survivor.clear();
			Special.clear();
			Common.clear();
            Boss.clear();

			DWORD offset = 0x10;
			p_Base = *(DWORD*)((DWORD)server_dll + (DWORD)0x75E9B8);

			if (p_Base == NULL) return;

			if (*(DWORD*)(p_Base + 0x04) == NULL) return; // world not loaded

            for (size_t i = 0; i < 2048; i++) {
                C_Entity ent;
                ent.p_Base = *(DWORD*)(p_Base + 0x14 + offset);
                offset += 0x10;

                if (ent.p_Base == NULL) continue;

                ent.Update();

                if (!ent.IsValid()) continue;

                switch (ent.m_iTeamNum) {
                case 2:
                    Survivor.push_back(ent);
                    break;
                case 3:
                    if (strcmp(ent.m_typeName, "Common") == 0)
                        Common.push_back(ent);
                    else if (strcmp(ent.m_typeName, "Witch") == 0 || strcmp(ent.m_typeName, "Tank") == 0)
                        Boss.push_back(ent);
                    else
                        Special.push_back(ent);
                }
            }
			return;
		}
};