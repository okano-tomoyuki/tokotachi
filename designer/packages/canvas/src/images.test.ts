/**
 * 大きい画像を参照するウィンドウを描く。描画の要求(画像を base64 にしたものを含む)が wasm のスタック(64 KB)より大きくても描ける。
 * WebAssembly のモジュールが無ければ(pnpm canvas:build の前)飛ばす。
 */
import { deflateSync } from 'node:zlib';
import { TtuiDocument } from '@tokotachi-designer/core';
import { describe, expect, it } from 'vitest';
import { Canvas, isBuilt } from './index.ts';

const CRC_TABLE = Array.from({ length: 256 }, (_, n) => {
  let c = n;
  for (let k = 0; k < 8; k++) {
    c = c & 1 ? (c >>> 1) ^ 0xedb88320 : c >>> 1;
  }
  return c >>> 0;
});

function crc32(data: Uint8Array): number {
  let c = ~0;
  for (const b of data) {
    c = (CRC_TABLE[(c ^ b) & 0xff] ?? 0) ^ (c >>> 8);
  }
  return ~c >>> 0;
}

function chunk(type: string, data: Buffer): Buffer {
  const body = Buffer.concat([Buffer.from(type, 'ascii'), data]);
  const head = Buffer.alloc(4);
  head.writeUInt32BE(data.length);
  const tail = Buffer.alloc(4);
  tail.writeUInt32BE(crc32(body));
  return Buffer.concat([head, body, tail]);
}

/** 圧縮しない RGBA の PNG(大きさ ≒ width × height × 4 バイト)。 */
function png(width: number, height: number): Buffer {
  const row = width * 4 + 1;
  const raw = Buffer.alloc(row * height);
  for (let i = 0; i < raw.length; i++) {
    raw[i] = i % row === 0 ? 0 : (i * 31) & 0xff;
  }
  const header = Buffer.alloc(13);
  header.writeUInt32BE(width, 0);
  header.writeUInt32BE(height, 4);
  header[8] = 8; // 8 ビット
  header[9] = 6; // RGBA
  return Buffer.concat([
    Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]),
    chunk('IHDR', header),
    chunk('IDAT', deflateSync(raw, { level: 0 })),
    chunk('IEND', Buffer.alloc(0)),
  ]);
}

describe.skipIf(!isBuilt())('画像', () => {
  it('1 MB の画像を ImageBox に描く', async () => {
    const data = png(512, 512);
    expect(data.length).toBeGreaterThan(1024 * 1024);
    const doc = TtuiDocument.parse({
      format_version: 1,
      window: {
        name: 'ImageWindow',
        class: 'Window',
        layout: {
          class: 'BoxLayout',
          items: [
            {
              widget: {
                name: 'image_box_photo',
                class: 'ImageBox',
                properties: { image: 'photo.png' },
              },
            },
          ],
        },
      },
    });
    const canvas = await Canvas.load({ language: 'en' });
    const frame = canvas.render(doc, { images: new Map([['photo.png', new Uint8Array(data)]]) });
    expect(frame.errors).toEqual([]);
    expect(frame.rects['image_box_photo']?.showing).toBe(true);
  });
});
