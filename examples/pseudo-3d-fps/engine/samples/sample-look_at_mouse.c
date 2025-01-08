// TODO: Port to dh-engine

// #define DCE_DEBUG
#include "dce/dce.h"

int main(void) {
    dce_Initialize();
    getch();

    Vec2   aim     = MakeVec2(25.0f, 0.0f);
    Circle aimPath = Circ.make(V2.new(), V2.LengthOf(aim));

    while (dce_IsRunning()) {
        Screen_Clear(DCE_BLACK);

        Vec2  mousePos = Input_GetMousePosScreenToWorld();
        float angle    = V2.ShortestArcOf(aim, mousePos);
        aim            = Rot.v2MulV2(Rot.sincos(angle * Time_DeltaTime() * 2.0f), aim);

        if (Input_GetMouse(DCE_BTN_LT).isHeld) {
            // Screen_FillCircle(aimPath, DCE_WHITE);
            Screen_DrawArcRadians(aimPath, 0, V2.ShortestArcOf(V2.make(1, 0), aim), DCE_GREEN);
            Screen_FillTriangle(V2.new(), aim, V2.make(aim.x, 0), DCE_BLUE);
            Screen_DrawVector2D(V2.new(), aim, 0.5f, DCE_RED);
            Screen_FillBox(
                MakeAABB(
                    V2.sub(mousePos, V2.make(5.0f, 5.0f)),
                    V2.add(mousePos, V2.make(5.0f, 5.0f))
                ),
                DCE_WHITE
            );
        } else {
            Screen_DrawCircle(aimPath, DCE_WHITE);
            Screen_DrawTriangle(V2.new(), aim, V2.make(aim.x, 0), DCE_BLUE);
            Screen_DrawVector2D(V2.new(), aim, 0.5f, DCE_RED);
            Screen_DrawBox(
                MakeAABB(
                    V2.sub(mousePos, V2.make(5.0f, 5.0f)),
                    V2.add(mousePos, V2.make(5.0f, 5.0f))
                ),
                DCE_WHITE
            );
        }

        if (Input_GetKey(DCE_ESC).isPressed) {
            dce_Exit();
        }
    }

    return 0;
}
