//
// Copyright (c) 2026 Crane3D project.
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
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Imgui/ImGuiObj.h>
#include <ImGui/imgui.h> 

#include "ImguiSample.h"

#include <Urho3D/DebugNew.h>

//=============================================================================
//=============================================================================
URHO3D_DEFINE_APPLICATION_MAIN(ImguiSample)

//=============================================================================
//=============================================================================
ImguiSample::ImguiSample(Context* context) :
    Sample(context),
    demoWindow_(true),
    clrColor_(0.45f, 0.55f, 0.60f, 1.00f)
{
}

void ImguiSample::Setup()
{
    engineParameters_["WindowTitle"] = GetTypeName();
    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetProgramDir() + "imgui.log";
    engineParameters_["FullScreen"] = false;
    engineParameters_["Headless"] = false;
    engineParameters_["WindowWidth"] = 1280;
    engineParameters_["WindowHeight"] = 720;
    engineParameters_["ResourcePaths"] = "Data;CoreData;";
}

void ImguiSample::Start()
{
    Sample::Start();

    InitImGui();

    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_FREE);
}

void ImguiSample::InitImGui()
{
    imguiSample_ = new ImGuiObj(context_);
    imguiSample_->SetDarkColorStyle(true);
    imguiSample_->SetScaleSize(true);
}

void ImguiSample::SubscribeToEvents()
{
    //imgui rendering cycle
    SubscribeToEvent(E_BEGINRENDERING, URHO3D_HANDLER(ImguiSample, HandleBeginRendering));
}

void ImguiSample::HandleBeginRendering(StringHash eventType, VariantMap& eventData)
{
    if (demoWindow_)
    {
        ImGui::ShowDemoWindow(&demoWindow_);
    }

    DemoWindowAlt();
}

void ImguiSample::DemoWindowAlt()
{
    //straight copy of the sample in ImGui
    static float f = 0.0f;
    static int counter = 0;

    ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &demoWindow_);      // Edit bools storing our window open/close state
    //ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&clrColor_); // Edit 3 floats representing a color

    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        counter++;
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();

}

