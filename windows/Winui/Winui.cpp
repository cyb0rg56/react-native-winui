#include "pch.h"

#include "Winui.h"

namespace winrt::Winui
{

void Winui::Initialize(React::ReactContext const& reactContext) noexcept
{
  m_context = reactContext;
}

} // namespace winrt::Winui
