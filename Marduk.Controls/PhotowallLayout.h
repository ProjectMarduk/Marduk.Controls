#pragma once
#include "VisualWindow.h"
#include "PhotowallUnit.h"
#include "ILayout.h"

using namespace Windows::Foundation;

namespace Marduk
{
    namespace Controls
    {
        ref class PhotowallLayout sealed
            : ILayout
        {
        public:
            virtual RegisterReadOnlyProperty(double, _width, Width);
            virtual RegisterReadOnlyProperty(Size, Size(Width, UnitCount == 0 ? 0 : RowCount * (_unitSize + _spacing) - _spacing), LayoutSize);
            virtual RegisterReadOnlyProperty(int, _units->size(), UnitCount);

            RegisterReadOnlyProperty(double, _spacing, Spacing);
            RegisterReadOnlyProperty(double, _unitSize, UnitSize);
            RegisterReadOnlyProperty(int, _rowIndex + 1 - (_lastRowLocked ? 1 : 0), RowCount);

            PhotowallLayout(double spacing, double width, double unitSize);

            virtual void AddItem(int index, Platform::Object^ item, Size size);
            virtual void ChangeItem(int index, Platform::Object^ item, Size size);
            virtual void RemoveItem(int index);
            virtual void RemoveAll();

            virtual Platform::IntPtr GetVisableItems(VisualWindow window, int* firstIndex, int * lastIndex);
            virtual Rect GetItemLayoutRect(int index);
            virtual bool FillWindow(VisualWindow window);
            virtual bool IsItemInWindow(VisualWindow window, int index);
            virtual void ChangePanelSize(double width);
            virtual Size GetItemSize(int index);

            Size GetItemSize(Platform::Object^ item);

            void ChangeSpacing(double spacing);
            void ChangeUnitSize(double unitSize);
        private:
            ~PhotowallLayout();
            double _spacing;
            double _width;
            double _offset;
            int _rowIndex;
            double _unitSize;
            bool _lastRowLocked = false;
            std::vector<PhotowallUnit^>* _units;
            std::unordered_map<Platform::Object^, PhotowallUnit^, HashObject>* _itemUnitMap;
            int _requestRelayoutIndex = -1;
            void SetRelayoutIndex(int index);
            void Relayout();
            void RelayoutRow(int itemIndex);
        };
    }
}