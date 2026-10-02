#include "pch.h"

#include "Expander.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/Expander.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

// Header and body are strings. React children are not hosted inside the island.
struct ExpanderComponentView
    : winrt::implements<ExpanderComponentView, winrt::IInspectable>,
      winuiCodegen::BaseExpander<ExpanderComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_expander = winrt::Microsoft::UI::Xaml::Controls::Expander();
    m_content = winrt::Microsoft::UI::Xaml::Controls::TextBlock();
    m_content.TextWrapping(winrt::Microsoft::UI::Xaml::TextWrapping::Wrap);
    m_expander.Content(m_content);

    auto weakThis = get_weak();
    m_sizeRevoker = m_expander.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->m_session.Invalidate();
      }
    });
    AttachEvents();
    m_session.Attach(view, m_expander);
  }

  ~ExpanderComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::ExpanderProps> const& newProps,
      winrt::com_ptr<winuiCodegen::ExpanderProps> const&) noexcept override {
    BaseExpander::UpdateProps(view, newProps, nullptr);
    if (!newProps || !m_expander) {
      return;
    }

    WithEventsSuspended([&] {
      PrepareElement(m_expander, newProps->theme, newProps->disabled);
      m_expander.Header(winrt::box_value(ToHString(newProps->header)));
      m_content.Text(ToHString(newProps->content));
      m_expander.IsExpanded(newProps->isExpanded.value_or(false));
      WrapTextBlocks(m_expander);
    });
    m_session.Invalidate();
  }

  void UpdateState(
      winrt::Microsoft::ReactNative::ComponentView const&,
      winrt::Microsoft::ReactNative::IComponentState const& newState) noexcept override {
    m_state = newState;
    m_session.SetState(newState);
  }

 private:
  void AttachEvents() {
    auto weakThis = get_weak();
    auto handler = [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->EmitChange();
      }
    };
    m_expandingRevoker = m_expander.Expanding(winrt::auto_revoke, handler);
    m_collapsedRevoker = m_expander.Collapsed(winrt::auto_revoke, handler);
  }

  void WithEventsSuspended(auto&& action) {
    m_expandingRevoker.revoke();
    m_collapsedRevoker.revoke();
    action();
    AttachEvents();
  }

  void EmitChange() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::ExpanderSpec_onExpandChange args{};
    args.isExpanded = m_expander.IsExpanded();
    emitter->onExpandChange(std::move(args));
  }

  XamlIslandSession m_session;
  winrt::Microsoft::UI::Xaml::Controls::Expander m_expander{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::TextBlock m_content{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::Expander::Expanding_revoker m_expandingRevoker;
  winrt::Microsoft::UI::Xaml::Controls::Expander::Collapsed_revoker m_collapsedRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterExpanderComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterExpanderNativeComponent<winrt::Winui::ExpanderComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::ConfigureXamlIsland(builder, {360, 180}, [](auto const& view) {
          auto userData = winrt::make_self<winrt::Winui::ExpanderComponentView>();
          userData->Attach(view);
          view.UserData(*userData);
        });
      });
}
