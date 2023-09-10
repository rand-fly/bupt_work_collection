#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"

class RouterBase : public ftxui::ComponentBase {
  public:
    RouterBase() {
        using namespace ftxui;
        m_alertBox = Button(
                         "确认", [&] { m_showAlertBox = false; }, ButtonOption::Ascii())
                   | Renderer([&](Element inner) {
                         return vbox({text(m_alertMessage) | hcenter,
                                      inner | hcenter})
                              | border;
                     });
    }

    void switchPage(ftxui::Component page) {
        using namespace ftxui;
        if (!children_.empty()) {
            m_delayDelete = children_[0];
        }
        DetachAllChildren();
        Add(page | Modal(m_alertBox, &m_showAlertBox));
    }

    ftxui::Component currentPage() {
        return children_.empty() ? nullptr : children_[0];
    }

    void alert(const std::string &msg) {
        m_alertMessage = msg;
        m_showAlertBox = true;
    }

  private:
    ftxui::Component m_delayDelete;
    ftxui::Component m_alertBox;
    std::string m_alertMessage;
    bool m_showAlertBox = false;
};

inline std::shared_ptr<RouterBase> Router() {
    return ftxui::Make<RouterBase>();
}