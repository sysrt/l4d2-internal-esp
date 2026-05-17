#include "framework.h"

class C_LocalPlayer {
public:
    DWORD p_Base = NULL;

    int m_iTeamNum;
    int m_iHealth;
    Vector3 m_vecOrigin;

    void Update() {
        p_Base = *(DWORD*)((DWORD)server_dll + 0x7E0774);
        if (!p_Base) return;

        m_iTeamNum = *(int*)(p_Base + 0x238);
        m_iHealth = *(int*)(p_Base + 0xEC);
        m_vecOrigin = {
            *(float*)(p_Base + 0x388),
            *(float*)(p_Base + 0x38C),
            *(float*)(p_Base + 0x390)
        };
    }
};

class C_Entity {
public:
    DWORD p_Base = NULL;

    DWORD m_clrRender;
    DWORD m_Glow;

    int m_iTeamNum;
    int m_iHealth;
    int m_lifeState;
    int m_iMaxHealth;
    const char* m_typeName;

    const char* p_iName;
    char iNameBuff[128] = { 0 };

    Vector3 m_vecOrigin;

    void Update() {
        if (p_Base == NULL) return;

        m_iTeamNum = *(int*)(p_Base + 0x238);
        m_iHealth = *(int*)(p_Base + 0xEC);
        m_iMaxHealth = *(int*)(p_Base + 0xE8);
        m_lifeState = *(int*)(p_Base + 0xF0);

        iNameBuff[0] = '\0';

        __try {
            p_iName = *(const char**)(p_Base + 0x24C);

            if (p_iName != NULL && (DWORD)p_iName > 0x10000 && (DWORD)p_iName < 0x7FFFFFFF) {
                strncpy_s(iNameBuff, sizeof(iNameBuff), p_iName, 127);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            iNameBuff[0] = '\0';
        }

        DetermineType();

        m_clrRender = p_Base + 0x110;
        m_Glow = p_Base + 0x1DC;

        m_vecOrigin = {
            *(float*)((DWORD)p_Base + (DWORD)0x388),
            *(float*)((DWORD)p_Base + (DWORD)0x38C),
            *(float*)((DWORD)p_Base + (DWORD)0x390)
        };
    }
    bool IsValid() const {
        return m_iTeamNum > 1 && m_iHealth > 1;
    }

    void DetermineType() {
        m_typeName = "Unknown";

        if (iNameBuff[0] == '\0') return;

        if (strstr(iNameBuff, "hulk")) {
            m_typeName = "Tank";
        }
        else if (strstr(iNameBuff, "hunter")) {
            m_typeName = "Hunter";
        }
        else if (strstr(iNameBuff, "smoker")) {
            m_typeName = "Smoker";
        }
        else if (strstr(iNameBuff, "boomer")) {
            m_typeName = "Boomer";
        }
        else if (strstr(iNameBuff, "spitter")) {
            m_typeName = "Spitter";
        }
        else if (strstr(iNameBuff, "jockey")) {
            m_typeName = "Jockey";
        }
        else if (strstr(iNameBuff, "charger")) {
            m_typeName = "Charger";
        }
        else if (strstr(iNameBuff, "witch")) {
            m_typeName = "Witch";
        }
        else if (strstr(iNameBuff, "infected/")) {
            m_typeName = "Common";
        }
        else if (m_iTeamNum == 2) {
            m_typeName = "Survivor";
        }
    }
};