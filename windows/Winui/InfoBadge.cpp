#include "pch.h"

#include "InfoBadge.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/InfoBadge.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct InfoBadgeComponentView
    : winrt::implements<InfoBadgeComponentView, winrt::IInspectable>,
      XamlComponentView<
          InfoBadgeComponentView,
          winuiCodegen::BaseInfoBadge<InfoBadgeComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::InfoBadge> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    Host(view, winrt::Microsoft::UI::Xaml::Controls::InfoBadge());
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::InfoBadgeProps> const& newProps,
      winrt::com_ptr<winuiCodegen::InfoBadgeProps> const& oldProps) noexcept override {
    winuiCodegen::BaseInfoBadge<InfoBadgeComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"InfoBadge.UpdateProps", [&] {
      if (!newProps || !m_control) {
        return;
      }

      bool changed = !oldProps;
      if (ChromeChanged(newProps, oldProps)) {
        PrepareElement(m_control, newProps->theme, newProps->disabled, newProps->ViewProps, m_session);
        changed = true;
      }
      if (!oldProps || oldProps->value != newProps->value) {
        m_control.Value(newProps->value);
        if (newProps->value < 0) {
          m_control.Width(28);
        } else {
          m_control.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::WidthProperty());
        }
        changed = true;
      }
      if (changed) {
        m_session.Invalidate();
      }
    });
  }
};

} // namespace winrt::Winui

void RegisterInfoBadgeComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterInfoBadgeNativeComponent<winrt::Winui::InfoBadgeComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::InfoBadgeComponentView>(builder, {28, 28});
      });
}
