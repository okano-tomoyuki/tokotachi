// テストで見本のテキスト・画像を読み込む(Vite の ?raw・?inline の import。codegen は Node の fs を使わない)
declare module '*?raw' {
  const text: string;
  export default text;
}

/** data URL(`data:image/png;base64,...`)。 */
declare module '*?inline' {
  const url: string;
  export default url;
}
