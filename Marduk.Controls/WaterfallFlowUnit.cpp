#include "pch.h"
#include "WaterfallFlowUnit.h"

using namespace Marduk::Controls;

WaterfallFlowUnit::WaterfallFlowUnit(Platform::Object^ item, Size desiredSize)
{
    _item = item;
    _desiredSize = desiredSize;
}
