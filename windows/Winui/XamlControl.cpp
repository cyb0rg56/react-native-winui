#include "pch.h"

#include "XamlControl.h"

#include <cmath>
#include <limits>

#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.UI.ViewManagement.h>

namespace winrt::Winui {

constexpr double kControlFontSize = 24.0;
constexpr double kControlMinHeight = 56.0;
constexpr double kToggleGlyph = 24.0;
constexpr double kRatingFontSize = 36.0;
constexpr double kBadgeSize = 28.0;

namespace {

bool SameSize(
    winrt::Windows::Foundation::Size const& left,
    winrt::Windows::Foundation::Size const& right) noexcept {
  return left.Width == right.Width && left.Height == right.Height;
}

struct MeasuringGuard {
  bool& measuring;
  explicit MeasuringGuard(bool& measuring) : measuring(measuring) {
    measuring = true;
  }
  ~MeasuringGuard() {
    measuring = false;
  }
};

enum class DensityKind { Standard, Toggle, Rating, Badge };

bool Near(double value, double target) noexcept {
  return std::abs(value - target) < 0.5;
}

void ScaleSquare(winrt::Microsoft::UI::Xaml::FrameworkElement const& element, double from, double to) {
  if (Near(element.Width(), from) && Near(element.Height(), from)) {
    auto const scaled = element.Width() * (to / from);
    element.Width(scaled);
    element.Height(scaled);
  }
}

void ApplyVisualDensity(
    winrt::Microsoft::UI::Xaml::DependencyObject const& node,
    DensityKind kind) {
  if (!node) {
    return;
  }

  // The ComboBox popup is a child of the control. Leave its items at the
  // standard row size instead of stretching them to the closed-control chrome.
  if (node.try_as<winrt::Microsoft::UI::Xaml::Controls::Primitives::Popup>() ||
      node.try_as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>()) {
    return;
  }

  if (node.try_as<winrt::Microsoft::UI::Xaml::Controls::CheckBox>() ||
      node.try_as<winrt::Microsoft::UI::Xaml::Controls::RadioButton>()) {
    kind = DensityKind::Toggle;
  } else if (node.try_as<winrt::Microsoft::UI::Xaml::Controls::RatingControl>()) {
    kind = DensityKind::Rating;
  } else if (node.try_as<winrt::Microsoft::UI::Xaml::Controls::InfoBadge>()) {
    kind = DensityKind::Badge;
  }

  auto const fontSize = kind == DensityKind::Rating ? kRatingFontSize : kControlFontSize;

  if (kind == DensityKind::Toggle) {
    // The glyph row is a fixed 32px grid pinned to the top, while the label is
    // centered in the control. That leaves the box sitting above the text once
    // the label is larger than the glyph.
    if (auto const control = node.try_as<winrt::Microsoft::UI::Xaml::Controls::Control>()) {
      control.VerticalContentAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);
      control.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::MinHeightProperty());
    }
    if (auto const presenter = node.try_as<winrt::Microsoft::UI::Xaml::Controls::ContentPresenter>()) {
      presenter.FontSize(fontSize);
      presenter.VerticalAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);
      presenter.Margin(winrt::Microsoft::UI::Xaml::Thickness{8, 0, 0, 0});
    }
    if (auto const text = node.try_as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>()) {
      text.FontSize(fontSize);
      text.VerticalAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);
    }
    if (auto const icon = node.try_as<winrt::Microsoft::UI::Xaml::Controls::FontIcon>()) {
      icon.FontSize(18);
      icon.VerticalAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);
      icon.HorizontalAlignment(winrt::Microsoft::UI::Xaml::HorizontalAlignment::Center);
    }
    if (auto const element = node.try_as<winrt::Microsoft::UI::Xaml::FrameworkElement>()) {
      ScaleSquare(element, 20, kToggleGlyph);
      ScaleSquare(element, 12, kToggleGlyph * 12.0 / 20.0);
      if (Near(element.Height(), 32) &&
          element.VerticalAlignment() == winrt::Microsoft::UI::Xaml::VerticalAlignment::Top) {
        element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::HeightProperty());
        element.VerticalAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);
      }
    }
    if (auto const grid = node.try_as<winrt::Microsoft::UI::Xaml::Controls::Grid>()) {
      auto const columns = grid.ColumnDefinitions();
      if (columns.Size() > 0) {
        auto const column = columns.GetAt(0);
        auto const width = column.Width();
        if (width.GridUnitType == winrt::Microsoft::UI::Xaml::GridUnitType::Pixel && width.Value > 0 &&
            width.Value <= 22) {
          column.Width(winrt::Microsoft::UI::Xaml::GridLength{kToggleGlyph, winrt::Microsoft::UI::Xaml::GridUnitType::Pixel});
        }
      }
    }
  } else if (kind == DensityKind::Rating) {
    if (auto const rating = node.try_as<winrt::Microsoft::UI::Xaml::Controls::RatingControl>()) {
      rating.FontSize(kRatingFontSize);
      rating.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::MinHeightProperty());
    }
    if (auto const text = node.try_as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>()) {
      text.FontSize(kRatingFontSize);
    }
    if (auto const icon = node.try_as<winrt::Microsoft::UI::Xaml::Controls::FontIcon>()) {
      icon.FontSize(kRatingFontSize);
    }
  } else if (kind == DensityKind::Badge) {
    if (auto const badge = node.try_as<winrt::Microsoft::UI::Xaml::Controls::InfoBadge>()) {
      badge.FontSize(16);
      badge.MinWidth(kBadgeSize);
      badge.MinHeight(kBadgeSize);
      badge.Height(kBadgeSize);
      // A dot has no label, so the template gives it a tiny explicit width.
      if (badge.Value() < 0) {
        badge.Width(kBadgeSize);
      }
    }
    if (auto const text = node.try_as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>()) {
      text.FontSize(16);
    }
    if (auto const element = node.try_as<winrt::Microsoft::UI::Xaml::FrameworkElement>()) {
      if (!element.try_as<winrt::Microsoft::UI::Xaml::Controls::InfoBadge>() && element.Width() > 0 &&
          element.Width() < kBadgeSize && element.Height() > 0 && element.Height() < kBadgeSize &&
          Near(element.Width(), element.Height())) {
        element.Width(kBadgeSize);
        element.Height(kBadgeSize);
      }
    }
  } else {
    // Template parts stamp their own FontSize from theme resources, which hides
    // a FontSize set on the control itself.
    if (auto const text = node.try_as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>()) {
      text.FontSize(fontSize);
    } else if (auto const box = node.try_as<winrt::Microsoft::UI::Xaml::Controls::TextBox>()) {
      box.FontSize(fontSize);
      box.MinHeight(kControlMinHeight);
    } else if (auto const presenter = node.try_as<winrt::Microsoft::UI::Xaml::Controls::ContentPresenter>()) {
      presenter.FontSize(fontSize);
    } else if (auto const icon = node.try_as<winrt::Microsoft::UI::Xaml::Controls::FontIcon>()) {
      icon.FontSize(fontSize);
    }

    if (auto const element = node.try_as<winrt::Microsoft::UI::Xaml::FrameworkElement>()) {
      auto const minHeight = element.MinHeight();
      if (minHeight >= 28.0 && minHeight < kControlMinHeight) {
        element.MinHeight(kControlMinHeight);
      }
    }
  }

  auto const count = winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(node);
  for (int32_t index = 0; index < count; ++index) {
    ApplyVisualDensity(winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChild(node, index), kind);
  }
}

void ApplyVisualDensity(winrt::Microsoft::UI::Xaml::DependencyObject const& node) {
  ApplyVisualDensity(node, DensityKind::Standard);
}

} // namespace

void XamlIslandSession::Attach(
    winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view,
    winrt::Microsoft::UI::Xaml::UIElement const& content) {
  Close();
  element = content.as<winrt::Microsoft::UI::Xaml::FrameworkElement>();
  element.HorizontalAlignment(winrt::Microsoft::UI::Xaml::HorizontalAlignment::Left);
  element.VerticalAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Top);

  // A Canvas measures children with infinite space, so the control can report
  // its content size instead of the island's current slot.
  host = winrt::Microsoft::UI::Xaml::Controls::Canvas{};
  host.Children().Append(element);

  island = winrt::Microsoft::UI::Xaml::XamlIsland{};
  island.Content(host);
  view.Connect(island.ContentIsland());

  destroyingRevoker = view.Destroying(
      winrt::auto_revoke,
      [this](
          winrt::Windows::Foundation::IInspectable const&,
          winrt::Microsoft::ReactNative::ComponentView const&) { Close(); });

  layoutRevoker = view.LayoutMetricsChanged(
      winrt::auto_revoke,
      [this](
          winrt::Windows::Foundation::IInspectable const&,
          winrt::Microsoft::ReactNative::LayoutMetricsChangedArgs const& args) {
        layoutWidth = args.NewLayoutMetrics().Frame.Width;
        Invalidate();
      });

  loadedRevoker = element.Loaded(winrt::auto_revoke, [this](auto const&, auto const&) { Invalidate(); });
}

void XamlIslandSession::Close() noexcept {
  layoutRevoker.revoke();
  loadedRevoker.revoke();
  destroyingRevoker.revoke();
  element = nullptr;
  host = nullptr;
  state = nullptr;
  layoutWidth = 0;
  if (!island) {
    return;
  }

  auto closing = island;
  island = nullptr;
  closing.Content(nullptr);
  closing.Close();
}

void XamlIslandSession::SetState(winrt::Microsoft::ReactNative::IComponentState const& value) {
  state = value;
  Invalidate();
}

void XamlIslandSession::Invalidate() {
  if (measuring || !element || !state || !state.Data()) {
    return;
  }

  MeasuringGuard guard{measuring};

  auto const infinite = std::numeric_limits<float>::infinity();
  auto const applyChrome = [](winrt::Microsoft::UI::Xaml::FrameworkElement const& target) {
    if (auto const control = target.try_as<winrt::Microsoft::UI::Xaml::Controls::Control>()) {
      if (!control.try_as<winrt::Microsoft::UI::Xaml::Controls::ProgressBar>()) {
        ApplyVisualDensity(control);
      }
    }
  };

  // Stars and badge dots are created when the template is measured, so the
  // second pass sizes parts that did not exist yet.
  applyChrome(element);
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::WidthProperty());
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::HeightProperty());
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::MaxWidthProperty());
  element.Measure({infinite, infinite});
  applyChrome(element);
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::WidthProperty());
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::HeightProperty());
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::MaxWidthProperty());
  element.Measure({infinite, infinite});
  auto natural = element.DesiredSize();
  natural.Width = std::ceil(natural.Width);
  natural.Height = std::ceil(natural.Height);

  winrt::Windows::Foundation::Size wrapped{0, 0};
  float wrappedWidth = 0;
  auto const slot = layoutWidth;
  if (slot > 1.f && std::isfinite(slot) && std::abs(slot - natural.Width) > 1.f) {
    auto const width = std::ceil(slot);
    element.Width(width);
    element.Measure({width, infinite});
    wrapped = element.DesiredSize();
    wrapped.Width = width;
    wrapped.Height = std::ceil(wrapped.Height) + 1.f;
    wrappedWidth = width;
  }

  if (!(natural.Width > 0) || !(natural.Height > 0)) {
    return;
  }

  auto const current = winrt::get_self<MeasuredSize>(state.Data());
  auto const unchanged = current && SameSize(current->natural, natural) && SameSize(current->wrapped, wrapped) &&
      current->wrappedWidth == wrappedWidth;
  if (!unchanged) {
    state.UpdateStateWithMutation([natural, wrapped, wrappedWidth](winrt::Windows::Foundation::IInspectable const&) {
      auto measured = winrt::make_self<MeasuredSize>();
      measured->natural = natural;
      measured->wrapped = wrapped;
      measured->wrappedWidth = wrappedWidth;
      return measured.as<winrt::Windows::Foundation::IInspectable>();
    });
  }

}

void ApplyTheme(
    winrt::Microsoft::UI::Xaml::FrameworkElement const& element,
    std::string const& theme) {
  winrt::Windows::UI::ViewManagement::AccessibilitySettings accessibility;
  if (accessibility.HighContrast()) {
    element.HighContrastAdjustment(winrt::Microsoft::UI::Xaml::ElementHighContrastAdjustment::Auto);
    element.RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Default);
    return;
  }

  if (theme == "dark") {
    element.RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Dark);
  } else if (theme == "light") {
    element.RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Light);
  } else {
    element.RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Default);
  }
}

void ApplyControlDensity(winrt::Microsoft::UI::Xaml::Controls::Control const& element) {
  // Progress bars stay a thin track. Theme dictionaries are left alone: a larger
  // ControlContentThemeFontSize is inherited by popups such as the ComboBox list.
  if (element.try_as<winrt::Microsoft::UI::Xaml::Controls::ProgressBar>()) {
    return;
  }

  if (auto const badge = element.try_as<winrt::Microsoft::UI::Xaml::Controls::InfoBadge>()) {
    badge.FontSize(16);
    badge.MinWidth(kBadgeSize);
    badge.MinHeight(kBadgeSize);
    badge.Height(kBadgeSize);
    return;
  }

  if (auto const rating = element.try_as<winrt::Microsoft::UI::Xaml::Controls::RatingControl>()) {
    rating.FontSize(kRatingFontSize);
    return;
  }

  // A tall minimum pins the checkbox and radio glyphs to the top of the row
  // while the label stays vertically centered.
  if (element.try_as<winrt::Microsoft::UI::Xaml::Controls::CheckBox>() ||
      element.try_as<winrt::Microsoft::UI::Xaml::Controls::RadioButton>()) {
    element.FontSize(kControlFontSize);
    element.VerticalContentAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);
    element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::MinHeightProperty());
    return;
  }

  element.FontSize(kControlFontSize);
  element.MinHeight(kControlMinHeight);
}

void PrepareElement(
    winrt::Microsoft::UI::Xaml::Controls::Control const& element,
    std::optional<std::string> const& theme,
    std::optional<bool> const& disabled) {
  element.HorizontalAlignment(winrt::Microsoft::UI::Xaml::HorizontalAlignment::Stretch);
  element.VerticalAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Stretch);
  element.IsEnabled(!disabled.value_or(false));
  ApplyControlDensity(element);
  ApplyTheme(element, theme.value_or("system"));
}

void WrapTextBlocks(winrt::Microsoft::UI::Xaml::DependencyObject const& root) {
  if (!root) {
    return;
  }

  if (auto const text = root.try_as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>()) {
    text.TextWrapping(winrt::Microsoft::UI::Xaml::TextWrapping::Wrap);
  }

  auto const count = winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(root);
  for (int32_t index = 0; index < count; ++index) {
    WrapTextBlocks(winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChild(root, index));
  }
}

winrt::hstring ToHString(std::string const& value) {
  return winrt::to_hstring(value);
}

winrt::hstring ToHString(std::optional<std::string> const& value) {
  return winrt::to_hstring(value.value_or(std::string{}));
}

void ConfigureXamlIsland(
    winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder const& builder,
    winrt::Windows::Foundation::Size fallback,
    std::function<void(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const&)> const&
        initialize) {
  auto viewBuilder = builder.as<winrt::Microsoft::ReactNative::IReactViewComponentBuilder>();
  viewBuilder.XamlSupport(true);

  builder.SetContentIslandComponentViewInitializer(
      [initialize](winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& islandView) noexcept {
        initialize(islandView);
      });

  viewBuilder.SetInitialStateDataFactory(
      [](winrt::Microsoft::ReactNative::IComponentProps const&) noexcept {
        return winrt::make<MeasuredSize>();
      });

  viewBuilder.SetMeasureContentHandler(
      [fallback](
          winrt::Microsoft::ReactNative::ShadowNode const& shadowNode,
          winrt::Microsoft::ReactNative::LayoutContext const&,
          winrt::Microsoft::ReactNative::LayoutConstraints const& constraints) noexcept {
        auto const maxWidth = constraints.MaximumSize.Width;
        auto const minWidth = constraints.MinimumSize.Width;
        auto const widthExact = std::isfinite(maxWidth) && maxWidth > 0.f && std::abs(minWidth - maxWidth) < 0.5f;

        if (auto const data = shadowNode.StateData()) {
          auto const current = winrt::get_self<MeasuredSize>(data);
          if (current && current->natural.Width > 0.f && current->natural.Height > 0.f) {
            if (widthExact && current->wrapped.Height > 0.f && std::abs(current->wrappedWidth - maxWidth) < 1.f) {
              return winrt::Windows::Foundation::Size{maxWidth, current->wrapped.Height};
            }
            if (widthExact) {
              return winrt::Windows::Foundation::Size{maxWidth, current->natural.Height};
            }
            if (std::isfinite(maxWidth) && maxWidth > 0.f && current->natural.Width > maxWidth &&
                current->wrapped.Height > 0.f && std::abs(current->wrappedWidth - maxWidth) < 1.f) {
              return current->wrapped;
            }
            return current->natural;
          }
        }
        return fallback;
      });
}

} // namespace winrt::Winui
