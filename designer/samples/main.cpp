// Startup code created with the Tokotachi designer. Regions enclosed in markers are overwritten when regenerated.
// <tokotachi-designer:begin id="tokotachi_includes">
#include <tokotachi/tokotachi.hpp>

#include "main_window.hpp"
// <tokotachi-designer:end id="tokotachi_includes" hash="ea5892c8">

int main(int argc, char** argv)
{
    // <tokotachi-designer:begin id="tokotachi_main">
    tokotachi::Application app(argc, argv);
    MainWindow window;
    window.visible = true;
    return app.exec();
    // <tokotachi-designer:end id="tokotachi_main" hash="339b4d77">
}
