// 表: TableView でセルを選び、その場で編集する(表計算の流儀)。合計の列は、点を書き換えると計算し直す。
// セルを選んで文字を打つと上書き、F2・ダブルクリックで続きから編集。Enter で下へ、Tab で右へ。Ctrl+C・Ctrl+V で写す・貼る。
#include <cstdlib>

#include "gallery.hpp"

namespace
{

// 点を書き換えたら、合計(5 列目)を計算し直すモデル。
class ScoreModel : public tokotachi::ListModel
{
public:
    explicit ScoreModel(tokotachi::Object* owner) : tokotachi::ListModel(owner) {}

    void set_text(const tokotachi::ModelIndex& index, int column, const std::string& text) override
    {
        tokotachi::ListModel::set_text(index, column, text);
        tokotachi::Item* item = items.get()[static_cast<std::size_t>(index.row())];
        std::vector<std::string> texts = item->column_texts;  // 2 列目以降
        int total = 0;
        for (std::size_t k = 0; k < 3 && k < texts.size(); ++k)
        {
            total += std::atoi(texts[k].c_str());
        }
        if (texts.size() > 3 && texts[3] != std::to_string(total))
        {
            texts[3] = std::to_string(total);
            item->column_texts = texts;
        }
    }
};

}  // namespace

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("表", 460, 300);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
    box->margins = tokotachi::Margins(12);
    box->spacing = 8;

    // 1 行が 1 人。1 列目は名前、2 列目以降(column_texts)は点と合計
    ScoreModel* scores = new ScoreModel(&window);
    const char* const NAMES[] = {"佐藤", "鈴木", "高橋", "田中", "伊藤", "渡辺"};
    const int POINTS[][3] = {{82, 75, 90}, {64, 88, 71}, {95, 92, 87}, {70, 66, 79}, {58, 81, 74}, {88, 79, 93}};
    for (int i = 0; i < 6; ++i)
    {
        tokotachi::Item* item = scores->add_item(NAMES[i]);
        std::vector<std::string> texts;
        int total = 0;
        for (int k = 0; k < 3; ++k)
        {
            texts.push_back(std::to_string(POINTS[i][k]));
            total += POINTS[i][k];
        }
        texts.push_back(std::to_string(total));
        item->column_texts = texts;
        item->editable = true;
    }

    tokotachi::TableView* table_view_scores = new tokotachi::TableView(&window);
    table_view_scores->add_column("名前", 90);
    const char* const SUBJECTS[] = {"国語", "数学", "英語", "合計"};
    for (int k = 0; k < 4; ++k)
    {
        table_view_scores->add_column(SUBJECTS[k], 70)->alignment = tokotachi::Align::END;
    }
    table_view_scores->model = scores;
    table_view_scores->selection_mode = tokotachi::SelectionMode::MULTI;

    tokotachi::Label* label_status = new tokotachi::Label(&window, "セルを選んで編集できます");
    table_view_scores->on_selection_change = [=](tokotachi::Object*)
    {
        const std::vector<tokotachi::CellRange> ranges = table_view_scores->selected_ranges;
        int cells = 0;
        for (std::size_t k = 0; k < ranges.size(); ++k)
        {
            cells += (ranges[k].last_row - ranges[k].first_row + 1) * (ranges[k].last_column - ranges[k].first_column + 1);
        }
        label_status->text = std::to_string(cells) + " 個のセルを選んでいます";
    };
    table_view_scores->select_range(tokotachi::CellRange(1, 1, 2, 3));

    box->add(table_view_scores, 1);
    box->add(label_status);
    return run_gallery(app, window, argc, argv);
}
