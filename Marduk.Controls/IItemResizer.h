#pragma once

using namespace Windows::Foundation;

namespace Marduk
{
    namespace Controls
    {
        public interface class IItemResizer
        {
        public:
            Size Resize(Object^ item, Size oldSize, Size availableSize);
        };
    }
}