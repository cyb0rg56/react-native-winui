#include "pch.h"

#include "ComboBox.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/ComboBox.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct ComboBoxComponentView
    : winrt::implements<ComboBoxComponentView, winrt::IInspectable>,
      winuiCodegen::BaseComboBox<ComboBoxComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_combo = winrt::Microsoft::UI::Xaml::Controls::ComboBox();
    m_combo.HorizontalAlignment(winrt::Microsoft::UI::Xaml::HorizontalAlignment::Stretch);
    m_combo.SelectionChangedTrigger(
        winrt::Microsoft::UI::Xaml::Controls::ComboBoxSelectionChangedTrigger::Always);

    auto weakThis = get_weak();
    m_sizeRevoker = m_combo.SizeChanged(
        winrt::auto_revoke, [weakThis](auto const&, auto const&) {
          if (auto self = weakThis.get()) {
            self->m_session.Invalidate();
          }
        });
    AttachEvents();
    m_session.Attach(view, m_combo);
  }

  ~ComboBoxComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::ComboBoxProps> const& newProps,
      winrt::com_ptr<winuiCodegen::ComboBoxProps> const& oldProps) noexcept override {
    BaseComboBox::UpdateProps(view, newProps, oldProps);
    if (!newProps || !m_combo) {
      return;
    }

    WithEventsSuspended([&] {
      PrepareElement(m_combo, newProps->theme, newProps->disabled);
      m_combo.PlaceholderText(ToHString(newProps->placeholder));

      bool rebuild = !oldProps || oldProps->items.size() != newProps->items.size();
      if (!rebuild) {
        for (size_t index = 0; index < newProps->items.size(); ++index) {
          if (oldProps->items[index].label != newProps->items[index].label ||
              oldProps->items[index].value != newProps->items[index].value) {
            rebuild = true;
            break;
          }
        }
      }

      if (rebuild) {
        m_items = newProps->items;
        m_combo.Items().Clear();
        for (auto const& item : m_items) {
          auto comboItem = winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem();
          comboItem.Content(winrt::box_value(ToHString(item.label)));
          // The ComboBox font is the closed-control size. Items keep the standard row.
          comboItem.FontSize(14);
          m_combo.Items().Append(comboItem);
        }
      }

      auto const selected = newProps->selectedIndex;
      if (selected >= 0 && selected < static_cast<int32_t>(m_combo.Items().Size())) {
        m_combo.SelectedIndex(selected);
      } else {
        m_combo.SelectedIndex(-1);
      }
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
    m_selectionRevoker = m_combo.SelectionChanged(
        winrt::auto_revoke, [weakThis](auto const&, auto const&) {
          if (auto self = weakThis.get()) {
            self->EmitSelect();
          }
        });
  }

  void WithEventsSuspended(auto&& action) {
    m_selectionRevoker.revoke();
    action();
    AttachEvents();
  }

  void EmitSelect() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }

    winuiCodegen::ComboBoxSpec_onSelect args{};
    args.itemIndex = m_combo.SelectedIndex();
    if (args.itemIndex >= 0 && args.itemIndex < static_cast<int32_t>(m_items.size())) {
      auto const& item = m_items[static_cast<size_t>(args.itemIndex)];
      args.text = item.label;
      args.value = item.value.value_or(item.label);
    }
    emitter->onSelect(std::move(args));
  }

  XamlIslandSession m_session;
  winrt::Microsoft::UI::Xaml::Controls::ComboBox m_combo{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  std::vector<winuiCodegen::ComboBoxSpec_ComboBoxProps_items> m_items;
  winrt::Microsoft::UI::Xaml::Controls::ComboBox::SelectionChanged_revoker m_selectionRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterComboBoxComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterComboBoxNativeComponent<winrt::Winui::ComboBoxComponentView>(
      packageBuilder,
      [](winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder const& builder) {
        winrt::Winui::ConfigureXamlIsland(
            builder,
            {160, 56},
            [](winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
              auto userData = winrt::make_self<winrt::Winui::ComboBoxComponentView>();
              userData->Attach(view);
              view.UserData(*userData);
            });
      });
}
