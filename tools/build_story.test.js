'use strict';

const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const test = require('node:test');
const docco = require('docco');
const { expandChapter } = require('./build_story');

function codeSections(file, markdown) {
  const language = { name: 'c', symbol: '@docco', commentMatcher: /^\s*@docco\s?/, commentFilter: /^#!/ };
  return docco.parse(file, markdown, { languages: { '.c': language, '.mk': { ...language, name: 'makefile' } } })
    .map(section => section.codeText.replace(/^\n+|\n+$/g, '')).filter(Boolean);
}

test('inserts current source, preserves comments and tabs, and recalculates ranges', () => {
  const chapter = '# Example\n\n{{#include src/EXAMPLE.C}}\n\nSecond part.\n\n{{#include src/EXAMPLE.C | "void run(void)"}}\n';
  const source = '// Original comment\r\nint added;\r\n\r\nvoid run(void)\r\n{\r\n\treturn;\r\n}\r\n// New trailing comment\r\n';
  const expanded = expandChapter(chapter, () => source);
  assert.match(expanded, /blob\/main\/src\/EXAMPLE.C#L1-L3/);
  assert.match(expanded, /#L4-L8/);
  assert.deepEqual(codeSections('example.c.md', expanded), [
    '// Original comment\nint added;', 'void run(void)\n{\n\treturn;\n}\n// New trailing comment'
  ]);
});

test('rejects missing, ambiguous, reversed and incomplete source anchors', () => {
  const beginning = '{{#include src/EXAMPLE.C}}\n\n';
  const anchor = text => `{{#include src/EXAMPLE.C | ${JSON.stringify(text)}}}\n\n`;
  assert.throws(() => expandChapter(beginning + anchor('missing'), () => 'int x;\n'), /found 0/);
  assert.throws(() => expandChapter(beginning + anchor('int x;'), () => 'int x;\nint x;\n'), /found 2/);
  assert.throws(() => expandChapter(beginning + anchor('int z;') + anchor('int y;'), () => 'int x;\nint y;\nint z;\n'), /file order/);
  assert.throws(() => expandChapter(anchor('int y;'), () => 'int x;\nint y;\n'), /beginning/);
  assert.throws(() => expandChapter(beginning + '{{#include src/OTHER.C}}', () => 'int x;\n'), /one complete/);
  assert.throws(() => expandChapter('{{#include invalid}}'), /Malformed/);
});

test('all story chapters render complete current files in source order', () => {
  const root = path.resolve(__dirname, '..');
  const chapters = fs.readdirSync(path.join(root, 'story')).filter(file => /^\d{2}-.*\.md$/.test(file));
  for (const file of chapters) {
    const expanded = expandChapter(fs.readFileSync(path.join(root, 'story', file), 'utf8'));
    const ranges = [...expanded.matchAll(/\[Lines (\d+)–(\d+)\]\(https:\/\/github.com\/sjmelia\/civd\/blob\/main\/(src\/[^#]+)#L\d+-L\d+\)/g)];
    if (file === '00-opening.c.md') continue;
    assert.ok(ranges.length, file);
    const source = fs.readFileSync(path.join(root, ranges[0][3]), 'utf8').replace(/\r\n/g, '\n').replace(/\n$/, '').split('\n');
    const code = codeSections(file, expanded);
    assert.equal(code.length, ranges.length, file);
    let previousEnd = 0;
    ranges.forEach((range, i) => {
      const start = Number(range[1]), end = Number(range[2]);
      assert.equal(start, previousEnd + 1, file);
      assert.equal(code[i], source.slice(start - 1, end).join('\n').replace(/^\n+|\n+$/g, ''), file);
      previousEnd = end;
    });
    assert.equal(previousEnd, source.length, file);
    assert.doesNotMatch(expanded, /github\.com\/sjmelia\/civd\/blob\/(?!main\/)/);
  }
});
