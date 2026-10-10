// デザイナーのキャンバスの描画の部分(docs/adr/0039)。WebAssembly にして、拡張(Node.js)の中で動かす。
//
// 拡張は定義ファイルから「部品を組む操作の並び」(designer/packages/codegen/src/model.ts)を作って渡し、ここで実行して
// 画面なし(SDL の offscreen)で描く。生成したコードも同じ並びから作るので、描いた画面は生成したコードが組む画面と同じになる。
//
//   ttc_configure(config)  拡張を動かしている環境(フォントのファイル・OS・デスクトップ)を JSON で渡す。最初の描画の前に 1 回呼ぶ
//   ttc_render(request)    操作の並びを実行して描き、結果(大きさ・画素の場所・各変数の部品の位置)を JSON で返す。
//                          画像のプロパティの値(パス)は、request の images(パス → base64 の中身)で画像にする
//   ttc_defaults()         カタログの各クラスを作って、プロパティの既定値と望む大きさを JSON で返す(カタログの既定値の測定)
#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include <emscripten/emscripten.h>
#include <tokotachi/tokotachi.hpp>

#include "json.hpp"
#include "table.hpp"
#include "values.hpp"

namespace
{

tokotachi::Application* g_app = nullptr;
/// 描いたウィンドウ(次の描画で破棄する)。Dialog のときは、その持ち主の隠れたウィンドウも。
tokotachi::Window* g_window = nullptr;
tokotachi::Window* g_owner = nullptr;
/// 返す文字・画素(次の呼び出しまで有効)。
std::string g_result;
std::vector<std::uint8_t> g_pixels;

void ensure_app()
{
    if (!g_app)
    {
        // ブラウザの画面(SDL の emscripten のドライバ)は無いので、画面なしで描く
        setenv("SDL_VIDEO_DRIVER", "offscreen", 1);
        static char name[] = "tokotachi-canvas";
        static char* argv[] = {name, nullptr};
        g_app = new tokotachi::Application(1, argv);
    }
}

void discard_window()
{
    delete g_window;
    g_window = nullptr;
    delete g_owner;
    g_owner = nullptr;
}

/// 操作の並びを実行する。
class Runner
{
public:
    Runner(tokotachi::Window* window, const std::string& window_class) : window_(window)
    {
        vars_["this"] = window;
        classes_["this"] = window_class;
    }

    void run(const ttc::Json& ops)
    {
        for (std::size_t i = 0; i < ops.items().size(); ++i)
        {
            const ttc::Json& op = ops.items()[i];
            const std::string kind = op["op"].as_string();
            if (kind == "new")
            {
                run_new(op);
            }
            else if (kind == "set")
            {
                run_set(op);
            }
            else if (kind == "call")
            {
                run_call(op);
            }
            else if (kind == "place")
            {
                run_place(op);
            }
            else if (kind == "variable")
            {
                run_variable(op);
            }
            else if (kind == "bind")
            {
                run_bind(op);
            }
            // connect(ハンドラ)と blank は描画に関係しない
        }
    }

    const std::vector<std::string>& errors() const { return errors_; }
    const std::map<std::string, tokotachi::Object*>& vars() const { return vars_; }

private:
    void error(const std::string& message) { errors_.push_back(message); }

    tokotachi::Object* var(const std::string& name)
    {
        std::map<std::string, tokotachi::Object*>::const_iterator it = vars_.find(name);
        return it == vars_.end() ? nullptr : it->second;
    }

    /// target が無ければウィンドウ自身。
    std::string target_of(const ttc::Json& op) const { return op.has("target") ? op["target"].as_string() : "this"; }

    void define(const std::string& name, const std::string& cls, tokotachi::Object* object)
    {
        vars_[name] = object;
        classes_[name] = cls;
    }

    void run_new(const ttc::Json& op)
    {
        const std::string cls = op["class"].as_string();
        tokotachi::Object* parent = op.has("parent") ? var(op["parent"].as_string()) : nullptr;
        tokotachi::Object* object = ttc::create(cls, parent);
        if (!object)
        {
            error("cannot create " + cls);
            return;
        }
        define(op["variable"].as_string(), cls, object);
    }

    void run_set(const ttc::Json& op)
    {
        const std::string target = target_of(op);
        tokotachi::Object* object = var(target);
        const std::string property = op["property"].as_string();
        if (!object)
        {
            error("unknown variable " + target);
            return;
        }
        const ttc::Json& type = op["type"];
        if (type.is_object() && type.has("ref"))
        {
            set_reference(object, property, var(op["value"].as_string()));
            return;
        }
        if (!ttc::set_property(object, classes_[target], property, op["value"]))
        {
            error("cannot set " + classes_[target] + "." + property);
        }
    }

    /// Variable は作らず、値だけを覚える(キャンバスでは値が変わらないので、結びつけは値の設定と同じ)。
    void run_variable(const ttc::Json& op)
    {
        if (op.has("value"))
        {
            variables_[op["variable"].as_string()] = op["value"];
            return;
        }
        // 初期値を省くと T()
        const std::string type = op["type"].as_string();
        variables_[op["variable"].as_string()] =
            type == "string" ? ttc::Json(std::string()) : type == "bool" ? ttc::Json(false) : ttc::Json(0.0);
    }

    /// tokotachi::bind(target->property, variable->value) は、結んだ時点でプロパティを Variable の値にする。
    void run_bind(const ttc::Json& op)
    {
        const std::string target = target_of(op);
        tokotachi::Object* object = var(target);
        const std::string property = op["property"].as_string();
        std::map<std::string, ttc::Json>::const_iterator it = variables_.find(op["variable"].as_string());
        if (!object || it == variables_.end())
        {
            error("cannot bind " + target + "." + property);
            return;
        }
        if (!ttc::set_property(object, classes_[target], property, it->second))
        {
            error("cannot set " + classes_[target] + "." + property);
        }
    }

    /// 参照のプロパティ(ウィンドウのメニューバー等、ScrollArea の中身)。
    void set_reference(tokotachi::Object* object, const std::string& property, tokotachi::Object* value)
    {
        if (property == "menu_bar" || property == "tool_bar" || property == "status_bar")
        {
            if (property == "menu_bar")
            {
                window_->menu_bar = dynamic_cast<tokotachi::MenuBar*>(value);
            }
            else if (property == "tool_bar")
            {
                window_->tool_bar = dynamic_cast<tokotachi::ToolBar*>(value);
            }
            else
            {
                window_->status_bar = dynamic_cast<tokotachi::StatusBar*>(value);
            }
            return;
        }
        tokotachi::ScrollArea* scroll = dynamic_cast<tokotachi::ScrollArea*>(object);
        if (scroll && property == "content")
        {
            scroll->content = dynamic_cast<tokotachi::Widget*>(value);
            return;
        }
        error("cannot set reference " + property);
    }

    // ---- 呼び出しの引数 ----

    static const ttc::Json& arg(const ttc::Json& args, std::size_t i)
    {
        static const ttc::Json null_value;
        return i < args.items().size() ? args.items()[i] : null_value;
    }

    tokotachi::Object* arg_object(const ttc::Json& args, std::size_t i) { return var(arg(args, i)["name"].as_string()); }

    tokotachi::Widget* arg_widget(const ttc::Json& args, std::size_t i)
    {
        return dynamic_cast<tokotachi::Widget*>(arg_object(args, i));
    }

    static int arg_int(const ttc::Json& args, std::size_t i, int fallback)
    {
        const ttc::Json& a = arg(args, i);
        return a.is_null() ? fallback : static_cast<int>(a["value"].as_number());
    }

    static float arg_float(const ttc::Json& args, std::size_t i, float fallback)
    {
        const ttc::Json& a = arg(args, i);
        return a.is_null() ? fallback : static_cast<float>(a["value"].as_number());
    }

    static std::string arg_string(const ttc::Json& args, std::size_t i)
    {
        return arg(args, i)["value"].as_string();
    }

    template <typename E>
    static E arg_enum(const ttc::Json& args, std::size_t i, E fallback)
    {
        E v = fallback;
        const ttc::Json& a = arg(args, i);
        if (!a.is_null())
        {
            ttc::from_json(a["value"], v);
        }
        return v;
    }

    // ---- 呼び出し ----

    void run_call(const ttc::Json& op)
    {
        const std::string target = target_of(op);
        tokotachi::Object* object = var(target);
        const std::string cls = classes_[target];
        const std::string method = op["method"].as_string();
        const ttc::Json& args = op["args"];
        if (!object)
        {
            error("unknown variable " + target);
            return;
        }
        tokotachi::Object* result = call(object, cls, method, args);
        if (op.has("result"))
        {
            const ttc::Json& r = op["result"];
            if (result)
            {
                define(r["variable"].as_string(), r["class"].as_string(), result);
            }
            else
            {
                error("no result from " + cls + "::" + method);
            }
        }
    }

    tokotachi::Object* call(tokotachi::Object* object, const std::string& cls, const std::string& method,
                            const ttc::Json& args)
    {
        using tokotachi::Align;
        if (tokotachi::BoxLayout* box = dynamic_cast<tokotachi::BoxLayout*>(object))
        {
            if (method == "add")
            {
                box->add(arg_widget(args, 0), arg_int(args, 1, 0), arg_enum(args, 2, Align::FILL));
            }
            else if (method == "add_stretch")
            {
                box->add_stretch(arg_int(args, 0, 1));
            }
            else if (method == "add_spacing")
            {
                box->add_spacing(arg_float(args, 0, 0));
            }
            else
            {
                unknown(cls, method);
            }
            return nullptr;
        }
        if (tokotachi::GridLayout* grid = dynamic_cast<tokotachi::GridLayout*>(object))
        {
            if (method == "add")
            {
                grid->add(arg_widget(args, 0), arg_int(args, 1, 0), arg_int(args, 2, 0), arg_int(args, 3, 1),
                          arg_int(args, 4, 1), arg_enum(args, 5, Align::FILL), arg_enum(args, 6, Align::CENTER));
            }
            else if (method == "set_row_stretch")
            {
                grid->set_row_stretch(arg_int(args, 0, 0), arg_int(args, 1, 0));
            }
            else if (method == "set_column_stretch")
            {
                grid->set_column_stretch(arg_int(args, 0, 0), arg_int(args, 1, 0));
            }
            else if (method == "set_row_minimum")
            {
                grid->set_row_minimum(arg_int(args, 0, 0), arg_float(args, 1, 0));
            }
            else if (method == "set_column_minimum")
            {
                grid->set_column_minimum(arg_int(args, 0, 0), arg_float(args, 1, 0));
            }
            else
            {
                unknown(cls, method);
            }
            return nullptr;
        }
        if (tokotachi::Menu* menu = dynamic_cast<tokotachi::Menu*>(object))
        {
            if (method == "add_action")
            {
                menu->add_action(dynamic_cast<tokotachi::Action*>(arg_object(args, 0)));
            }
            else if (method == "add_separator")
            {
                menu->add_separator();
            }
            else if (method == "add_submenu")
            {
                return menu->add_submenu(arg_string(args, 0));
            }
            else
            {
                unknown(cls, method);
            }
            return nullptr;
        }
        if (tokotachi::MenuBar* bar = dynamic_cast<tokotachi::MenuBar*>(object))
        {
            if (method == "add_menu")
            {
                return bar->add_menu(arg_string(args, 0));
            }
        }
        if (tokotachi::ToolBar* tools = dynamic_cast<tokotachi::ToolBar*>(object))
        {
            if (method == "add_separator")
            {
                tools->add_separator();
                return nullptr;
            }
            if (method == "add_widget")
            {
                tools->add_widget(arg_widget(args, 0));
                return nullptr;
            }
        }
        if (tokotachi::StatusBar* status = dynamic_cast<tokotachi::StatusBar*>(object))
        {
            if (method == "add_widget")
            {
                status->add_widget(arg_widget(args, 0));
                return nullptr;
            }
        }
        if (tokotachi::Splitter* split = dynamic_cast<tokotachi::Splitter*>(object))
        {
            if (method == "add_widget")
            {
                return split->add_widget(arg_widget(args, 0), arg_int(args, 1, 1));
            }
        }
        if (tokotachi::TabWidget* tabs = dynamic_cast<tokotachi::TabWidget*>(object))
        {
            if (method == "add_tab")
            {
                return tabs->add_tab(arg_widget(args, 0), arg_string(args, 1));
            }
        }
        if (tokotachi::Dialog* dialog = dynamic_cast<tokotachi::Dialog*>(object))
        {
            if (method == "add_button")
            {
                dialog->add_button(arg_enum(args, 0, tokotachi::DialogButton::OK), arg_string(args, 1));
                return nullptr;
            }
        }
        if (tokotachi::Widget* widget = dynamic_cast<tokotachi::Widget*>(object))
        {
            if (method == "add_action")
            {
                widget->add_action(dynamic_cast<tokotachi::Action*>(arg_object(args, 0)));
                return nullptr;
            }
        }
        unknown(cls, method);
        return nullptr;
    }

    void unknown(const std::string& cls, const std::string& method) { error("unknown call " + cls + "::" + method); }

    void run_place(const ttc::Json& op)
    {
        tokotachi::PlaceLayout* place = dynamic_cast<tokotachi::PlaceLayout*>(var(op["layout"].as_string()));
        tokotachi::Widget* widget = dynamic_cast<tokotachi::Widget*>(var(op["widget"].as_string()));
        tokotachi::Placement p;
        if (!place || !widget || !ttc::from_json(op["placement"], p))
        {
            error("cannot place " + op["widget"].as_string());
            return;
        }
        place->add(widget, p);
    }

    tokotachi::Window* window_;
    std::map<std::string, tokotachi::Object*> vars_;
    std::map<std::string, std::string> classes_;
    /// Variable の名前 → 値。
    std::map<std::string, ttc::Json> variables_;
    std::vector<std::string> errors_;
};

/// 拡張が読んだ画像(パス → base64 の中身)を、プロパティに設定できるようにする。
void set_images(const ttc::Json& images)
{
    std::map<std::string, tokotachi::Image> decoded;
    for (std::size_t i = 0; i < images.members().size(); ++i)
    {
        const std::pair<std::string, ttc::Json>& m = images.members()[i];
        std::vector<unsigned char> bytes;
        if (m.second.is_string() && ttc::decode_base64(m.second.as_string(), bytes) && !bytes.empty())
        {
            decoded[m.first] = tokotachi::Image::load_from_data(&bytes[0], bytes.size());
        }
    }
    ttc::set_images(decoded);
}

ttc::Json number(double v)
{
    return ttc::Json(v);
}

/// 部品の位置と大きさ(ウィンドウの座標。dp)。
ttc::Json rect_of(tokotachi::Widget* w)
{
    const tokotachi::Point origin = w->map_to_window(tokotachi::Point(0, 0));
    const tokotachi::Rect b = w->bounds;
    ttc::Json r = ttc::Json::object();
    r.set("x", number(origin.x));
    r.set("y", number(origin.y));
    r.set("width", number(b.width));
    r.set("height", number(b.height));
    r.set("showing", ttc::Json(w->showing.get()));
    return r;
}

}  // namespace

extern "C"
{

/// config: {"fonts": [ファイル...], "os": "windows" | "linux" | ..., "desktop": XDG_CURRENT_DESKTOP}
/// ネイティブの Tokotachi が OS から得るもの(フォント・ダイアログのボタンの順)を、環境変数で渡す。
EMSCRIPTEN_KEEPALIVE void ttc_configure(const char* config_text)
{
    ttc::Json config;
    std::string error;
    if (!ttc::Json::parse(config_text, config, error))
    {
        return;
    }
    std::string fonts;
    for (std::size_t i = 0; i < config["fonts"].items().size(); ++i)
    {
        fonts += (i > 0 ? ";" : "") + config["fonts"].items()[i].as_string();
    }
    setenv("TOKOTACHI_FONT_FILES", fonts.c_str(), 1);
    setenv("TOKOTACHI_HOST_OS", config["os"].as_string().c_str(), 1);
    setenv("XDG_CURRENT_DESKTOP", config["desktop"].as_string().c_str(), 1);
}

EMSCRIPTEN_KEEPALIVE const char* ttc_render(const char* request_text)
{
    ensure_app();
    discard_window();
    ttc::Json request;
    std::string parse_error;
    ttc::Json result = ttc::Json::object();
    if (!ttc::Json::parse(request_text, request, parse_error))
    {
        result.set("error", ttc::Json(parse_error));
        g_result = result.dump();
        return g_result.c_str();
    }
    g_app->color_scheme =
        request["color_scheme"].as_string() == "DARK" ? tokotachi::ColorScheme::DARK : tokotachi::ColorScheme::LIGHT;
    set_images(request["images"]);

    const std::string window_class = request["window_class"].as_string();
    tokotachi::Dialog* dialog = nullptr;
    if (window_class == "Dialog")
    {
        g_owner = new tokotachi::Window(std::string());
        dialog = new tokotachi::Dialog(g_owner, std::string());
        g_window = dialog;
    }
    else
    {
        g_window = new tokotachi::Window(std::string());
    }
    Runner runner(g_window, window_class);
    runner.run(request["ops"]);
    if (dialog)
    {
        dialog->open();  // 大きさを中身に合わせる
    }
    else
    {
        g_window->visible = true;
    }

    tokotachi::Bitmap bitmap = g_window->capture();
    // アルファ乗算済みの 0xAARRGGBB(不透明)を、ImageData と同じ RGBA の並びにする
    const int count = bitmap.width() * bitmap.height();
    g_pixels.resize(static_cast<std::size_t>(count) * 4);
    const std::uint32_t* px = bitmap.pixels();
    for (int i = 0; i < count; ++i)
    {
        const std::uint32_t p = px[i];
        g_pixels[i * 4 + 0] = static_cast<std::uint8_t>(p >> 16);
        g_pixels[i * 4 + 1] = static_cast<std::uint8_t>(p >> 8);
        g_pixels[i * 4 + 2] = static_cast<std::uint8_t>(p);
        g_pixels[i * 4 + 3] = static_cast<std::uint8_t>(p >> 24);
    }

    result.set("width", number(bitmap.width()));
    result.set("height", number(bitmap.height()));
    result.set("scale", number(bitmap.scale()));
    result.set("window_width", number(g_window->width));
    result.set("window_height", number(g_window->height));
    result.set("pixels", number(static_cast<double>(reinterpret_cast<std::uintptr_t>(g_pixels.data()))));
    ttc::Json rects = ttc::Json::object();
    for (std::map<std::string, tokotachi::Object*>::const_iterator it = runner.vars().begin();
         it != runner.vars().end(); ++it)
    {
        tokotachi::Widget* w = dynamic_cast<tokotachi::Widget*>(it->second);
        if (w && it->first != "this")
        {
            rects.set(it->first, rect_of(w));
        }
    }
    result.set("rects", rects);
    ttc::Json errors = ttc::Json::array();
    for (std::size_t i = 0; i < runner.errors().size(); ++i)
    {
        errors.push(ttc::Json(runner.errors()[i]));
    }
    result.set("errors", errors);
    g_result = result.dump();
    return g_result.c_str();
}

EMSCRIPTEN_KEEPALIVE const char* ttc_defaults()
{
    ensure_app();
    discard_window();
    g_app->color_scheme = tokotachi::ColorScheme::LIGHT;
    tokotachi::Window window("");
    tokotachi::Widget* host = new tokotachi::Widget(&window);

    // 作れないクラス(ウィンドウと、呼び出しの結果)も、実物を用意して読む
    tokotachi::Dialog* dialog = new tokotachi::Dialog(&window, std::string());
    tokotachi::MenuBar* bar = new tokotachi::MenuBar(host);
    tokotachi::TabWidget* tabs = new tokotachi::TabWidget(host);
    tokotachi::Splitter* split = new tokotachi::Splitter(host);
    std::map<std::string, tokotachi::Object*> extra;
    extra["Window"] = &window;
    extra["Dialog"] = dialog;
    extra["Menu"] = bar->add_menu(std::string());
    extra["Tab"] = tabs->add_tab(new tokotachi::Widget(tabs), std::string());
    extra["SplitterPane"] = split->add_widget(new tokotachi::Widget(split));

    const char* const CLASSES[] = {"Window", "Dialog", "Menu", "Tab", "SplitterPane"};
    std::vector<std::string> classes = ttc::creatable_classes();
    classes.insert(classes.end(), CLASSES, CLASSES + 5);

    ttc::Json result = ttc::Json::object();
    for (std::size_t i = 0; i < classes.size(); ++i)
    {
        const std::string& cls = classes[i];
        tokotachi::Object* object = extra.count(cls) ? extra[cls] : nullptr;
        if (!object)
        {
            // レイアウトは子を持たない部品に、ほかは隠した入れ物に付ける
            object = ttc::create(cls, cls.find("Layout") != std::string::npos
                                          ? static_cast<tokotachi::Object*>(new tokotachi::Widget(host))
                                          : static_cast<tokotachi::Object*>(host));
        }
        if (!object)
        {
            continue;
        }
        ttc::Json info = ttc::Json::object();
        ttc::Json props = ttc::Json::object();
        const std::vector<std::string> names = ttc::property_names(cls);
        for (std::size_t j = 0; j < names.size(); ++j)
        {
            ttc::Json v;
            if (ttc::get_property(object, cls, names[j], v))
            {
                props.set(names[j], v);
            }
        }
        info.set("properties", props);
        if (tokotachi::Widget* w = dynamic_cast<tokotachi::Widget*>(object))
        {
            info.set("preferred_size", ttc::to_json(w->preferred_size.get()));
        }
        result.set(cls, info);
    }
    g_result = result.dump();
    return g_result.c_str();
}

}  // extern "C"
