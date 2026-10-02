#include "pch.h"

#include "ProgressBar.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/ProgressBar.g.h"

#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct ProgressBarComponentView
    : winrt::implements<ProgressBarComponentView, winrt::IInspectable>,
      winuiCodegen::BaseProgressBar<ProgressBarComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_bar = winrt::Microsoft::UI::Xaml::Controls::ProgressBar();
    m_bar.Minimum(0);
    m_bar.Maximum(100);
    winrt::Microsoft::UI::Xaml::Automation::AutomationProperties::SetName(m_bar, L"Progress");
    auto weakThis = get_weak();
    m_sizeRevoker = m_bar.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->m_session.Invalidate();
      }
    });
    m_session.Attach(view, m_bar);
  }

  ~ProgressBarComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::ProgressBarProps> const& newProps,
      winrt::com_ptr<winuiCodegen::ProgressBarProps> const&) noexcept override {
    BaseProgressBar::UpdateProps(view, newProps, nullptr);
    if (!newProps || !m_bar) {
      return;
    }

    PrepareElement(m_bar, newProps->theme, newProps->disabled);
    m_bar.IsIndeterminate(newProps->isIndeterminate.value_or(false));
    m_bar.Value(newProps->value.value_or(0));
    m_session.Invalidate();
  }

  void UpdateState(
      winrt::Microsoft::ReactNative::ComponentView const&,
      winrt::Microsoft::ReactNative::IComponentState const& newState) noexcept override {
    m_state = newState;
    m_session.SetState(newState);
  }

 private:
  XamlIslandSession m_session;
  winrt::Microsoft::UI::Xaml::Controls::ProgressBar m_bar{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterProgressBarComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterProgressBarNativeComponent<winrt::Winui::ProgressBarComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::ConfigureXamlIsland(builder, {200, 8}, [](auto const& view) {
          auto userData = winrt::make_self<winrt::Winui::ProgressBarComponentView>();
          userData->Attach(view);
          view.UserData(*userData);
        });
      });
}
