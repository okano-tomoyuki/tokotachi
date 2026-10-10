# Tokotachi Designer

GUI designer for [Tokotachi](https://github.com/okano-tomoyuki/tokotachi), a cross-platform C++ GUI library.

- Open a window definition (`*.ttui.json`) to edit it in the designer. The canvas is drawn by Tokotachi itself
  (compiled to WebAssembly), so it looks the same as the built application.
- Generate the C++ class (`MainWindow : public tokotachi::Window`) with **Generate Code** or on save.
  Only the regions enclosed in markers are overwritten; your handler code is kept.
- Create a new window with **Tokotachi Designer: New Window...**.
