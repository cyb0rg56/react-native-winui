#include "pch.h"

#include "ProgressBar.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/ProgressBar.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct ProgressBarComponentView
    : winrt::implements<ProgressBarComponentView, winrt::IInspectable>,
      XamlComponentView<
          ProgressBarComponentView,
          winuiCodegen::BaseProgressBar<ProgressBarComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::ProgressBar> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    Host(view, winrt::Microsoft::UI::Xaml::Controls::ProgressBar());
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::ProgressBarProps> const& newProps,
      winrt::com_ptr<winuiCodegen::ProgressBarProps> const& oldProps) noexcept override {
    winuiCodegen::BaseProgressBar<ProgressBarComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"ProgressBar.UpdateProps", [&] {
      if (!newProps || !m_control) {
        return;
      }

      bool changed = !oldProps;
      if (ChromeChanged(newProps, oldProps)) {
        PrepareElement(m_control, newProps->theme, newProps->disabled, newProps->ViewProps, m_session);
        changed = true;
      }
      if (!oldProps || oldProps->minimum != newProps->minimum) {
        m_control.Minimum(newProps->minimum.value_or(0));
        changed = true;
      }
      if (!oldProps || oldProps->maximum != newProps->maximum) {
        m_control.Maximum(newProps->maximum);
        changed = true;
      }
      if (!oldProps || oldProps->isIndeterminate != newProps->isIndeterminate) {
        m_control.IsIndeterminate(newProps->isIndeterminate.value_or(false));
        changed = true;
      }
      if (!oldProps || oldProps->value != newProps->value) {
        m_control.Value(newProps->value.value_or(0));
        changed = true;
      }
      if (changed) {
        m_session.Invalidate();
      }
    });
  }
};

} // namespace winrt::Winui

void RegisterProgressBarComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterProgressBarNativeComponent<winrt::Winui::ProgressBarComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::ProgressBarComponentView>(builder, {200, 4});
      });
}
