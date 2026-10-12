# Changelog

## Unreleased

For Tokotachi 0.3.

- **File formats**: window definitions and project files from older format versions are converted when opened (and
  written in the current format when saved). Files made by a newer designer are not opened; you are asked to update the
  designer instead.
- The project settings page shows the Tokotachi version the generated code is for.

- **Window ownership** (Tokotachi ADR 0062): the generated `main.cpp` creates the main window with `new` and sets it as
  `app.main_window`, so closing it quits the application. Generated window classes take an optional owner window
  (`explicit MainWindow(tokotachi::Window* owner = nullptr)`); pass one to show the window above its owner and destroy it
  together. The constructor is outside the generated regions, so existing files keep their constructor until you edit it.
- The generated `CMakeLists.txt` asks for the library version the designer was built for (`find_package(tokotachi 0.3)`).
- Windows and dialogs have the `destroy_on_close` property in the inspector.

## 0.2.0 (preview)

- **Selection and editing**: select several items (Ctrl/Shift+click, rubber band on the canvas), then delete, move,
  copy, edit or reorder them together (Alt+Up/Down). Copy, paste and duplicate widgets, actions, objects and variables.
- **Free placement**: align, distribute and match the size of freely placed widgets. Widgets placed partly outside
  their container are marked on the canvas and in the structure tree.
- **Reuse**: turn a group of widgets into a component file and place it in other windows. Your own widgets written in
  C++ appear in the palette once you register their headers in the project.
- **Themes**: override the theme per widget, register shared themes in the project, and preview them in the project
  settings.
- **Inspector**: built-in icons (Lucide), images, dates, colors, references to other widgets (such as the buddy of a
  label), ranges for numbers, and a resizable, collapsible pane.
- **New widgets** in the palette: ImageBox, GroupBox, TableView, DateEdit, Calendar and ColorButton.
- **Code generation**: embedded images are written to a separate source (`*_images.cpp`), and large images are
  reported. Handler stubs added on regeneration are placed in the same order as in a new file.
- **Projects**: a settings page for the windows, code generation and themes; links to the generated code.
- Palette tabs (widgets and non-visual objects), class icons in the palette and the structure tree, and an extension
  icon.
- Fixes: images chosen on another drive letter case, large images and large window definitions in the canvas.

## 0.1.0 (preview)

The first preview.

- Design windows and dialogs on a canvas drawn by Tokotachi (WebAssembly).
- Palette, structure tree, inspector, layouts (box, grid, free placement), menus, toolbars and actions.
- Variable bindings and images in the inspector.
- C++ code generation with marker regions and handler stubs; projects generate `main.cpp` and `CMakeLists.txt`.
- English and Japanese user interface.
