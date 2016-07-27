#pragma once
#include "OrientedVirtualizingPanel.h"
#include "VisualWindow.h"
#include "WaterfallFlowLayout.h"
#include "IItemResizer.h"

using namespace Windows::Foundation;

namespace Marduk
{
    namespace Controls
    {
        [Windows::Foundation::Metadata::WebHostHidden()]
        public ref class WaterfallFlowView sealed :
            public OrientedVirtualizingPanel
        {
            RegisterDependencyProperty(double, _spacingProperty, SpacingProperty, Spacing);
            RegisterDependencyProperty(int, _stackCountProperty, StackCountProperty, StackCount);
            RegisterDependencyProperty(bool, _isAdaptiveEnableProperty, IsAdaptiveEnableProperty, IsAdaptiveEnable);
            RegisterDependencyProperty(int , _maxItemWidthProperty, MaxItemWidthProperty, MaxItemWidth);
            RegisterDependencyProperty(int , _minItemWidthProperty, MinItemWidthProperty, MinItemWidth);

        public:
            WaterfallFlowView();

        protected:
            virtual void RegisterDependencyProperties() override;
            virtual Size GetItemAvailableSize(Size availableSize) override;
            virtual bool NeedRelayout(Size availableSize) override;
            virtual void Relayout(Size availableSize) override;
            virtual ILayout^ GetLayout(Size availableSize) override;

        private:
            RegisterReadOnlyPropertyWithExpression(WaterfallFlowLayout^, if (_waterfallFlowLayout == nullptr) { _waterfallFlowLayout = dynamic_cast<WaterfallFlowLayout^>(Layout); } return _waterfallFlowLayout; , WaterfallFlow)
                WaterfallFlowLayout^ _waterfallFlowLayout;
            static void OnSpacingChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
            static void OnStackCountChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
            static void OnIsAdaptiveEnableChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
            static void OnMaxItemWidthChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
            static void OnMinItemWidthChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);

            void ResetStackCount();
        };
    }
}