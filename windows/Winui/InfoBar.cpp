#include "pch.h"

#include "InfoBar.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/InfoBar.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct InfoBarComponentView
    : winrt::implements<InfoBarComponentView, winrt::IInspectable>,
      XamlComponentView<
          InfoBarComponentView,
          winuiCodegen::BaseInfoBar<InfoBarComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::InfoBar> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    Host(view, winrt::Microsoft::UI::Xaml::Controls::InfoBar());
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::InfoBarProps> const& newProps,
      winrt::com_ptr<winuiCodegen::InfoBarProps> const& oldProps) noexcept override {
    winuiCodegen::BaseInfoBar<InfoBarComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"InfoBar.UpdateProps", [&] {
      if (!newProps || !m_control) {
        return;
      }

      bool changed = !oldProps;
      WithEventsSuspended([&] {
        if (ChromeChanged(newProps, oldProps)) {
          PrepareElement(m_control, newProps->theme, newProps->disabled, newProps->ViewProps, m_session);
          changed = true;
        }
        if (!oldProps || oldProps->title != newProps->title) {
          m_control.Title(ToHString(newProps->title));
          changed = true;
        }
        if (!oldProps || oldProps->message != newProps->message) {
          m_control.Message(ToHString(newProps->message));
          WrapTextBlocks(m_control);
          changed = true;
        }
        if (!oldProps || oldProps->severity != newProps->severity) {
          m_control.Severity(SeverityFrom(newProps->severity.value_or("informational")));
          changed = true;
        }
        if (!oldProps || oldProps->isClosable != newProps->isClosable) {
          m_control.IsClosable(newProps->isClosable);
          changed = true;
        }
        if (!oldProps || oldProps->isOpen != newProps->isOpen) {
          ApplyOpen(*newProps);
          changed = true;
        }
      });
      if (changed) {
        m_session.Invalidate();
      }
    });
  }

 protected:
  void AttachEvents() override {
    auto weakThis = get_weak();
    // Closing fires before IsOpen becomes false. Cancel keeps the prop in
    // charge and the event reports the close the parent can accept.
    m_closingRevoker = m_control.Closing(
        winrt::auto_revoke,
        [weakThis](auto const&, winrt::Microsoft::UI::Xaml::Controls::InfoBarClosingEventArgs const& args) {
          args.Cancel(true);
          if (auto self = weakThis.get()) {
            GuardedCall(L"InfoBar.Closing", [&] { self->EmitClose(); });
          }
        });
  }

  void DetachEvents() override {
    m_closingRevoker.revoke();
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

  void ApplyOpen(winuiCodegen::InfoBarProps const& props) {
    m_control.IsOpen(props.isOpen);
  }

  void EmitClose() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::InfoBarSpec_onClose args{};
    args.isOpen = false;
    emitter->onClose(std::move(args));
    RestoreProps([this](winuiCodegen::InfoBarProps const& props) { ApplyOpen(props); });
  }

  winrt::Microsoft::UI::Xaml::Controls::InfoBar::Closing_revoker m_closingRevoker;
};

} // namespace winrt::Winui

void RegisterInfoBarComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterInfoBarNativeComponent<winrt::Winui::InfoBarComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::InfoBarComponentView>(builder, {320, 64});
      });
}
