#include "pch.h"

#include "InfoBadge.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/InfoBadge.g.h"

#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct InfoBadgeComponentView
    : winrt::implements<InfoBadgeComponentView, winrt::IInspectable>,
      winuiCodegen::BaseInfoBadge<InfoBadgeComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_badge = winrt::Microsoft::UI::Xaml::Controls::InfoBadge();
    winrt::Microsoft::UI::Xaml::Automation::AutomationProperties::SetName(m_badge, L"Badge");
    auto weakThis = get_weak();
    m_sizeRevoker = m_badge.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->m_session.Invalidate();
      }
    });
    m_session.Attach(view, m_badge);
  }

  ~InfoBadgeComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::InfoBadgeProps> const& newProps,
      winrt::com_ptr<winuiCodegen::InfoBadgeProps> const&) noexcept override {
    BaseInfoBadge::UpdateProps(view, newProps, nullptr);
    if (!newProps || !m_badge) {
      return;
    }

    PrepareElement(m_badge, newProps->theme, newProps->disabled);
    m_badge.Value(newProps->value);
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
  winrt::Microsoft::UI::Xaml::Controls::InfoBadge m_badge{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterInfoBadgeComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterInfoBadgeNativeComponent<winrt::Winui::InfoBadgeComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::ConfigureXamlIsland(builder, {28, 28}, [](auto const& view) {
          auto userData = winrt::make_self<winrt::Winui::InfoBadgeComponentView>();
          userData->Attach(view);
          view.UserData(*userData);
        });
      });
}
