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
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/DropDownList.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/Log.h>

#include "ScaleUI.h"
#include <cstdio>
#include <Urho3D/DebugNew.h>

//=============================================================================
//This example is to test how UI can be scaled using two methods: UI scale and flex scale, 
//both of which are found in UI source code. The scaling is based on the height difference from base to selected resolution height.
//What I'm seeing in this test is that while UI scale is effective, the flex scale just doesn't 
//seem to make any difference despite setting one of the two required alignments: LM_HORIZONTAL and LM_VERTICAL.
//=============================================================================
URHO3D_DEFINE_APPLICATION_MAIN(ScaleUI)

//=============================================================================
//=============================================================================
ScaleUI::ScaleUI(Context* context) :
    Sample(context)
{
    baseWidth_ = 0;
    baseHeight_ = 0;

    resWidth_ = 0;
    resHeight_ = 0;
    scaleType_ = NoScale;
    recursiveCnt_ = 0;
}

void ScaleUI::Setup()
{
    baseWidth_ = 1280;
    baseHeight_ = 720;

    engineParameters_["WindowTitle"] = GetTypeName();
    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetProgramDir() + "scaleui.log";
    engineParameters_["FullScreen"] = false;
    engineParameters_["Headless"] = false;
    engineParameters_["WindowWidth"] = baseWidth_;
    engineParameters_["WindowHeight"] = baseHeight_;
    engineParameters_["ResourcePaths"] = "Data;CoreData;";

}

void ScaleUI::Start()
{
    Sample::Start();

    LoadMainUI();
    LoadScaleMenu();
    LoadBorderUI();

    AdjustUIBars();

    Sample::InitMouseMode(MM_FREE);
}

void ScaleUI::LoadMainUI()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();
    Graphics* graphics = GetSubsystem<Graphics>();

    XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    uiRoot->SetDefaultStyle(uiStyle);

    //main menu
    mainMenu_ = ui->LoadLayout(cache->GetResource<XMLFile>("ScaleUI/mainMenu.xml"));
    uiRoot->AddChild(mainMenu_);

    RegisterMenuButton(mainMenu_, "buttonResolution");
    RegisterMenuButton(mainMenu_, "buttonScale");
    RegisterMenuButton(mainMenu_, "buttonQuit");

    //resolutions: w, h, and refresh rate
    IntVector2 curMonitorSize = graphics->GetSize();
    int monitor = graphics->GetMonitor();
    PODVector<IntVector3> resolutions = graphics->GetResolutions(monitor);
    resMenu_ = ui->LoadLayout(cache->GetResource<XMLFile>("ScaleUI/resMenu.xml"));
    resMenu_->SetVisible(false);
    uiRoot->AddChild(resMenu_);

    //capture cur res
    resWidth_ = curMonitorSize.x_;
    resHeight_ = curMonitorSize.y_;

    dropDownListResolution_ = resMenu_->GetChildStaticCast<DropDownList>("resDropList", true);
    dropDownListResolution_->SetLayout(LM_VERTICAL, 4, IntRect(10, 4, 4, 4));
    dropDownListResolution_->SetStyleAuto();
    ListView *listView = dropDownListResolution_->GetListView();
    listView->SetLayout(LM_VERTICAL, 10, IntRect(10, 4, 4, 4));
    listView->SetMinWidth(140);

    //remove filler items created by default
    listView->RemoveAllItems();

    SubscribeToEvent(listView, E_ITEMCLICKED, URHO3D_HANDLER(ScaleUI, HandleListViewClicked));

    RegisterMenuButton(resMenu_, "buttonResSet");
    RegisterMenuButton(resMenu_, "buttonResCancel");

    //list res
    char resbuf[64];
    unsigned selectionIdx = 0;
    PODVector<int> resList;
    for (unsigned i = 0; i < resolutions.Size(); ++i)
    {
        //check for unique resolution
        int resSig = resolutions[i].x_ * 10000 + resolutions[i].y_;
        bool inList = false;
        for (unsigned lx = 0; lx < resList.Size(); ++lx)
        {
            if (resList[lx] == resSig)
            {
                inList = true;
                break;
            }
        }
        if (inList)
        {
            continue;
        }

        //add resolution
        resList.Push(resSig);
        Text *txt = new Text(context_);

        sprintf(resbuf, "%d x %d", resolutions[i].x_, resolutions[i].y_);
        txt->SetText(resbuf);
        txt->SetStyleAuto();
        dropDownListResolution_->AddItem(txt);

        //check cur selection
        if (curMonitorSize.x_ == resolutions[i].x_ && curMonitorSize.y_ == resolutions[i].y_)
        {
            selectionIdx = i;
        }
        String msg(resbuf);
        URHO3D_LOGDEBUG(msg);
    }

    dropDownListResolution_->SetSelection(selectionIdx);
}

void ScaleUI::LoadScaleMenu()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();

    scaleMenu_ = ui->LoadLayout(cache->GetResource<XMLFile>("ScaleUI/scaleMenu.xml"));
    scaleMenu_->SetVisible(false);
    uiRoot->AddChild(scaleMenu_);

    RegisterMenuButton(scaleMenu_, "scaleButtonOK");
    RegisterMenuButton(scaleMenu_, "scaleButtonCancel");

    RegisterGroupCheckBox(scaleMenu_, "scaleCheckbox1");
    RegisterGroupCheckBox(scaleMenu_, "scaleCheckbox2");
    RegisterGroupCheckBox(scaleMenu_, "scaleCheckbox3");

}

void ScaleUI::LoadBorderUI()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();

    topBar_ = ui->LoadLayout(cache->GetResource<XMLFile>("ScaleUI/topBar.xml"));
    //tested the below to see what happens but scaling is not correct
    //topBar_->SetLayout(LM_VERTICAL, 4, IntRect(4, 4, 4, 4));
    //topBar_->SetLayoutMode(LM_VERTICAL);
    uiRoot->AddChild(topBar_);

    //top bar
    topBarText_ = topBar_->GetChildStaticCast<Text>("topBarText", true);
    topBarBorderImage_ = topBar_->GetChildStaticCast<BorderImage>("topBarBorderImage", true);

    //right bar
    rightBar_ = ui->LoadLayout(cache->GetResource<XMLFile>("ScaleUI/rightBarFlex.xml"));
    uiRoot->AddChild(rightBar_);
}

void ScaleUI::AdjustUIScale()
{
    UI* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();

    float scale = (float)resHeight_/(float)baseHeight_;
    char buf[80];
    sprintf(buf, "adj scale() resolution=%d x %d, type=%d, scale=%0.3f", resWidth_, resHeight_, scaleType_, scale);
    String msg(buf);
    URHO3D_LOGDEBUG(msg);

    switch (scaleType_)
    {
    case NoScale:
        ui->SetScale(1.0f);
        uiRoot->SetLayoutFlexScale(Vector2::ONE);
        uiRoot->UpdateLayout();
        break;

    case UIScale:
        ui->SetScale(scale);
        uiRoot->SetLayoutFlexScale(Vector2::ONE);
        uiRoot->UpdateLayout();
        break;

    case FlexScale:
        ui->SetScale(1.0f);
        uiRoot->SetLayoutFlexScale(Vector2(scale, scale));
        uiRoot->UpdateLayout();
        break;
    }

    AdjustUIBars();
}

void ScaleUI::AdjustUIBars()
{
    //adjust topbar width
    float scale = (float)resHeight_/(float)baseHeight_;
    topBar_->SetWidth(resWidth_);
    topBarBorderImage_->SetWidth(resWidth_);

    //adjust the image for scaling
    if (scaleType_ == UIScale)
    {
        int scaledWidth = (int)(scale *(float)resWidth_);
        //diff will also get scaled, so negate that scaling by dividing it 
        int diff = (int)(((float)resWidth_ - (float)scaledWidth)/scale);
        topBar_->SetWidth(resWidth_ + diff);
        topBarBorderImage_->SetWidth(resWidth_ + diff);
    }

    //adjust rightbar position
    const IntVector2 &topSize = topBar_->GetSize();
    const IntVector2 &topImgPos = topBarBorderImage_->GetPosition();
    const IntVector2 &topImgSize = topBarBorderImage_->GetSize();
    const IntVector2 &rgtbarPos = rightBar_->GetPosition();

    //the top bar image is bottom aligned, which puts the bottom of the image at y_, (constructed from bottom then upwards?)
    //you see this as pivot(0, 1) in the topbar.xml file, but look at it in the editor
    //and you will see the aligned position as 'Bottom' which is then written as pivot(0, 1)
    //?? is this a bug? top size specifies the height of all elements contained within it, 
    //yet the bottom aligned image goes outside of this boundary (positioned and rendered outside the boundary)
    rightBar_->SetPosition(IntVector2(rgtbarPos.x_, topSize.y_ + topImgPos.y_));

    char buf[80];
    sprintf(buf, "AdjustUIBars() topbar size(%d, %d), topimage pos(%d, %d) size(%d, %d)", 
            topSize.x_, topSize.y_, 
            topImgPos.x_, topImgPos.y_, topImgSize.x_, topImgSize.y_);
    String msg(buf);
    URHO3D_LOGDEBUG(msg);

}

void ScaleUI::RegisterMenuButton(UIElement *elem, const char *str)
{
    Button *button = elem->GetChildStaticCast<Button>(str, true);
    SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(ScaleUI, HandleButtonReleased));
}

void ScaleUI::RegisterGroupCheckBox(UIElement *elem, const char *str)
{
    CheckBox *checkBox = elem->GetChildStaticCast<CheckBox>(str, true);

    //set the default as checked
    if (checkBox && checkBoxGrp_.Size() == 0)
    {
        checkBox->SetChecked(true);
    }
    SubscribeToEvent(checkBox, E_TOGGLED, URHO3D_HANDLER(ScaleUI, HandleCheckboxToggle));
    checkBoxGrp_.Push(checkBox);
}

void ScaleUI::HandleListViewClicked(StringHash eventType, VariantMap& eventData)
{
    using namespace ItemClicked;
    UIElement *elem = (UIElement*)eventData[P_ELEMENT].GetVoidPtr();
    Text *txt = (Text*)eventData[P_ITEM].GetVoidPtr();
    
    //parse resolution - specified as "%d x %d" format at insertion
    const String &str = txt->GetText();
    unsigned int xidx = str.Find('x');
    String width(str.CString(), xidx - 1);
    String height(str.CString() + xidx + 1);
    resWidth_ = ToInt(width);
    resHeight_ = ToInt(height);

    //dbg
    char buf[80];
    sprintf(buf, "ListViewClick() resolution=%d x %d", resWidth_, resHeight_);
    String msg(buf);
    URHO3D_LOGDEBUG(msg);
}

void ScaleUI::HandleButtonReleased(StringHash eventType, VariantMap& eventData)
{
    using namespace Released;
    UIElement *elem = (UIElement*)eventData[P_ELEMENT].GetVoidPtr();
    String name = elem->GetName();
    Graphics* graphics = GetSubsystem<Graphics>();

    //main
    if (name == "buttonResolution")
    {
        mainMenu_->SetVisible(false);
        resMenu_->SetVisible(true);
    }
    else if (name == "buttonScale")
    {
        scaleMenu_->SetVisible(true);
        mainMenu_->SetVisible(false);

    }
    else if (name == "buttonQuit")
    {
        engine_->Exit();
    }
    //resolution
    else if (name == "buttonResSet")
    {
        //get selection and change resolution
        graphics->SetMode(resWidth_, resHeight_);

        AdjustUIScale();
        AdjustUIBars();

        char buf[80];
        sprintf(buf, "buttonResSet() resolution=%dx%d", resWidth_, resHeight_);
        String msg(buf);
        URHO3D_LOGDEBUG(msg);

        //restore menu
        mainMenu_->SetVisible(true);
        resMenu_->SetVisible(false);
    }
    else if (name == "buttonResCancel")
    {
        mainMenu_->SetVisible(true);
        resMenu_->SetVisible(false);
    }
    //scale
    else if (name == "scaleButtonOK")
    {
        mainMenu_->SetVisible(true);
        scaleMenu_->SetVisible(false);

        AdjustUIScale();

    }
    else if (name == "scaleButtonCancel")
    {
        mainMenu_->SetVisible(true);
        scaleMenu_->SetVisible(false);
    }
}

void ScaleUI::HandleCheckboxToggle(StringHash eventType, VariantMap& eventData)
{
    using namespace Toggled;
    UIElement *elem = (UIElement*)eventData[P_ELEMENT].GetVoidPtr();
    bool checked = eventData[P_STATE].GetBool();
    
    recursiveCnt_++;

    if (recursiveCnt_ == 1)
    {

        if (elem->GetName() == "scaleCheckbox1")
        {
            scaleType_ = NoScale;
            if (checked)
            {
                checkBoxGrp_[UIScale]->SetChecked(false);
                checkBoxGrp_[FlexScale]->SetChecked(false);
            }
        }
        else if (elem->GetName() == "scaleCheckbox2")
        {
            scaleType_ = UIScale;
            if (checked)
            {
                checkBoxGrp_[NoScale]->SetChecked(false);
                checkBoxGrp_[FlexScale]->SetChecked(false);
            }
        }
        else if(elem->GetName() == "scaleCheckbox3")
        {
            scaleType_ = FlexScale;
            if (checked)
            {
                checkBoxGrp_[NoScale]->SetChecked(false);
                checkBoxGrp_[UIScale]->SetChecked(false);
            }
        }

        // have the default checked to none
        if (!checkBoxGrp_[UIScale]->IsChecked() && !checkBoxGrp_[FlexScale]->IsChecked())
        {
            scaleType_ = NoScale;
            checkBoxGrp_[NoScale]->SetChecked(true);
        }
    }

    recursiveCnt_--;
}

