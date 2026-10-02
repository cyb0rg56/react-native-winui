#include "pch.h"

#include "NumberBox.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/NumberBox.g.h"

#include <cmath>
#include <limits>

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

namespace {

double NumberMinimum(std::optional<double> const& value) {
  return value.value_or(std::numeric_limits<double>::lowest());
}

double NumberMaximum(std::optional<double> const& value) {
  return value.value_or((std::numeric_limits<double>::max)());
}

winrt::Microsoft::UI::Xaml::Controls::NumberBoxSpinButtonPlacementMode SpinMode(std::optional<std::string> const& value) {
  using winrt::Microsoft::UI::Xaml::Controls::NumberBoxSpinButtonPlacementMode;
  auto const mode = value.value_or("compact");
  if (mode == "hidden") {
    return NumberBoxSpinButtonPlacementMode::Hidden;
  }
  if (mode == "inline") {
    return NumberBoxSpinButtonPlacementMode::Inline;
  }
  return NumberBoxSpinButtonPlacementMode::Compact;
}

} // namespace

struct NumberBoxComponentView
    : winrt::implements<NumberBoxComponentView, winrt::IInspectable>,
      XamlComponentView<
          NumberBoxComponentView,
          winuiCodegen::BaseNumberBox<NumberBoxComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::NumberBox> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    Host(view, winrt::Microsoft::UI::Xaml::Controls::NumberBox());
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::NumberBoxProps> const& newProps,
      winrt::com_ptr<winuiCodegen::NumberBoxProps> const& oldProps) noexcept override {
    winuiCodegen::BaseNumberBox<NumberBoxComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"NumberBox.UpdateProps", [&] {
      if (!newProps || !m_control) {
        return;
      }

      bool changed = !oldProps;
      WithEventsSuspended([&] {
        if (ChromeChanged(newProps, oldProps)) {
          PrepareElement(m_control, newProps->theme, newProps->disabled, newProps->ViewProps, m_session);
          changed = true;
        }
        if (!oldProps || oldProps->placeholder != newProps->placeholder) {
          m_control.PlaceholderText(ToHString(newProps->placeholder));
          changed = true;
        }
        if (!oldProps || oldProps->minimum != newProps->minimum) {
          m_control.Minimum(NumberMinimum(newProps->minimum));
          changed = true;
        }
        if (!oldProps || oldProps->maximum != newProps->maximum) {
          m_control.Maximum(NumberMaximum(newProps->maximum));
          changed = true;
        }
        if (!oldProps || oldProps->step != newProps->step) {
          m_control.SmallChange(newProps->step.value_or(1));
          changed = true;
        }
        if (!oldProps || oldProps->spinButtons != newProps->spinButtons) {
          m_control.SpinButtonPlacementMode(SpinMode(newProps->spinButtons));
          changed = true;
        }
        if (!oldProps || oldProps->value != newProps->value) {
          ApplyValue(*newProps);
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
    m_valueRevoker = m_control.ValueChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        GuardedCall(L"NumberBox.ValueChanged", [&] { self->EmitValue(); });
      }
    });
  }

  void DetachEvents() override {
    m_valueRevoker.revoke();
  }

 private:
  void ApplyValue(winuiCodegen::NumberBoxProps const& props) {
    if (props.value && std::isfinite(*props.value)) {
      m_control.Value(*props.value);
    } else {
      m_control.Value(std::numeric_limits<double>::quiet_NaN());
    }
  }

  void EmitValue() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    auto const raw = m_control.Value();
    winuiCodegen::NumberBoxSpec_onValueChange args{};
    args.isEmpty = !std::isfinite(raw);
    args.value = args.isEmpty ? 0 : raw;
    emitter->onValueChange(std::move(args));
    RestoreProps([this](winuiCodegen::NumberBoxProps const& props) { ApplyValue(props); });
  }

  winrt::Microsoft::UI::Xaml::Controls::NumberBox::ValueChanged_revoker m_valueRevoker;
};

} // namespace winrt::Winui

void RegisterNumberBoxComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterNumberBoxNativeComponent<winrt::Winui::NumberBoxComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::NumberBoxComponentView>(builder, {160, 56});
      });
}
