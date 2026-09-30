#pragma once

#include "../Core/Object.h"

//=============================================================================
//=============================================================================
namespace Urho3D
{
	class ImGuiObj : public Object
	{
		URHO3D_OBJECT(ImGuiObj, Object);

	public:
        ImGuiObj(Context* context);
		virtual ~ImGuiObj();

        /// Register object factory.
        static void RegisterObject(Context* context);

        void SetDarkColorStyle(bool dark);
        void SetScaleSize(float scale);

	protected:
        void HandleOnDeviceLost(StringHash eventType, VariantMap& eventData);
		void HandleRenderUpdate(StringHash eventType, VariantMap& eventData);
		void HandleBeginRendering(StringHash eventType, VariantMap& eventData);
		void HandleEndRendering(StringHash eventType, VariantMap& eventData);

		// Input
		void HandleKeyUp(StringHash eventType, VariantMap& eventData);
		void HandleKeyDown(StringHash eventType, VariantMap& eventData);
		void HandleTextInput(StringHash eventType, VariantMap& eventData);
		void HandleTouchBegin(StringHash eventType, VariantMap& eventData);
		void HandleTouchEnd(StringHash eventType, VariantMap& eventData);
		void HandleTouchMove(StringHash eventType, VariantMap& eventData);

    protected:
        int touchID_;
	};
}


