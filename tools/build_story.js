'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { spawnSync } = require('node:child_process');

const root = path.resolve(__dirname, '..');
const sourceUrl = 'https://github.com/sjmelia/civd/blob/main/';

function sourceLines(text) {
  return text.replace(/\r\n/g, '\n').replace(/\n$/, '').split('\n');
}

// Each directive starts a section. The next directive supplies its end, so
// additions between anchors and at the end of a file are included automatically.
function expandChapter(markdown, readSource = file => fs.readFileSync(path.join(root, file), 'utf8')) {
  const pattern = /^\{\{#include (src\/[A-Za-z0-9_.]+)(?: \| (".*"))?\}\}$/gm;
  const sections = [...markdown.matchAll(pattern)];
  if (!sections.length) {
    if (markdown.includes('{{#include')) throw new Error('Malformed source include');
    return markdown;
  }
  const file = sections[0][1];
  const lines = sourceLines(readSource(file));
  const starts = sections.map((section, index) => {
    if (section[1] !== file) throw new Error('A chapter must include one complete source file');
    if (!section[2]) {
      if (index !== 0) throw new Error('Only the first source section can omit its anchor');
      return 0;
    }
    if (index === 0) throw new Error('The first source section must start at the beginning of the file');
    const anchor = JSON.parse(section[2]);
    const matches = lines.flatMap((line, i) => line.trim() === anchor ? [i] : []);
    if (matches.length !== 1) throw new Error(`${file}: expected one occurrence of ${section[2]}, found ${matches.length}`);
    return matches[0];
  });
  for (let i = 1; i < starts.length; i++) {
    if (starts[i] <= starts[i - 1]) throw new Error(`${file}: source anchors must appear in file order`);
  }
  let index = 0;
  const result = markdown.replace(pattern, () => {
    const start = starts[index];
    const end = starts[index + 1] ?? lines.length;
    index++;
    const link = `[Lines ${start + 1}–${end}](${sourceUrl}${file}#L${start + 1}-L${end}).`;
    const code = lines.slice(start, end).map(line => line ? `    ${line}` : '').join('\n');
    return `${link}\n\n${code}\n`;
  });
  if (result.includes('{{#include')) throw new Error('Malformed source include');
  return result;
}

function build() {
  const story = path.join(root, 'story');
  const chapters = fs.readdirSync(story).filter(file => /^\d{2}-.*\.md$/.test(file)).sort();
  if (!chapters.includes('00-opening.c.md')) throw new Error('Missing opening chapter');
  // Validate all anchors before replacing generated output.
  const expanded = chapters.map(file => {
    const markdown = fs.readFileSync(path.join(story, file), 'utf8');
    if (file !== '00-opening.c.md' && !markdown.includes('{{#include ')) {
      throw new Error(`${file}: missing source includes`);
    }
    return expandChapter(markdown);
  });
  const inputs = path.join(root, 'build', 'story-source');
  const output = path.join(root, 'build', 'story');
  fs.rmSync(inputs, { recursive: true, force: true });
  fs.mkdirSync(inputs, { recursive: true });
  chapters.forEach((file, i) => fs.writeFileSync(path.join(inputs, file), expanded[i]));
  fs.rmSync(output, { recursive: true, force: true });
  const docco = path.join(root, 'node_modules', 'docco', 'bin', 'docco');
  const result = spawnSync(process.execPath, [docco, '--languages', path.join(story, 'languages.json'),
    '--layout', 'parallel', '--output', output, ...chapters], { cwd: inputs, stdio: 'inherit' });
  if (result.error) throw result.error;
  if (result.status !== 0) throw new Error(`Docco failed with status ${result.status}`);
  fs.copyFileSync(path.join(output, '00-opening.c.html'), path.join(output, 'index.html'));
}

module.exports = { expandChapter };
if (require.main === module) build();
