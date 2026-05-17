#pragma once
#include "framework.h"

namespace C_View {

	struct View_Matrix {
		float a11, a12, a13, a14;
		float a21, a22, a23, a24;
		float a31, a32, a33, a34;
		float a41, a42, a43, a44;
	};

	inline View_Matrix vm;

	inline float width, height;

	inline void Update() {

		HWND hwnd = FindWindowA("Valve001", NULL);
		if (hwnd) {
			RECT rect;
			GetClientRect(hwnd, &rect);
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;
		}

		DWORD p1 = *(DWORD*)((DWORD)engine_dll + 0x601FEC);
		if (!p1) return;

		DWORD pMatrix = p1 + 0x2E4; 

		memcpy(&vm, (void*)pMatrix, sizeof(vm));
	}

	inline Vector2 WorldToScreen(Vector3 world_pos) {
		Vector2 screen{ 0, 0 };

		float w = vm.a41 * world_pos.x + vm.a42 * world_pos.y + vm.a43 * world_pos.z + vm.a44;

		if (w < 0.001f) return screen;

		float x = vm.a11 * world_pos.x + vm.a12 * world_pos.y + vm.a13 * world_pos.z + vm.a14;
		float y = vm.a21 * world_pos.x + vm.a22 * world_pos.y + vm.a23 * world_pos.z + vm.a24;

		float camX = width / 2.0f;
		float camY = height / 2.0f;

		screen.x = camX + camX * x / w;
		screen.y = camY - camY * y / w;

		return screen;
	}
}