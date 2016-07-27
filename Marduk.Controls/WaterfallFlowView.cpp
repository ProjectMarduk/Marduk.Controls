#include "pch.h"
#include "WaterfallFlowView.h"

using namespace Marduk::Controls;

DependencyProperty^ WaterfallFlowView::_spacingProperty = nullptr;
DependencyProperty^ WaterfallFlowView::_stackCountProperty = nullptr;
DependencyProperty^ WaterfallFlowView::_isAdaptiveEnableProperty = nullptr;
DependencyProperty^ WaterfallFlowView::_maxItemWidthProperty = nullptr;
DependencyProperty^ WaterfallFlowView::_minItemWidthProperty = nullptr;

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

    if (_isAdaptiveEnableProperty == nullptr)
    {
        _isAdaptiveEnableProperty = DependencyProperty::Register(
            nameof(IsAdaptiveEnable),
            typeof(bool),
            typeof(WaterfallFlowView),
            ref new PropertyMetadata(false,
                ref new PropertyChangedCallback(
                    &WaterfallFlowView::OnIsAdaptiveEnableChangedStatic)));
    }

    if (_maxItemWidthProperty == nullptr)
    {
        _maxItemWidthProperty = DependencyProperty::Register(
            nameof(MaxItemWidth),
            typeof(int),
            typeof(WaterfallFlowView),
            ref new PropertyMetadata(300,
                ref new PropertyChangedCallback(
                    &WaterfallFlowView::OnMaxItemWidthChangedStatic)));
    }

    if (_minItemWidthProperty == nullptr)
    {
        _minItemWidthProperty = DependencyProperty::Register(
            nameof(MinItemWidth),
            typeof(int),
            typeof(WaterfallFlowView),
            ref new PropertyMetadata(200,
                ref new PropertyChangedCallback(
                    &WaterfallFlowView::OnMinItemWidthChangedStatic)));
    }
}

Size WaterfallFlowView::GetItemAvailableSize(Size availableSize)
{
    availableSize.Width = (availableSize.Width - ((StackCount - 1) * Spacing)) / StackCount;
    return availableSize;
}

bool WaterfallFlowView::NeedRelayout(Size availableSize)
{
    ResetStackCount();
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

void WaterfallFlowView::OnIsAdaptiveEnableChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<WaterfallFlowView^>(sender);

    if (panel == nullptr || panel->WaterfallFlow == nullptr)
    {
        return;
    }

    if (panel->IsAdaptiveEnable)
    {
        panel->ResetStackCount();
    }
}

void WaterfallFlowView::OnMaxItemWidthChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<WaterfallFlowView^>(sender);

    if (panel == nullptr || panel->WaterfallFlow == nullptr)
    {
        return;
    }

    if (panel->IsAdaptiveEnable)
    {
        panel->ResetStackCount();
    }
}

void WaterfallFlowView::OnMinItemWidthChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<WaterfallFlowView^>(sender);

    if (panel == nullptr || panel->WaterfallFlow == nullptr)
    {
        return;
    }

    if (panel->IsAdaptiveEnable)
    {
        panel->ResetStackCount();
    }
}

void WaterfallFlowView::ResetStackCount()
{
    if (IsAdaptiveEnable)
    {
        auto aw = ActualWidth;
        Size unit = GetItemAvailableSize(Size(ActualWidth, INFINITY));
        if (unit.Width < MinItemWidth || unit.Width > MaxItemWidth)
        {
            int stackCount = 1;
            if (unit.Width < MinItemWidth)
            {
                for (int count = StackCount - 1; count > 1; count--)
                {
                    float width = (ActualWidth - ((count - 1) * Spacing)) / count;
                    if (width > MinItemWidth)
                    {
                        stackCount = count;
                        break;
                    }
                }
            }
            else
            {
                for (int count = StackCount + 1; true; count++)
                {
                    float width = (ActualWidth - ((count - 1) * Spacing)) / count;
                    if (width < MaxItemWidth)
                    {
                        stackCount = count;
                        break;
                    }
                }
            }

            StackCount = stackCount;
        }
    }
}