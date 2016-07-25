#include "pch.h"
#include "PhotowallUnit.h"

using namespace Marduk::Controls;

PhotowallUnit::PhotowallUnit(Platform::Object^ item, Size desiredSize)
{
    Item = item;
    DesiredSize = desiredSize;
}