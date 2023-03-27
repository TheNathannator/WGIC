#pragma once
#include "pch.h"

namespace winrt::WGIC
{
    // General-purpose wrapper for using a std::vector to implement IVector
    template<typename T>
    struct VectorCollection : winrt::implements<VectorCollection<T>, Collections::IVector<T>,
        Collections::IVectorView<T>, Collections::IIterable<T>>,
        winrt::vector_base<VectorCollection<T>, T>
    {
    private:
        std::vector<T> m_values;

    public:
        auto& get_container() const noexcept
        {
            return m_values;
        }

        auto& get_container() noexcept
        {
            return m_values;
        }
    };
}
