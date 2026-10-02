#include "pch.h"

#include "InfoBar.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/InfoBar.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct InfoBarComponentView
    : winrt::implements<InfoBarComponentView, winrt::IInspectable>,
      winuiCodegen::BaseInfoBar<InfoBarComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_bar = winrt::Microsoft::UI::Xaml::Controls::InfoBar();
    auto weakThis = get_weak();
    m_sizeRevoker = m_bar.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->m_session.Invalidate();
      }
    });
    AttachEvents();
    m_session.Attach(view, m_bar);
  }

  ~InfoBarComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::InfoBarProps> const& newProps,
      winrt::com_ptr<winuiCodegen::InfoBarProps> const&) noexcept override {
    BaseInfoBar::UpdateProps(view, newProps, nullptr);
    if (!newProps || !m_bar) {
      return;
    }

    WithEventsSuspended([&] {
      PrepareElement(m_bar, newProps->theme, newProps->disabled);
      m_bar.Title(ToHString(newProps->title));
      m_bar.Message(ToHString(newProps->message));
      m_bar.Severity(SeverityFrom(newProps->severity.value_or("informational")));
      m_bar.IsClosable(newProps->isClosable);
      m_bar.IsOpen(newProps->isOpen);
      WrapTextBlocks(m_bar);
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
  static winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity SeverityFrom(std::string const& severity) {
    using winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity;
    if (severity == "success") {
      return InfoBarSeverity::Success;
    }
    if (severity == "warning") {
      return InfoBarSeverity::Warning;
    }
    if (severity == "error") {
      return InfoBarSeverity::Error;
    }
    return InfoBarSeverity::Informational;
  }

  void AttachEvents() {
    auto weakThis = get_weak();
    m_closeRevoker = m_bar.CloseButtonClick(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->EmitClose();
      }
    });
  }

  void WithEventsSuspended(auto&& action) {
    m_closeRevoker.revoke();
    action();
    AttachEvents();
  }

  void EmitClose() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::InfoBarSpec_onClose args{};
    args.isOpen = m_bar.IsOpen();
    emitter->onClose(std::move(args));
  }

  XamlIslandSession m_session;
  winrt::Microsoft::UI::Xaml::Controls::InfoBar m_bar{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::InfoBar::CloseButtonClick_revoker m_closeRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterInfoBarComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterInfoBarNativeComponent<winrt::Winui::InfoBarComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::ConfigureXamlIsland(builder, {360, 128}, [](auto const& view) {
          auto userData = winrt::make_self<winrt::Winui::InfoBarComponentView>();
          userData->Attach(view);
          view.UserData(*userData);
        });
      });
}
