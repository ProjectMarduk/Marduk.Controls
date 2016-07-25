#pragma once

using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;

namespace MoePic
{
    namespace Controls
    {
        [uuid("F077B577-39BD-46EE-BDD7-0826BEED71B8")]
        public interface class IOrientedVirtualizingPanel
        {
        public:
            void LineUp();
            void LineDown();
            void LineLeft();
            void LineRight();
            void PageUp();
            void PageDown();
            void PageLeft();
            void PageRight();
            void MouseWheelUp();
            void MouseWheelDown();
            void MouseWheelLeft();
            void MouseWheelRight();
            void SetHorizontalOffset(double offset);
            void SetVerticalOffset(double offset);
            Rect MakeVisible(UIElement^ visual, Rect rectangle);
            property bool CanHorizontallyScroll;
            property bool CanVerticallyScroll;
            property double ExtentHeight { double get(); }
            property double ExtentWidth { double get(); }
            property double HorizontalOffset { double get(); }
            property Object^ ScrollOwner;
            property double VerticalOffset { double get(); }
            property double ViewportHeight { double get(); }
            property double ViewportWidth { double get(); }
        };
    }
}