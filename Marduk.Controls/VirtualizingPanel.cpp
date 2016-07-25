#include "pch.h"
#include "VirtualizingPanel.h"

using namespace Marduk::Controls;

DependencyProperty^ VirtualizingPanel::_itemContainerStyleProperty = nullptr;
DependencyProperty^ VirtualizingPanel::_itemContainerStyleSelectorProperty = nullptr;
DependencyProperty^ VirtualizingPanel::_itemSourceProperty = nullptr;
DependencyProperty^ VirtualizingPanel::_itemTemplateProperty = nullptr;
DependencyProperty^ VirtualizingPanel::_itemTemplateSelectorProperty = nullptr;

VirtualizingPanel::VirtualizingPanel()
{
    //RegisterDependencyProperties();
    _selectMode = ItemSelectMode::Multiple;
    _items = ref new Vector<Object^>();
    _selectedItems = ref new Vector<Object^>();
    _recycledContainers = ref new Vector<VirtualizingViewItem^>();
    _itemContainerMap = ref new UnorderedMap<Object^, VirtualizingViewItem^, HashObject>();

    _items->VectorChanged += ref new Windows::Foundation::Collections::VectorChangedEventHandler<Object ^>(this, &Marduk::Controls::VirtualizingPanel::OnItemsChanged);
    _selectedItems->VectorChanged += ref new Windows::Foundation::Collections::VectorChangedEventHandler<Platform::Object ^>(this, &Marduk::Controls::VirtualizingPanel::OnSeletionChanged);

    UIElement::AddHandler(UIElement::TappedEvent, ref new Input::TappedEventHandler(this, &VirtualizingPanel::OnItemTapped), true);
    UIElement::AddHandler(UIElement::DoubleTappedEvent, ref new Input::DoubleTappedEventHandler(this, &VirtualizingPanel::OnItemDoubleTapped), true);
    UIElement::AddHandler(UIElement::RightTappedEvent, ref new Input::RightTappedEventHandler(this, &VirtualizingPanel::OnItemRightTapped), true);
    UIElement::AddHandler(UIElement::KeyDownEvent, ref new Input::KeyEventHandler(this, &VirtualizingPanel::OnKeyDown), true);
    UIElement::AddHandler(UIElement::KeyUpEvent, ref new Input::KeyEventHandler(this, &VirtualizingPanel::OnKeyUp), true);

    ChildrenTransitions = ref new Media::Animation::TransitionCollection();
    ChildrenTransitions->Append(ref new Media::Animation::RepositionThemeTransition());
    ChildrenTransitions->Append(ref new Media::Animation::AddDeleteThemeTransition());
    ChildrenTransitions->Append(ref new Media::Animation::ReorderThemeTransition());
    ChildrenTransitions->Append(ref new Media::Animation::PaneThemeTransition());
    ChildrenTransitions->Append(ref new Media::Animation::EdgeUIThemeTransition());
}

void VirtualizingPanel::RegisterDependencyProperties()
{
    if (_itemContainerStyleProperty == nullptr)
    {
        _itemContainerStyleProperty = DependencyProperty::Register(
            nameof(ItemContainerStyle),
            typeof(Windows::UI::Xaml::Style),
            typeof(VirtualizingPanel),
            ref new PropertyMetadata(nullptr,
                ref new PropertyChangedCallback(
                    &VirtualizingPanel::OnItemContainerStyleChangedStatic)));
    }
    if (_itemContainerStyleSelectorProperty == nullptr)
    {
        _itemContainerStyleSelectorProperty = DependencyProperty::Register(
            nameof(ItemContainerStyleSelector),
            typeof(WinCon::StyleSelector),
            typeof(VirtualizingPanel),
            ref new PropertyMetadata(nullptr,
                ref new PropertyChangedCallback(
                    &VirtualizingPanel::OnItemContainerStyleChangedStatic)));
    }
    if (_itemSourceProperty == nullptr)
    {
        _itemSourceProperty = DependencyProperty::Register(
            nameof(ItemSource),
            typeof(Object),
            typeof(VirtualizingPanel),
            ref new PropertyMetadata(nullptr,
                ref new PropertyChangedCallback(
                    &VirtualizingPanel::OnItemSourceChangedStatic)));
    }
    if (_itemTemplateProperty == nullptr)
    {
        _itemTemplateProperty = DependencyProperty::Register(
            nameof(ItemTemplate),
            typeof(DataTemplate),
            typeof(VirtualizingPanel),
            ref new PropertyMetadata(nullptr,
                ref new PropertyChangedCallback(
                    &VirtualizingPanel::OnItemTemplateChangedStatic)));
    }
    if (_itemTemplateSelectorProperty == nullptr)
    {
        _itemTemplateSelectorProperty = DependencyProperty::Register(
            nameof(ItemTemplateSelector),
            typeof(WinCon::DataTemplateSelector),
            typeof(VirtualizingPanel),
            ref new PropertyMetadata(nullptr,
                ref new PropertyChangedCallback(
                    &VirtualizingPanel::OnItemTemplateSelectorChangedStatic)));
    }
}

void VirtualizingPanel::OnItemTapped(Object^ sender, Input::TappedRoutedEventArgs^ e)
{
    if (e->OriginalSource == this)
    {
        return;
    }

    auto original = dynamic_cast<FrameworkElement^>(e->OriginalSource);
    auto item = original->DataContext;

    if (item == nullptr)
    {
        return;
    }

    HandleTapped(item, ItemTapMode::Left);
}

void VirtualizingPanel::OnItemDoubleTapped(Object^ sender, Input::DoubleTappedRoutedEventArgs^ e)
{
    if (e->OriginalSource == this)
    {
        return;
    }

    auto original = dynamic_cast<FrameworkElement^>(e->OriginalSource);
    auto item = original->DataContext;

    if (item == nullptr)
    {
        return;
    }

    HandleTapped(item, ItemTapMode::LeftDouble);
}


void VirtualizingPanel::OnItemRightTapped(Object^ sender, Input::RightTappedRoutedEventArgs^ e)
{
    if (e->OriginalSource == this)
    {
        return;
    }

    auto original = dynamic_cast<FrameworkElement^>(e->OriginalSource);
    auto item = original->DataContext;

    if (item == nullptr)
    {
        return;
    }

    if (IsRightTapSelectEnable)
    {
        _rightTapSelecting = true;
    }
    HandleTapped(item, ItemTapMode::Right);
    if (IsRightTapSelectEnable)
    {
        _rightTapSelecting = false;
    }
}

void VirtualizingPanel::HandleTapped(Platform::Object^ item, ItemTapMode tapMode)
{
    unsigned int index = 0;
    if (!Items->IndexOf(item, &index))
    {
        return;
    }

    auto container = GetContainerFormItem(item);

    switch (_selectMode)
    {
    case ItemSelectMode::Single:
        if (Selecting)
        {
            if (_selectedItems->Size == 1 && _selectedItems->IndexOf(item, &index))
            {
                if (container != nullptr)
                {
                    container->IsSelected = false;
                }
                _selectedItems->Clear();
            }
            else
            {
                for each (auto i in _selectedItems)
                {
                    auto c = GetContainerFormItem(i);
                    if (c != nullptr)
                    {
                        c->IsSelected = false;
                    }
                }
                _selectedItems->Clear();
                _selectedItems->Append(item);
            }
            break;
        }
    case ItemSelectMode::Multiple:
        if (Selecting)
        {
            if (_selectedItems->IndexOf(item, &index))
            {
                if (container != nullptr)
                {
                    container->IsSelected = false;
                }
                _selectedItems->RemoveAt(index);
            }
            else
            {
                if (container != nullptr)
                {
                    container->IsSelected = true;
                }
                _selectedItems->Append(item);
            }
            break;
        }
    case ItemSelectMode::None:
    default:
        ItemTapped(this, ref new ItemTappedEventArgs(container, item, tapMode));
        break;
    }

}

void VirtualizingPanel::OnKeyDown(Object^ sender, Input::KeyRoutedEventArgs^ e)
{
    if (IsShiftSelectEnable && (e->Key | Windows::System::VirtualKey::Shift) == Windows::System::VirtualKey::Shift)
    {
        _shiftSelecting = true;
    }
}

void VirtualizingPanel::OnKeyUp(Object^ sender, Input::KeyRoutedEventArgs^ e)
{
    if (IsShiftSelectEnable && (e->Key | Windows::System::VirtualKey::Shift) == Windows::System::VirtualKey::Shift)
    {
        _shiftSelecting = false;
    }
}

bool VirtualizingPanel::IsItemItsOwnContainerOverride(Object^ obj)
{
    auto container = dynamic_cast<VirtualizingViewItem^>(obj);
    return container != nullptr;
}

VirtualizingViewItem^ VirtualizingPanel::GetContainerForItemOverride()
{
    return ref new VirtualizingViewItem();
}

void VirtualizingPanel::ClearContainerForItemOverride(VirtualizingViewItem^ container, Object^ item)
{
    if (IsItemItsOwnContainerOverride(item))
    {
        return;
    }

    container->Content = nullptr;
    container->ContentTemplate = nullptr;
    container->Style = nullptr;
    container->IsSelected = false;
}

void VirtualizingPanel::PrepareContainerForItemOverride(VirtualizingViewItem^ container, Object^ item)
{
    if (IsItemItsOwnContainerOverride(item))
    {
        return;
    }

    container->Content = item;

    ApplyItemContainerStyle(container, item);
    ApplyItemTemplate(container, item);

    unsigned int index = 0;
    container->IsSelected = SelectedItems->IndexOf(item, &index);
}

VirtualizingViewItem^ VirtualizingPanel::GetContainerFormItem(Object^ item)
{
    if (_itemContainerMap->HasKey(item))
    {
        return _itemContainerMap->Lookup(item);
    }
    else
    {
        return nullptr;
    }
}

VirtualizingViewItem^ VirtualizingPanel::GetContainerFormIndex(int index)
{
    if (index < 0 || index >= _items->Size)
    {
        throw ref new OutOfBoundsException("Index out of bounds.");
    }

    auto item = _items->GetAt(index);
    return GetContainerFormItem(item);
}

Object^ VirtualizingPanel::GetItemFormContainer(VirtualizingViewItem^ container)
{
    if (container == nullptr)
    {
        return nullptr;
    }

    auto item = container->Content;

    if (_itemContainerMap->HasKey(item))
    {
        return item;
    }
    else
    {
        return nullptr;
    }
}

Object^ VirtualizingPanel::GetItemFormIndex(int index)
{
    if (index < 0 || index >= _items->Size)
    {
        throw ref new OutOfBoundsException("Index out of bounds.");
    }

    return _items->GetAt(index);
}

void VirtualizingPanel::OnItemSourceChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<VirtualizingPanel^>(sender);

    if (panel == nullptr)
    {
        return;
    }

    panel->OnItemSourceChanged(e->NewValue, e->OldValue);
}

void VirtualizingPanel::OnItemTemplateChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<VirtualizingPanel^>(sender);

    if (panel == nullptr)
    {
        return;
    }

    panel->OnItemTemplateChanged(dynamic_cast<DataTemplate^>(e->NewValue), dynamic_cast<DataTemplate^>(e->OldValue));
}

void VirtualizingPanel::OnItemTemplateSelectorChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<VirtualizingPanel^>(sender);

    if (panel == nullptr)
    {
        return;
    }

    panel->OnItemTemplateSelectorChanged(dynamic_cast<WinCon::DataTemplateSelector^>(e->NewValue), dynamic_cast<WinCon::DataTemplateSelector^>(e->OldValue));
}

void VirtualizingPanel::OnItemContainerStyleChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<VirtualizingPanel^>(sender);

    if (panel == nullptr)
    {
        return;
    }

    panel->OnItemContainerStyleChanged(dynamic_cast<Windows::UI::Xaml::Style^>(e->NewValue), dynamic_cast<Windows::UI::Xaml::Style^>(e->OldValue));
}

void VirtualizingPanel::OnItemContainerStyleSelectorChangedStatic(DependencyObject^ sender, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
    auto panel = dynamic_cast<VirtualizingPanel^>(sender);

    if (panel == nullptr)
    {
        return;
    }

    panel->OnItemContainerStyleSelectorChanged(dynamic_cast<WinCon::StyleSelector^>(e->NewValue), dynamic_cast<WinCon::StyleSelector^>(e->OldValue));
}

void VirtualizingPanel::OnItemContainerStyleChanged(Windows::UI::Xaml::Style^ newStyle, Windows::UI::Xaml::Style^ oldStyle)
{
    for each (auto var in _itemContainerMap)
    {
        ApplyItemContainerStyle(var->Value, var->Key);
    }
}

void VirtualizingPanel::OnItemContainerStyleSelectorChanged(WinCon::StyleSelector^ newStyleSelector, WinCon::StyleSelector^ oldStyleSelector)
{
    for each (auto var in _itemContainerMap)
    {
        ApplyItemContainerStyle(var->Value, var->Key);
    }
}

void VirtualizingPanel::OnItemTemplateChanged(DataTemplate^ newTemplate, DataTemplate^ oldTemplate)
{
    for each (auto var in _itemContainerMap)
    {
        ApplyItemTemplate(var->Value, var->Key);
    }
}

void VirtualizingPanel::OnItemTemplateSelectorChanged(WinCon::DataTemplateSelector^ newTemplateSelector, WinCon::DataTemplateSelector^ oldTemplateSelector)
{
    for each (auto var in _itemContainerMap)
    {
        ApplyItemTemplate(var->Value, var->Key);
    }
}

void VirtualizingPanel::OnItemSourceChanged(Object^ newItems, Object^ oldItems)
{
    this->RecycleAllItem();
    _itemContainerMap->Clear();
    _items->Clear();

    auto items = dynamic_cast<Windows::UI::Xaml::Interop::IBindableIterable^>(newItems);

    if (items != nullptr)
    {
        auto itertor = items->First();

        while (itertor->HasCurrent)
        {
            _items->Append(itertor->Current);
            itertor->MoveNext();
        }
    }
    else
    {
        _items->Append(newItems);
    }

    auto nc = dynamic_cast<Windows::UI::Xaml::Interop::INotifyCollectionChanged^>(oldItems);
    if (nc != nullptr)
    {
        nc->CollectionChanged -= _collectionEventToken;
    }

    nc = dynamic_cast<Windows::UI::Xaml::Interop::INotifyCollectionChanged^>(newItems);
    if (nc != nullptr)
    {
        _collectionEventToken = nc->CollectionChanged += ref new Windows::UI::Xaml::Interop::NotifyCollectionChangedEventHandler(this, &Marduk::Controls::VirtualizingPanel::OnCollectionChanged);
    }

    _sil = nullptr;
    _loadCount = 0;
    _sil = dynamic_cast<Windows::UI::Xaml::Data::ISupportIncrementalLoading^>(newItems);
}

void VirtualizingPanel::OnCollectionChanged(Object^ sender, Windows::UI::Xaml::Interop::NotifyCollectionChangedEventArgs^ e)
{
    Windows::UI::Xaml::Interop::IBindableIterator^ newItertor = nullptr;
    int newIndex = -1;

    if (e->NewItems != nullptr)
    {
        newItertor = e->NewItems->First();
        newIndex = e->NewStartingIndex;
    }

    switch (e->Action)
    {
    case Windows::UI::Xaml::Interop::NotifyCollectionChangedAction::Add:
        while (newItertor->HasCurrent)
        {
            _items->InsertAt(newIndex++, newItertor->Current);
            newItertor->MoveNext();
        }
        break;
    case Windows::UI::Xaml::Interop::NotifyCollectionChangedAction::Move:
        throw Exception::CreateException(-1, "Unexpected collection operation.");
        break;
    case Windows::UI::Xaml::Interop::NotifyCollectionChangedAction::Remove:
        for (int i = 0; i < e->OldItems->Size; i++)
        {
            RecycleItem(e->OldItems->GetAt(i));
            _items->RemoveAt(e->OldStartingIndex + i);
        }
        break;
    case Windows::UI::Xaml::Interop::NotifyCollectionChangedAction::Replace:
        while (newItertor->HasCurrent)
        {
            _items->SetAt(newIndex++, newItertor->Current);
            newItertor->MoveNext();
        }
        break;
    case Windows::UI::Xaml::Interop::NotifyCollectionChangedAction::Reset:
        _items->Clear();
        break;
    default:
        throw Exception::CreateException(-1, "Unexpected collection operation.");
        break;
    }
}

void VirtualizingPanel::OnItemsChanged(IObservableVector<Object^>^ source, IVectorChangedEventArgs^ e)
{
    return;
}

void  VirtualizingPanel::OnSeletionChanged(IObservableVector<Object^>^ source, IVectorChangedEventArgs^ e)
{
    SeletionChanged(this, ref new SeletionChangedEventArgs());
}

void VirtualizingPanel::ApplyItemContainerStyle(VirtualizingViewItem^ container, Object^ item)
{
    if (ItemContainerStyleSelector != nullptr)
    {
        container->Style = ItemContainerStyleSelector->SelectStyle(item, container);
    }

    if (container->Style == nullptr)
    {
        container->Style = ItemContainerStyle;
    }
}

void VirtualizingPanel::ApplyItemTemplate(VirtualizingViewItem^ container, Object^ item)
{
    if (ItemTemplateSelector != nullptr)
    {
        container->ContentTemplate = ItemTemplateSelector->SelectTemplate(item);
    }

    if (container->ContentTemplate == nullptr)
    {
        container->ContentTemplate = ItemTemplate;
    }
}

void VirtualizingPanel::RecycleItem(Object^ item)
{
    auto container = GetContainerFormItem(item);

    if (container == nullptr)
    {
        return;
    }

    unsigned int index = 0;
    if (Children->IndexOf(container, &index))
    {
        Children->RemoveAt(index);
        _itemContainerMap->Remove(item);
        ClearContainerForItemOverride(container, item);

        if (!IsItemItsOwnContainerOverride(item))
        {
            RecycledContainers->Append(container);
        }
    }
    else
    {
        throw Exception::CreateException(-1, "Can't found container in panel.");
    }
}

VirtualizingViewItem^  VirtualizingPanel::RealizeItem(Object^ item)
{
    VirtualizingViewItem^ container = nullptr;

    if (_itemContainerMap->HasKey(item))
    {
        return _itemContainerMap->Lookup(item);
    }

    if (!IsItemItsOwnContainerOverride(item))
    {
        if (RecycledContainers->Size > 0)
        {
            container = RecycledContainers->GetAt(RecycledContainers->Size - 1);
            RecycledContainers->RemoveAtEnd();
        }
        else
        {
            container = GetContainerForItemOverride();
        }
    }
    else
    {
        container = dynamic_cast<VirtualizingViewItem^>(item);
    }

    PrepareContainerForItemOverride(container, item);
    _itemContainerMap->Insert(item, container);
    Children->Append(container);

    return container;
}

void VirtualizingPanel::RecycleAllItem()
{
    for each (auto items in _itemContainerMap)
    {
        RecycleItem(items->Key);
    }
}

void VirtualizingPanel::LoadMoreItems(int count)
{
    if (_sil != nullptr)
    {
        if (_sil->HasMoreItems)
        {
            _sil->LoadMoreItemsAsync(count);
        }
    }
}

void VirtualizingPanel::LoadMoreItems()
{
    LoadMoreItems(_loadCount++);
}

void VirtualizingPanel::BeginSelect()
{
    _userSelecting = true;
}

void VirtualizingPanel::EndSelect()
{
    _userSelecting = false;
}

ItemTappedEventArgs::ItemTappedEventArgs(VirtualizingViewItem^ container, Platform::Object^ item, ItemTapMode mode)
{
    _container = container;
    _item = item;
    _tapMode = mode;
}

SeletionChangedEventArgs::SeletionChangedEventArgs()
{

}