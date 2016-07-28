#include "pch.h"
#include "OrientedVirtualizingPanel.h"

using namespace Marduk::Controls;

OrientedVirtualizingPanel::OrientedVirtualizingPanel()
{
	_timer = ref new Windows::UI::Xaml::DispatcherTimer();

	_timer->Interval = TimeSpan{ 100000 };
	_timer->Tick += ref new Windows::Foundation::EventHandler<Object ^>(this, &OrientedVirtualizingPanel::OnTick);

    Loaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &Marduk::Controls::OrientedVirtualizingPanel::OnLoaded);
    Unloaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &Marduk::Controls::OrientedVirtualizingPanel::OnUnloaded);
	auto mc = MeasureControl;
}

void OrientedVirtualizingPanel::OnTick(Object^ sender, Object^e)
{
	_timer->Stop();
	_isSkip = false;
    RequestMeasure();
    RequestArrange();
}

Size OrientedVirtualizingPanel::MeasureOverride(Size availableSize)
{
    VirtualizingPanel::MeasureOverride(availableSize);
	if (_parentScrollView == nullptr)
	{
		_parentScrollView = dynamic_cast<WinCon::ScrollViewer^>(this->Parent);

		if (_parentScrollView != nullptr)
		{
			_parentScrollView->ViewChanging += ref new Windows::Foundation::EventHandler<WinCon::ScrollViewerViewChangingEventArgs ^>(this, &OrientedVirtualizingPanel::OnViewChanging);
			_sizeChangedToken = _parentScrollView->SizeChanged += ref new Windows::UI::Xaml::SizeChangedEventHandler(this, &OrientedVirtualizingPanel::OnSizeChanged);
            _parentScrollView->Loaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &Marduk::Controls::OrientedVirtualizingPanel::OnScrollViewLoaded);
            _parentScrollView->Unloaded += ref new Windows::UI::Xaml::RoutedEventHandler(this, &Marduk::Controls::OrientedVirtualizingPanel::OnScrollViewUnloaded);
		}
	}

	if (_parentScrollView == nullptr)
	{
		return Size(availableSize.Width, 0);
	}

	if (_parentScrollView->ViewportHeight == 0)
	{
		return Size(availableSize.Width, 0);
	}

	if (_layout == nullptr)
	{
		_layout = GetLayout(availableSize);
	}

	_itemAvailableSize = GetItemAvailableSize(availableSize);

	if (_requestRelayout || NeedRelayout(availableSize))
	{
		if (DelayMeasure && !_requestRelayout)
		{
			_isSkip = true;
		}

		_requestRelayout = true;
	}

	if (_isSkip)
	{
		_isSkip = true;
		_timer->Stop();
		_timer->Start();
		return _layout->LayoutSize;
	}

	if (_requestRelayout)
	{
		_requestRelayout = false;

		if (_requestShowItemIndex < 0 && (_parentScrollView->VerticalOffset >= Layout->HeaderSize.Height))
		{
			int requestFirstVisableItemIndex = _firstRealizationItemIndex;
			int requestLastVisableItemIndex = _lastRealizationItemIndex;
			auto items = (std::vector<Object^>*)(void*)(_layout->GetVisableItems(VisualWindow{ _parentScrollView->VerticalOffset,_parentScrollView->ViewportHeight }, &requestFirstVisableItemIndex, &requestLastVisableItemIndex));
			delete(items);
			for (int i = requestFirstVisableItemIndex; i <= requestLastVisableItemIndex; i++)
			{
				if (i >= 0)
				{
					if (Layout->GetItemLayoutRect(i).Top >= _parentScrollView->VerticalOffset)
					{
						_requestShowItemIndex = i;
						break;
					}
				}
			}
		}

		Relayout(availableSize);
	}

	if (_requestShowItemIndex >= 0)
	{
		auto requestScrollViewOffset = MakeItemVisable(_requestShowItemIndex);

		if (_scrollViewOffsetCache != _scrollViewOffset)
		{
			_scrollViewOffsetCache = _scrollViewOffset;
			if (requestScrollViewOffset.Y != _scrollViewOffset.Y)
			{
				_timer->Start();
				return _layout->LayoutSize;
			}
		}

		_requestShowItemIndex = -1;
	}

	if (_scrollViewOffset.X < 0 || _scrollViewOffset.Y < 0)
	{
		_scrollViewOffset = Point((float)(_parentScrollView->HorizontalOffset), (float)(_parentScrollView->VerticalOffset));
	}
	_requestWindow = GetVisibleWindow(_scrollViewOffset, Size((float)_parentScrollView->ViewportWidth, (float)_parentScrollView->ViewportHeight));

	for (int i = _layout->UnitCount; i < (LONGLONG)Items->Size; i++)
	{
		if (_layout->FillWindow(_requestWindow))
		{
			break;
		}

		Size itemSize = MeasureItem(Items->GetAt(i), Size{ 0,0 });
		_layout->AddItem(i, Items->GetAt(i), itemSize);
	}

	if ((_scrollViewOffset.X > 0 || _scrollViewOffset.Y > 0 )&& !_layout->FillWindow(_requestWindow))
	{
		LoadMoreItems();
	}

	int requestFirstRealizationItemIndex = _firstRealizationItemIndex;
	int requestLastRealizationItemIndex = _lastRealizationItemIndex;

	auto visableItems = (std::vector<Object^>*)(void*)(_layout->GetVisableItems(_requestWindow, &requestFirstRealizationItemIndex, &requestLastRealizationItemIndex));
	std::sort(_visableItems->begin(), _visableItems->end(), CompareObject());
	std::sort(visableItems->begin(), visableItems->end(), CompareObject());

	std::vector<Object^>* needRealizeItems = new std::vector<Object^>();
	std::vector<Object^>* needRecycleItems = new std::vector<Object^>();

	std::set_difference(visableItems->begin(), visableItems->end(), _visableItems->begin(), _visableItems->end(), std::back_inserter(*needRealizeItems), CompareObject());
	std::set_difference(_visableItems->begin(), _visableItems->end(), visableItems->begin(), visableItems->end(), std::back_inserter(*needRecycleItems), CompareObject());

	auto children = Children;

	if ((_itemAvailableSizeCache.Width == _itemAvailableSize.Width && _itemAvailableSizeCache.Height == _itemAvailableSize.Height))
	{
		for (auto item : *needRealizeItems)
		{
			auto container = RealizeItem(item);
			container->Measure(_itemAvailableSize);
		}
	}
	else
	{
		for (auto item : *visableItems)
		{
			auto container = RealizeItem(item);
			container->Measure(_itemAvailableSize);
		}
	}

	for (auto item : *needRecycleItems)
	{
		RecycleItem(item);
	}

	delete(needRealizeItems);
	delete(needRecycleItems);
	delete(_visableItems);
	_visableItems = visableItems;

	_firstRealizationItemIndex = requestFirstRealizationItemIndex;
	_lastRealizationItemIndex = requestLastRealizationItemIndex;
	_itemAvailableSizeCache = _itemAvailableSize;

	if (HeaderContainer != nullptr)
	{
		//OnHeaderMeasureOverride(availableSize);
	}

	if (FooterContainer != nullptr)
	{
		//if (_lastRealizationItemIndex + 1 == Items->Size)
		//{
		//	FooterContainer->Visibility = Windows::UI::Xaml::Visibility::Visible;
		//}
		//else
		//{
		//	FooterContainer->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		//}
		//OnFooterMeasureOverride(availableSize);
	}

	return _layout->LayoutSize;
}

Point OrientedVirtualizingPanel::MakeItemVisable(int index)
{
	auto rect = _layout->GetItemLayoutRect(index);
	_parentScrollView->ChangeView((double)rect.Left, (double)rect.Top, 1.0f, true);
	return Point{ rect.Left, rect.Top };
}

Size OrientedVirtualizingPanel::ArrangeOverride(Size finalSize)
{
    VirtualizingPanel::ArrangeOverride(finalSize);
	if (_isSkip)
	{
		return finalSize;
	}

	if (_layout == nullptr)
	{
		return finalSize;
	}

	if (_firstRealizationItemIndex < 0 || _lastRealizationItemIndex < 0)
	{
		return finalSize;
	}

	for (int i = _firstRealizationItemIndex; i <= _lastRealizationItemIndex; i++)
	{
		auto rect = _layout->GetItemLayoutRect(i);
		auto container = GetContainerFormIndex(i);
		container->Arrange(rect);
	}

	if (HeaderContainer != nullptr)
	{
		//OnHeaderArrangeOverride(finalSize);
	}

	if (FooterContainer != nullptr)
	{
		//OnFooterArrangeOverride(finalSize);
	}

	return finalSize;
}

void OrientedVirtualizingPanel::OnViewChanging(Object^ sender, WinCon::ScrollViewerViewChangingEventArgs ^ e)
{
	auto i = e->FinalView;
	int viewIndex = (int)floor(e->NextView->VerticalOffset / (_parentScrollView->ViewportHeight / 2)) + 1;
	if (viewIndex != _viewIndex)
	{
		_viewIndex = viewIndex;
		_scrollViewOffset = Point((float)e->NextView->HorizontalOffset, (float)e->NextView->VerticalOffset);
        RequestMeasure();
        RequestArrange();
	}
}

void OrientedVirtualizingPanel::OnSizeChanged(Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	_parentScrollView->SizeChanged -= _sizeChangedToken;
    RequestMeasure();
    RequestArrange();
}

void OrientedVirtualizingPanel::OnItemsChanged(IObservableVector<Object^>^ source, IVectorChangedEventArgs^ e)
{
	if (_layout == nullptr)
	{
        RequestMeasure();
        RequestArrange();
		return;
	}

	switch (e->CollectionChange)
	{
	case CollectionChange::Reset:
		_layout->RemoveAll();
        RequestMeasure();
        RequestArrange();
		break;
	case CollectionChange::ItemInserted:
		if (e->Index != Items->Size - 1)
		{
			if ((LONGLONG)e->Index <= _layout->UnitCount)
			{
				_layout->AddItem(e->Index, Items->GetAt(e->Index), MeasureItem(Items->GetAt(e->Index), Size(0, 0)));
			}
		}
		else
		{
			if (_layout->FillWindow(_requestWindow))
			{
				break;
			}
		}

        RequestMeasure();
        RequestArrange();
		break;
	case CollectionChange::ItemRemoved:
		if ((LONGLONG)e->Index < _layout->UnitCount)
		{
			_layout->RemoveItem(e->Index);

            RequestMeasure();
            RequestArrange();
		}
		break;
	case CollectionChange::ItemChanged:
		if ((LONGLONG)e->Index < _layout->UnitCount)
		{
			_layout->ChangeItem(e->Index, Items->GetAt(e->Index), MeasureItem(Items->GetAt(e->Index), Size(0, 0)));

            RequestMeasure();
            RequestArrange();
		}
		break;
	default:
		throw Exception::CreateException(-1, "Unexpected collection operation.");
		break;
	}
}

Size OrientedVirtualizingPanel::MeasureItem(Object^ item, Size oldSize)
{
	if (Resizer != nullptr && oldSize.Height > 0)
	{
		return Resizer->Resize(item, oldSize, _itemAvailableSize);
	}

	if (IsItemItsOwnContainerOverride(item))
	{
		auto measureControl = dynamic_cast<WinCon::ContentControl^>(item);
		Children->Append(measureControl);
		measureControl->Measure(_itemAvailableSize);
		auto result = measureControl->DesiredSize;
		Children->RemoveAtEnd();
		return result;
	}
	else
	{
		PrepareContainerForItemOverride(MeasureControl, item);
		MeasureControl->Measure(_itemAvailableSize);
		ClearContainerForItemOverride(MeasureControl, item);
		return MeasureControl->DesiredSize;
	}
}

VisualWindow OrientedVirtualizingPanel::GetVisibleWindow(Point offset, Size viewportSize)
{
	return  VisualWindow{ max(offset.Y - viewportSize.Height, 0),viewportSize.Height * 3 };
}

Size OrientedVirtualizingPanel::GetItemAvailableSize(Size availableSize)
{
	return availableSize;
}

bool OrientedVirtualizingPanel::NeedRelayout(Size availableSize)
{
	return _layout->Width != availableSize.Width;
}

void OrientedVirtualizingPanel::Relayout(Size availableSize)
{
	_layout->ChangePanelSize(availableSize.Width);

	for (int i = 0; i < _layout->UnitCount; i++)
	{
		auto newSize = MeasureItem(Items->GetAt(i), _layout->GetItemSize(i));
		_layout->ChangeItem(i, Items->GetAt(i), newSize);
	}
}

ILayout^ OrientedVirtualizingPanel::GetLayout(Size availableSize)
{
	return nullptr;
}

void OrientedVirtualizingPanel::OnHeaderMeasureOverride(Size availableSize)
{
	if (HeaderContainer == nullptr)
		return;

	availableSize = Layout->GetHeaderAvailableSize();
	HeaderContainer->Measure(availableSize);
	Layout->SetHeaderSize(Size(availableSize.Width, HeaderContainer->DesiredSize.Height));
}
void OrientedVirtualizingPanel::OnHeaderArrangeOverride(Size finalSize)
{
	if (HeaderContainer == nullptr)
		return;

	HeaderContainer->Arrange(Layout->GetHeaderLayoutRect());
}

void OrientedVirtualizingPanel::OnFooterMeasureOverride(Size availableSize)
{
	if (FooterContainer == nullptr)
		return;

	availableSize = Layout->GetFooterAvailableSize();
	FooterContainer->Measure(availableSize);
	Layout->SetFooterSize(availableSize);
}
void OrientedVirtualizingPanel::OnFooterArrangeOverride(Size finalSize)
{
	if (FooterContainer == nullptr)
		return;

	FooterContainer->Arrange(Layout->GetFooterLayoutRect());
}

void OrientedVirtualizingPanel::OnItemContainerSizeChanged(Platform::Object^ item, VirtualizingViewItem^ itemContainer, Size newSize)
{
	UINT index = 0;
	if (Items->IndexOf(item, &index))
	{
		if (newSize != Layout->GetItemSize(index))
		{
			Layout->ChangeItem(index, item, newSize);
            RequestMeasure();
            RequestArrange();
		}
	}
}

void Marduk::Controls::OrientedVirtualizingPanel::OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
    RequestMeasure();
    RequestArrange();
}


void Marduk::Controls::OrientedVirtualizingPanel::OnUnloaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
    auto source = e->OriginalSource;
}

void Marduk::Controls::OrientedVirtualizingPanel::OnScrollViewLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
    RequestMeasure();
    RequestArrange();
}


void Marduk::Controls::OrientedVirtualizingPanel::OnScrollViewUnloaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
    auto source = e->OriginalSource;
}