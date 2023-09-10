#include "GlobalContext.h"
#include "scroller.hpp"
#include "ui.h"
#include <ftxui/dom/elements.hpp> // for color, Fit, LIGHT, align_right, bold, DOUBLE
#include <ftxui/dom/table.hpp>    // for Table, TableSelection

using namespace ftxui;

class RankPageBase : public PageBase {
  public:
    RankPageBase(GlobalContext &ctx) : PageBase(ctx) {
        auto buttonBack = Button(
            "返回", [&] { switchPage(MainPage(ctx)); }, ButtonOption::Ascii());

        m_bottomButtons = Container::Horizontal({buttonBack});

        m_socket.write("userlist\n");
        auto is = m_socket.readStream();
        std::string _;
        std::getline(is, _);
        int n;
        is >> n;
        for (int i = 0; i < n; i++) {
            std::string name;
            int type;
            is >> type;
            std::getline(is, _);
            std::getline(is, name);
            if (type == 1) {
                std::string level, exp, expForNextLevel, levelPassed;
                is >> level >> exp >> expForNextLevel >> levelPassed;
                m_challengerList.push_back({name, level, exp, levelPassed});
            } else if (type == 2) {
                std::string level, madeNum, madeNumForNextLevel;
                is >> level >> madeNum >> madeNumForNextLevel;
                m_authorList.push_back({name, level, madeNum});
            }
        }
        static const std::vector<std::string> typeList = {"闯关者", "出题者"};
        static const std::vector<std::string> sortMethodControllerList = {"降序", "升序"};

        RadioboxOption radioOption;
        radioOption.on_change = [&] {
            m_sortMode = 0;
            m_sortBy = 1;
            m_filterBy = 0;
            m_filterText.clear();
        };

        m_typeController = Radiobox(&typeList, &m_type, radioOption);

        m_sortController = Container::Vertical({Radiobox(&m_headerList, &m_sortBy),
                                                Toggle(&sortMethodControllerList, &m_sortMode)});

        m_filterController = Container::Vertical({Radiobox(&m_headerList, &m_filterBy),
                                                  Input(&m_filterText, "输入筛选内容")});

        auto controllers = Container::Vertical({m_typeController, m_sortController, m_filterController});
        Add(Container::Vertical({controllers, m_bottomButtons}));
    }

  private:
    Element Render() override {
        if (m_type == 0) {
            m_headerList = {"名称", "等级", "经验", "通过关卡数"};
        } else {
            m_headerList = {"名称", "等级", "出题数"};
        }

        auto table = Table(makeList());

        table.SelectAll().Border(LIGHT);
        table.SelectAll().SeparatorVertical(LIGHT);
        table.SelectRow(0).SeparatorVertical(LIGHT);
        table.SelectRow(0).Border(LIGHT);

        return vbox({hbox({vbox({window(text("类别"), m_typeController->Render()),
                                 window(text("排序"), m_sortController->Render()),
                                 window(text("筛选"), m_filterController->Render())}),
                           table.Render()}),
                     m_bottomButtons->Render() | hcenter})
             | hcenter
             | size(WIDTH, EQUAL, 90)
             | size(HEIGHT, EQUAL, 30)
             | border;
    }

    std::vector<std::vector<std::string>> makeList() {
        std::vector<std::vector<std::string>> list;
        list.push_back(m_headerList);
        auto &originalList = (m_type == 0 ? m_challengerList : m_authorList);
        for (auto &i : originalList) {
            if (m_filterText.empty()) {
                list.push_back(i);
            } else if (m_filterBy == 0 && i[m_filterBy].find(m_filterText) != std::string::npos) {
                list.push_back(i);
            } else if (m_filterBy != 0 && i[m_filterBy] == m_filterText) {
                list.push_back(i);
            }
        }
        std::sort(list.begin() + 1, list.end(), [&](const std::vector<std::string> &a, const std::vector<std::string> &b) {
            bool result;
            if (m_sortBy == 0)
                return a[m_sortBy] > b[m_sortBy];
            else
                return stoi(a[m_sortBy]) > stoi(b[m_sortBy]);
        });
        if (m_sortMode) {
            std::reverse(list.begin() + 1, list.end());
        }
        return list;
    }
    Component m_typeController, m_sortController, m_filterController;
    Component m_bottomButtons;
    int m_type = 0;
    int m_sortMode = 0;
    int m_sortBy = 1;
    int m_filterBy = 0;
    std::string m_filterText;
    std::vector<std::string> m_headerList;
    std::vector<std::vector<std::string>> m_challengerList, m_authorList;
};

Component RankPage(GlobalContext &ctx) {
    return Make<RankPageBase>(ctx);
}
