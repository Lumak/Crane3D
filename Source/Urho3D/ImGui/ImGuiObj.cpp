//
// Copyright (c) 2026 Crane3D project
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include "../Core/CoreEvents.h"
#include "../Core/Profiler.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsEvents.h"
#include "../Graphics/GraphicsImpl.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Texture2D.h"
#include "../Input/Input.h"
#include "../Input/InputEvents.h"
#include "../ImGui/ImGuiObj.h"
#include "../IO/Log.h"

#ifdef URHO3D_OPENGL
#include "../ImGui/OpenGL3/imgui_impl_opengl3.h"
#elif defined(URHO3D_D3D11)
#include "../ImGui/Direct3D11/imgui_impl_dx11.h"
#else
#include "../ImGui/Direct3D9/imgui_impl_dx9.h"
#endif

//#include <SDL/SDL_clipboard.h>
#include <ImGui/imgui.h>

//=============================================================================
//=============================================================================
#define DEFAULT_SCALESIZE   1.0f
#define TOUCHID_NONE        (-1)

namespace Urho3D
{
extern const char* UI_CATEGORY;

//=============================================================================
//=============================================================================
ImGuiObj::ImGuiObj(Context * context) :
    Object(context),
    touchID_(TOUCHID_NONE)
{
    //ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    //styles
    SetScaleSize(DEFAULT_SCALESIZE);

    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    Graphics *graphics = GetSubsystem<Graphics>();

#ifdef URHO3D_OPENGL
    ImGui_ImplOpenGL3_Init();
#elif defined(URHO3D_D3D11)
    ImGui_ImplDX11_Init(graphics->GetImpl()->GetDevice(), graphics->GetImpl()->GetDeviceContext());
#else
    ImGui_ImplDX9_Init(graphics->GetImpl()->GetDevice());
#endif

    //events to track
    SubscribeToEvent(E_DEVICELOST, URHO3D_HANDLER(ImGuiObj, HandleOnDeviceLost));
    SubscribeToEvent(E_RENDERUPDATE, URHO3D_HANDLER(ImGuiObj, HandleRenderUpdate));
    SubscribeToEvent(E_BEGINRENDERING, URHO3D_HANDLER(ImGuiObj, HandleBeginRendering));
    SubscribeToEvent(E_ENDRENDERING, URHO3D_HANDLER(ImGuiObj, HandleEndRendering));

    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(ImGuiObj, HandleKeyUp));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ImGuiObj, HandleKeyDown));
    SubscribeToEvent(E_TEXTINPUT, URHO3D_HANDLER(ImGuiObj, HandleTextInput));

    SubscribeToEvent(E_TOUCHBEGIN, URHO3D_HANDLER(ImGuiObj, HandleTouchBegin));
    SubscribeToEvent(E_TOUCHEND, URHO3D_HANDLER(ImGuiObj, HandleTouchEnd));
    SubscribeToEvent(E_TOUCHMOVE, URHO3D_HANDLER(ImGuiObj, HandleTouchMove));

}

ImGuiObj::~ImGuiObj()
{
#ifdef URHO3D_OPENGL
    ImGui_ImplOpenGL3_Shutdown();
#elif defined(URHO3D_D3D11)
    ImGui_ImplDX11_Shutdown();
#else
    ImGui_ImplDX9_Shutdown();
#endif

    ImGui::DestroyContext();
}

void ImGuiObj::RegisterObject(Context* context)
{
}

void ImGuiObj::SetDarkColorStyle(bool dark)
{
    if (dark)
    {
        ImGui::StyleColorsDark();
    }
    else
    {
        ImGui::StyleColorsLight();
    }

}

void ImGuiObj::SetScaleSize(float scale)
{
    if (scale < 1.0f)
    {
        scale = 1.0f;
    }

    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

}

#if 0
const char* ImGuiContext::GetClipboardText(void* user_data)
{
    return SDL_GetClipboardText();
}

void ImGuiContext::SetClipboardText(void* user_data, const char * text)
{
    SDL_SetClipboardText(text);
}
#endif

void ImGuiObj::HandleOnDeviceLost(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_OPENGL
    //nothing here
#elif defined(URHO3D_D3D11)
    ImGui_ImplDX11_InvalidateDeviceObjects();
    ImGui_ImplDX11_CreateDeviceObjects();
#else
    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui_ImplDX9_CreateDeviceObjects();
#endif
}

void ImGuiObj::HandleRenderUpdate(StringHash eventType, VariantMap & eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    ImGuiIO &io = ImGui::GetIO();

    // Get display size (every frame for resizing)
    Graphics *graphics = GetSubsystem<Graphics>();
    io.DisplaySize = ImVec2((float)graphics->GetWidth(), (float)graphics->GetHeight());

    // Setup time step
    io.DeltaTime = timeStep > 0.0f ? timeStep : 1.0f/30.0f;

    // mouse input
    Input *input = GetSubsystem<Input>();
    if (input->IsMouseVisible() && !input->GetTouchEmulation())
    {
        IntVector2 pos = input->GetMousePosition();
        io.MousePos.x = (float)pos.x_;
        io.MousePos.y = (float)pos.y_;
    }
    else
    {
        io.MousePos.x = -1.0f;
        io.MousePos.y = -1.0f;
    }

    io.MouseDown[0] = input->GetMouseButtonDown(MOUSEB_LEFT);
    io.MouseDown[1] = input->GetMouseButtonDown(MOUSEB_RIGHT);
    io.MouseDown[2] = input->GetMouseButtonDown(MOUSEB_MIDDLE);
    io.MouseWheel = (float)input->GetMouseMoveWheel();

    // Modifiers
    io.KeyCtrl = input->GetQualifierDown(QUAL_CTRL);
    io.KeyShift = input->GetQualifierDown(QUAL_SHIFT);
    io.KeyAlt = input->GetQualifierDown(QUAL_ALT);

    // Start new ImGui frame
#ifdef URHO3D_OPENGL
    ImGui_ImplOpenGL3_NewFrame();
#elif defined(URHO3D_D3D11)
    ImGui_ImplDX11_NewFrame();
#else
    ImGui_ImplDX9_NewFrame();
#endif

    ImGui::NewFrame();

    URHO3D_PROFILE(ImGuiUpdate);
}

void ImGuiObj::HandleBeginRendering(StringHash eventType, VariantMap& eventData)
{
    //setup all imgui objects to render in this stage of the frame
}

void ImGuiObj::HandleEndRendering(StringHash eventType, VariantMap & eventData)
{
    //EndFrame() is automatically called by ImGui::Render(), no need to call manually
    //only called if there is no render() call
    //ImGui::EndFrame();

    ImGui::Render();

    //**NOTE** at this stage all Urho3D UI rendering has finished.
    //imgui rendering should happen here after Urho3D's UI render cycle, otherwise,
    //you get nothing but a black screen and will not see any imgui rendering.
#ifdef URHO3D_OPENGL
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#elif defined(URHO3D_D3D11)
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#else
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
#endif

    URHO3D_PROFILE(ImGuiRender);
}

void ImGuiObj::HandleKeyUp(StringHash eventType, VariantMap & eventData)
{
    using namespace KeyUp;
    int scancode = eventData[P_SCANCODE].GetInt();
    ImGuiIO &io = ImGui::GetIO();

    if (scancode < ImGuiKey::ImGuiKey_NamedKey_BEGIN)
    {
        io.KeysData[scancode].Down = false;
    }
}

void ImGuiObj::HandleKeyDown(StringHash eventType, VariantMap & eventData)
{
    using namespace KeyDown;
    int scancode = eventData[P_SCANCODE].GetInt();
    ImGuiIO &io = ImGui::GetIO();

    if (scancode < ImGuiKey::ImGuiKey_NamedKey_BEGIN)
    {
        io.KeysData[scancode].Down = true;
    }
}

void ImGuiObj::HandleTextInput(StringHash eventType, VariantMap & eventData)
{
    using namespace TextInput;
    const String &text = eventData[P_TEXT].GetString();
    ImGuiIO &io = ImGui::GetIO();
    io.AddInputCharactersUTF8(text.CString());
}

void ImGuiObj::HandleTouchBegin(StringHash eventType, VariantMap & eventData)
{
    using namespace TouchBegin;

    if (touchID_ == TOUCHID_NONE)
    {
        touchID_ = eventData[P_TOUCHID].GetInt();

        ImGuiIO &io = ImGui::GetIO();
        io.MousePos.x = (float)eventData[P_X].GetInt();
        io.MousePos.y = (float)eventData[P_Y].GetInt();
        io.MouseDown[0] = true;
    }
}

void ImGuiObj::HandleTouchEnd(StringHash eventType, VariantMap & eventData)
{
    using namespace TouchEnd;
    
    int touchID = eventData[P_TOUCHID].GetInt();
    
    if (touchID_ == touchID)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.MousePos.x = (float)eventData[P_X].GetInt();
        io.MousePos.y = (float)eventData[P_Y].GetInt();
        io.MouseDown[0] = false;
    }

    touchID_ = TOUCHID_NONE;
}

void ImGuiObj::HandleTouchMove(StringHash eventType, VariantMap & eventData)
{
    using namespace TouchMove;

    int touchID = eventData[P_TOUCHID].GetInt();

    if (touchID_ == touchID)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.MousePos.x = (float)eventData[P_X].GetInt();
        io.MousePos.y = (float)eventData[P_Y].GetInt();
    }
}

}//urho3d namespace