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

#pragma once

#include "Sample.h"

namespace Urho3D
{
    class Node;
    class Scene;
    class UIElement;
    class Button;
    class CheckBox;
}

/// Imgui sample
class ScaleUI : public Sample
{
    URHO3D_OBJECT(ScaleUI, Sample);

public:
    /// Construct.
    ScaleUI(Context* context);

    /// Setup after engine initialization and before running the main loop.
    virtual void Start();

protected:
    virtual void Setup();

private:
    void LoadMainUI();
    void LoadScaleMenu();
    void LoadBorderUI();
    void RegisterMenuButton(UIElement *elem, const char *str);
    void RegisterGroupCheckBox(UIElement *elem, const char *str);
    void AdjustUIScale();
    void AdjustUIBars();
    void HandleButtonReleased(StringHash eventType, VariantMap& eventData);
    void HandleListViewClicked(StringHash eventType, VariantMap& eventData);
    void HandleCheckboxToggle(StringHash eventType, VariantMap& eventData);


private:
    enum {NoScale, UIScale, FlexScale};

    int baseWidth_;
    int baseHeight_;
    SharedPtr<UIElement> mainMenu_;

    SharedPtr<UIElement> resMenu_;
    WeakPtr<DropDownList> dropDownListResolution_;
    int resWidth_;
    int resHeight_;

    SharedPtr<UIElement> topBar_;
    WeakPtr<Text> topBarText_;
    WeakPtr<BorderImage> topBarBorderImage_;

    SharedPtr<UIElement> rightBar_;

    SharedPtr<UIElement> scaleMenu_;
    PODVector<CheckBox*> checkBoxGrp_;
    int scaleType_;
    int recursiveCnt_;

};
