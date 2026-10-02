#include "pch.h"

#include "ComboBox.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/ComboBox.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct ComboBoxComponentView
    : winrt::implements<ComboBoxComponentView, winrt::IInspectable>,
      XamlComponentView<
          ComboBoxComponentView,
          winuiCodegen::BaseComboBox<ComboBoxComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::ComboBox> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    auto combo = winrt::Microsoft::UI::Xaml::Controls::ComboBox();
    combo.SelectionChangedTrigger(winrt::Microsoft::UI::Xaml::Controls::ComboBoxSelectionChangedTrigger::Always);
    Host(view, combo);
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::ComboBoxProps> const& newProps,
      winrt::com_ptr<winuiCodegen::ComboBoxProps> const& oldProps) noexcept override {
    winuiCodegen::BaseComboBox<ComboBoxComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"ComboBox.UpdateProps", [&] {
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
        if (ItemsChanged(oldProps, newProps)) {
          m_items = newProps->items;
          m_control.Items().Clear();
          for (auto const& item : m_items) {
            auto comboItem = winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem();
            comboItem.Content(winrt::box_value(ToHString(item.label)));
            comboItem.FontSize(14);
            m_control.Items().Append(comboItem);
          }
          changed = true;
        }
        if (!oldProps || oldProps->selectedIndex != newProps->selectedIndex || ItemsChanged(oldProps, newProps)) {
          ApplySelection(*newProps);
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
    m_selectionRevoker = m_control.SelectionChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        GuardedCall(L"ComboBox.SelectionChanged", [&] { self->EmitSelect(); });
      }
    });
  }

  void DetachEvents() override {
    m_selectionRevoker.revoke();
  }

 private:
  static bool ItemsChanged(
      winrt::com_ptr<winuiCodegen::ComboBoxProps> const& oldProps,
      winrt::com_ptr<winuiCodegen::ComboBoxProps> const& newProps) {
    if (!oldProps || oldProps->items.size() != newProps->items.size()) {
      return true;
    }
    for (size_t index = 0; index < newProps->items.size(); ++index) {
      if (oldProps->items[index].label != newProps->items[index].label ||
          oldProps->items[index].value != newProps->items[index].value) {
        return true;
      }
    }
    return false;
  }

  void ApplySelection(winuiCodegen::ComboBoxProps const& props) {
    auto const selected = props.selectedIndex;
    if (selected >= 0 && selected < static_cast<int32_t>(m_control.Items().Size())) {
      m_control.SelectedIndex(selected);
    } else {
      m_control.SelectedIndex(-1);
    }
  }

  void EmitSelect() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }

    winuiCodegen::ComboBoxSpec_onSelect args{};
    args.itemIndex = m_control.SelectedIndex();
    if (args.itemIndex >= 0 && args.itemIndex < static_cast<int32_t>(m_items.size())) {
      auto const& item = m_items[static_cast<size_t>(args.itemIndex)];
      args.text = item.label;
      args.value = item.value.value_or(item.label);
    }
    emitter->onSelect(std::move(args));
    RestoreProps([this](winuiCodegen::ComboBoxProps const& props) { ApplySelection(props); });
  }

  std::vector<winuiCodegen::ComboBoxSpec_ComboBoxProps_items> m_items;
  winrt::Microsoft::UI::Xaml::Controls::ComboBox::SelectionChanged_revoker m_selectionRevoker;
};

} // namespace winrt::Winui

void RegisterComboBoxComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterComboBoxNativeComponent<winrt::Winui::ComboBoxComponentView>(
      packageBuilder,
      [](winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::ComboBoxComponentView>(builder, {160, 56});
      });
}
