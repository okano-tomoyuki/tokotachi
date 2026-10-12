# Tokotachi Designer

[日本語](README.ja.md)

A visual GUI designer for [Tokotachi](https://github.com/okano-tomoyuki/tokotachi), a cross-platform C++11 GUI library.
Lay out windows by drag and drop, and generate readable C++ classes that you keep editing by hand.

![The designer](images/screenshots/designer.png)

## Features

- **What you see is what you build.** The canvas is drawn by Tokotachi itself, compiled to WebAssembly,
  so the window looks the same as your built application (fonts, layout and theme).
- **Lay out with layouts or freely.** Drop widgets from the palette into box and grid layouts, or place them freely
  with alignment tools. Select by clicking, by dragging a rubber band, or in the structure tree.
- **A type-aware inspector.** Edit properties with editors that fit their types (colors, fonts, theme overrides,
  dates, images, references to other widgets), connect handlers, and bind properties to variables.
- **Built-in icons.** Pick from about 70 common icons ([Lucide](https://lucide.dev/)) for buttons, menus and toolbars.
  They follow the text color of the widget, so they work in dark themes without extra images.

  ![Choosing an icon](images/screenshots/icons.png)

- **Menus, toolbars and actions** with shortcuts, shared between the menu bar and the toolbar.
- **Reuse.** Turn a group of widgets into a component file and place it in other windows.
  Your own widgets written in C++ appear in the palette once you register their headers in the project.
- **Generate C++ that stays yours.** Each window becomes a class (`MainWindow : public tokotachi::Window`).
  Only the regions enclosed in markers are regenerated; your handler code and includes are kept, and new handler stubs
  are added where they belong. Generate with **Generate Code** or on every save.

  ![Generated code](images/screenshots/codegen.png)

- **Projects.** A project file lists the windows and generates `main.cpp` and `CMakeLists.txt`, so a new application
  builds right away. Register shared themes and preview them.
- **Problems as you type.** Invalid values, broken references and missing files are shown in the Problems panel.
  Window definitions are plain JSON with a schema, so you can also edit them as text.

## Getting started

1. Open a folder, and run **Tokotachi Designer: New Project...** from the Command Palette (or **New Project** in the
   Tokotachi Designer view in the Activity Bar). Enter the project name; it is also the name of the CMake target.
2. The main window (`main_window.ttui.json`) opens in the designer. Drag widgets from the palette onto the canvas,
   and double-click a widget to write its main handler.
3. Save. The designer generates the window class, `main.cpp` and `CMakeLists.txt`.
4. Build with CMake. The generated `CMakeLists.txt` uses `find_package(tokotachi)`; see the
   [Tokotachi README](https://github.com/okano-tomoyuki/tokotachi#readme) for installing the library.

## Requirements

- The extension needs nothing else to design windows.
- To build the generated code: a C++11 compiler, CMake 3.16 or later, and the Tokotachi library.

## Settings

| Setting                                    | Default       | Description                                                            |
| ------------------------------------------ | ------------- | ---------------------------------------------------------------------- |
| `tokotachiDesigner.codegen.generateOnSave` | `true`        | Generate the C++ code when a window definition is saved.               |
| `tokotachiDesigner.codegen.commentLocale`  | `en`          | Language of the comments in the generated code (`en` or `ja`).         |
| `tokotachiDesigner.codegen.namespace`      | (none)        | Namespace of the generated classes (e.g. `app::ui`).                   |
| `tokotachiDesigner.codegen.headerDir`      | (same folder) | Folder of the generated headers, relative to the window definition.    |
| `tokotachiDesigner.codegen.sourceDir`      | (same folder) | Folder of the generated sources, relative to the window definition.    |
| `tokotachiDesigner.canvas.colorScheme`     | `auto`        | Color scheme of the canvas. `auto` follows the color theme of VS Code. |

Windows that belong to a project use the code generation settings of the project file instead.

## Commands

| Command                               | Description                                                  |
| ------------------------------------- | ------------------------------------------------------------ |
| New Project...                        | Create a project with a main window.                         |
| New Window... / New Component File... | Create a window definition or a reusable component.          |
| Generate Code / Generate Project      | Generate the C++ code of the window or of the whole project. |
| Open as Text                          | Open the window definition in the text editor.               |

## Language

The user interface is available in English and Japanese (it follows the display language of VS Code).

## License

[MIT](LICENSE). The extension includes third-party software (the Tokotachi library and its dependencies compiled to
WebAssembly, the [Lucide](https://lucide.dev/) icons, and JavaScript packages); see
[THIRD_PARTY_LICENSES.txt](THIRD_PARTY_LICENSES.txt).
