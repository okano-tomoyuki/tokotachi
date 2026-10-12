// Startup code created with the Tokotachi designer. Regions enclosed in markers are overwritten when regenerated.
// <tokotachi-designer:begin id="tokotachi_includes">
#include <tokotachi/tokotachi.hpp>

#include "main_window.hpp"
// <tokotachi-designer:end id="tokotachi_includes" hash="ea5892c8">

int main(int argc, char** argv)
{
    // <tokotachi-designer:begin id="tokotachi_main">
    tokotachi::Application app(argc, argv);
    MainWindow* window_main = new MainWindow();
    app.main_window = window_main;
    window_main->visible = true;
    return app.exec();
    // <tokotachi-designer:end id="tokotachi_main" hash="c741acc4">
}
