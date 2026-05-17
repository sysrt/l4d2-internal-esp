#include <iostream>
#include "framework.h"

#include <d3d9.h>

#include "kiero.h"
#include <assert.h>

#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_impl_win32.h"
#include "ext/imgui/imgui_impl_dx9.h"

#include "C_ClientEntityList.hpp"
#include "C_View.hpp"

typedef long(__stdcall* Reset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
static Reset oReset = NULL;

typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
static EndScene oEndScene = NULL;

static WNDPROC oWndProc = NULL;

bool init = false;
bool menuOpen = true;

C_ClientEntityList g_EntityList;
C_LocalPlayer g_LocalPlayer;

bool espEnable = true;
bool espBox = true;
bool espName = true;
bool espHealth = true;
bool espSnapline = false;
bool espDistance = true;
bool espGlow = false;

bool filterCommon = true;
bool filterSpecial = true;
bool filterSurvivor = false;
bool filterBoss = false;

ImVec4 colorCommon = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 colorSpecial = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
ImVec4 colorSurvivor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
ImVec4 colorBoss = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);

ImVec4 colorSnapline = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 colorBox = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
ImVec4 colorHealth = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
ImVec4 colorText = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

inline ImColor Vec4ToColor(const ImVec4& v) {
    return ImColor(v.x, v.y, v.z, v.w);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK hkWindowProc(
    _In_ HWND   hwnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
        menuOpen = !menuOpen;
        return 1L;
    }
    if (menuOpen && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) > 0)
        return 1L;
    return ::CallWindowProcA(oWndProc, hwnd, uMsg, wParam, lParam);
}

long __stdcall hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    long res = oReset(pDevice, pPresentationParameters);
    ImGui_ImplDX9_CreateDeviceObjects();
    return res;
}

void DrawEntityESP(const C_Entity& ent, const ImVec4& color, const Vector3& localPos) {
    Vector3 bottomPos = ent.m_vecOrigin;
    Vector3 topPos = ent.m_vecOrigin;
    topPos.z += 72.0f;

    Vector2 screenBottom = C_View::WorldToScreen(bottomPos);
    Vector2 screenTop = C_View::WorldToScreen(topPos);

    if (screenBottom.x <= 0 || screenBottom.y <= 0) return;
    if (screenTop.x <= 0 || screenTop.y <= 0) return;

    float boxHeight = screenBottom.y - screenTop.y;
    float boxWidth = boxHeight * 0.45f;

    ImVec2 topLeft(screenTop.x - boxWidth / 2, screenTop.y);
    ImVec2 bottomRight(screenTop.x + boxWidth / 2, screenBottom.y);
    ImVec2 center(screenTop.x, screenTop.y);

    auto draw = ImGui::GetBackgroundDrawList();

    if (espSnapline) {
        draw->AddLine(
            ImVec2(center.x, screenBottom.y),
            ImVec2((float)C_View::width / 2.0f, (float)C_View::height),
            Vec4ToColor(colorSnapline),
            1.0f
        );
    }

    if (espBox) {
        draw->AddRect(topLeft, bottomRight, Vec4ToColor(color), 0.0f, 0, 1.5f);
    }

    if (espHealth) {
        float hpBarX = topLeft.x - 6.0f;
        float hpBarW = 3.0f;
        float hpPercent = ent.m_iHealth / (float)ent.m_iMaxHealth;
        if (hpPercent > 1.0f) hpPercent = 1.0f;
        if (hpPercent < 0.0f) hpPercent = 0.0f;
        float hpBarH = boxHeight * hpPercent;

        draw->AddRectFilled(
            ImVec2(hpBarX - 1, topLeft.y - 1),
            ImVec2(hpBarX + hpBarW + 1, bottomRight.y + 1),
            ImColor(0, 0, 0, 150)
        );


        ImColor hpColor;
        if (hpPercent > 0.6f)
            hpColor = ImColor(0, 255, 0);
        else if (hpPercent > 0.3f)
            hpColor = ImColor(255, 255, 0);
        else
            hpColor = ImColor(255, 0, 0);

        draw->AddRectFilled(
            ImVec2(hpBarX, screenBottom.y - hpBarH),
            ImVec2(hpBarX + hpBarW, screenBottom.y),
            hpColor
        );

        char healthText[32];
        sprintf_s(healthText, "%d", ent.m_iHealth);
        draw->AddText(
            ImVec2(center.x - 25, bottomRight.y + 10),
            Vec4ToColor(colorText),
            healthText
        );
    }

    if (espName) {
        draw->AddText(
            ImVec2(center.x - 15, topLeft.y - 15),
            Vec4ToColor(color),
            ent.m_typeName
        );
    }
        

    if (espDistance) {
        float dist = sqrtf(
            powf(ent.m_vecOrigin.x - localPos.x, 2) +
            powf(ent.m_vecOrigin.y - localPos.y, 2) +
            powf(ent.m_vecOrigin.z - localPos.z, 2)
        );
        dist /= 39.0f;

        char distText[32];
        sprintf_s(distText, "%.0fm", dist);
        draw->AddText(
            ImVec2(center.x - 15, bottomRight.y + 2),
            Vec4ToColor(colorText),
            distText
        );
    }

    if (espGlow) {
        unsigned char r = (unsigned char)(color.x * 255);
        unsigned char g = (unsigned char)(color.y * 255);
        unsigned char b = (unsigned char)(color.z * 255);
        unsigned char a = (unsigned char)(color.w * 255);

        int gColor = (a << 24) | (r << 16) | (g << 8) | b;

        *(int*)ent.m_clrRender = gColor;
        *(int*)(ent.m_Glow + 0x04) = 3;
        *(int*)(ent.m_Glow + 0x10) = gColor;
    }
    else {
        *(int*)ent.m_clrRender = -1;
        *(int*)(ent.m_Glow + 0x04) = 0;
        *(int*)(ent.m_Glow + 0x10) = 0;
    }
}

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    if (!init)
    {
        D3DDEVICE_CREATION_PARAMETERS params;
        pDevice->GetCreationParameters(&params);

        oWndProc = (WNDPROC)::SetWindowLongPtr((HWND)params.hFocusWindow, GWLP_WNDPROC, (LONG)hkWindowProc);

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(params.hFocusWindow);
        ImGui_ImplDX9_Init(pDevice);

        init = true;
    }

    g_LocalPlayer.Update();
    g_EntityList.Update();
    C_View::Update();

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    Vector3 localPos = g_LocalPlayer.m_vecOrigin;
    if (!g_EntityList.Survivor.empty()) {
        localPos = g_EntityList.Survivor[0].m_vecOrigin;
    }

    if (espEnable) {
        if (filterCommon) {
            for (const auto& ent : g_EntityList.Common)
                DrawEntityESP(ent, colorCommon, localPos);
        }
        if (filterSpecial) {
            for (const auto& ent : g_EntityList.Special) {
                if (ent.p_Base == g_LocalPlayer.p_Base) continue;
                DrawEntityESP(ent, colorSpecial, localPos);
            }
        }
        if (filterBoss) {
            for (const auto& ent : g_EntityList.Boss) {
                if (ent.p_Base == g_LocalPlayer.p_Base) continue;
                DrawEntityESP(ent, colorBoss, localPos);
            }
        }
        if (filterSurvivor) {
            for (const auto& ent : g_EntityList.Survivor) {
                if (ent.p_Base == g_LocalPlayer.p_Base) continue;
                DrawEntityESP(ent, colorSurvivor, localPos);
            }
        }
    }

    if (menuOpen) {
        ImGui::Begin("###L4D4", &menuOpen, ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::CollapsingHeader("Main", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("ESP Enable", &espEnable);
            ImGui::Separator();

            ImGui::Checkbox("Box", &espBox);
            ImGui::SameLine();
            ImGui::ColorEdit4("##BoxColor", (float*)&colorBox, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::Checkbox("Name", &espName);
            ImGui::SameLine();
            ImGui::ColorEdit4("##TextColor", (float*)&colorText, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::Checkbox("Health bar", &espHealth);
            ImGui::SameLine();
            ImGui::ColorEdit4("##HealthColor", (float*)&colorHealth, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::Checkbox("Snapline", &espSnapline);
            ImGui::SameLine();
            ImGui::ColorEdit4("##SnaplineColor", (float*)&colorSnapline, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::Checkbox("Distance", &espDistance);

            ImGui::Separator();

            ImGui::Checkbox("Glow", &espGlow);
        }

        if (ImGui::CollapsingHeader("Filters")) {
            ImGui::Checkbox("Common", &filterCommon);
            ImGui::SameLine();
            ImGui::ColorEdit4("##CommonColor", (float*)&colorCommon, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::Checkbox("Special", &filterSpecial);
            ImGui::SameLine();
            ImGui::ColorEdit4("##SpecialColor", (float*)&colorSpecial, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::Checkbox("Survivor", &filterSurvivor);
            ImGui::SameLine();
            ImGui::ColorEdit4("##SurvivorColor", (float*)&colorSurvivor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::Checkbox("Boss", &filterBoss);
            ImGui::SameLine();
            ImGui::ColorEdit4("##BossColor", (float*)&colorBoss, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        }

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(pDevice);
}

void __stdcall Hkinit() {
    if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
    {
        kiero::bind(42, (void**)&oEndScene, hkEndScene);
        kiero::bind(16, (void**)&oReset, hkReset);
        return;
    }
}