#include "pch.h"
#include "WaterfallFlowView.h"

using namespace Marduk::Controls;

DependencyProperty^ WaterfallFlowView::_spacingProperty = nullptr;
DependencyProperty^ WaterfallFlowView::_stackCountProperty = nullptr;

WaterfallFlowView::WaterfallFlowView()
{
    RegisterDependencyProperties();
}

void WaterfallFlowView::RegisterDependencyProperties()
{
    VirtualizingPanel::RegisterDependencyProperties();
    if (_spacingProperty == nullptr)
    {
        _spacingProperty = DependencyProperty::Register(
            nameof(Spacing),
            typeof(double),
            typeof(WaterfallFlowView),
            ref new PropertyMetadata(5.0,
                ref new PropertyChangedCallback(
                    &WaterfallFlowView::OnSpacingChangedStatic)));
    }

    if (_stackCountProperty == nullptr)
    {
        _stackCountProperty = DependencyProperty::Register(
            nameof(StackCount),
            typeof(int),
            typeof(WaterfallFlowView),
            ref new PropertyMetadata(2,
                ref new PropertyChangedCallback(
                    &WaterfallFlowView::OnStackCountChangedStatic)));
    }
}

Size WaterfallFlowView::GetItemAvailableSize(Size availableSize)
{
    availableSize.Width = (availableSize.Width - ((StackCount - 1) * Spacing)) / StackCount;
    return availableSize;
}

bool WaterfallFlowView::NeedRelayout(Size availableSize)
{
    return OrientedVirtualizingPanel::NeedRelayout(availableSize) || WaterfallFlow->Spacing != Spacing || WaterfallFlow->StackCount != StackCount;
}

void WaterfallFlowView::Relayout(Size availableSize)
{
    WaterfallFlow->ChangeSpacing(Spacing);
    WaterfallFlow->ChangeStackCount(StackCount);
    OrientedVirtualizingPanel::Relayout(availableSize);
}

ILayout^ WaterfallFlowView::GetLayout(Size availableSize)
{
    return ref new WaterfallFlowLayout(Spacing, availableSize.Width, StackCount);
}

void WaterfallFlowView::OnSpacingChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<WaterfallFlowView^>(sender);

    if (panel == nullptr || panel->WaterfallFlow == nullptr)
    {
        return;
    }

    panel->InvalidateMeasure();
    panel->InvalidateArrange();
}

void WaterfallFlowView::OnStackCountChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<WaterfallFlowView^>(sender);
    
    if (panel == nullptr || panel->WaterfallFlow == nullptr)
    {
        return;
    }

    panel->InvalidateMeasure();
    panel->InvalidateArrange();
}