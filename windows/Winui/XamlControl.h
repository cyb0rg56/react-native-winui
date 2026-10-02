#pragma once

#include "pch.h"

#include <functional>
#include <string>

#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Microsoft.ReactNative.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct MeasuredSize : winrt::implements<MeasuredSize, winrt::IInspectable> {
  MeasuredSize() = default;
  winrt::Windows::Foundation::Size natural{0, 0};
  winrt::Windows::Foundation::Size wrapped{0, 0};
  float wrappedWidth{0};
};

struct XamlIslandSession {
  void Attach(
      winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view,
      winrt::Microsoft::UI::Xaml::UIElement const& content);
  void Close() noexcept;
  void SetState(winrt::Microsoft::ReactNative::IComponentState const& value);
  void Invalidate();

  winrt::Microsoft::UI::Xaml::XamlIsland island{nullptr};
  winrt::Microsoft::UI::Xaml::FrameworkElement element{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::Canvas host{nullptr};
  winrt::Microsoft::ReactNative::IComponentState state{nullptr};
  float layoutWidth{0};
  bool measuring{false};
  winrt::Microsoft::ReactNative::ComponentView::Destroying_revoker destroyingRevoker;
  winrt::Microsoft::ReactNative::ComponentView::LayoutMetricsChanged_revoker layoutRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::Loaded_revoker loadedRevoker;
};

void ApplyTheme(
    winrt::Microsoft::UI::Xaml::FrameworkElement const& element,
    std::string const& theme);

void PrepareElement(
    winrt::Microsoft::UI::Xaml::Controls::Control const& element,
    std::optional<std::string> const& theme,
    std::optional<bool> const& disabled);

void WrapTextBlocks(winrt::Microsoft::UI::Xaml::DependencyObject const& root);

winrt::hstring ToHString(std::optional<std::string> const& value);
winrt::hstring ToHString(std::string const& value);

void ConfigureXamlIsland(
    winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder const& builder,
    winrt::Windows::Foundation::Size fallback,
    std::function<void(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const&)> const&
        initialize);

} // namespace winrt::Winui
