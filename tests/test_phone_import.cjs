const assert=require("node:assert/strict");
const fs=require('fs'),vm=require('vm');
const html=fs.readFileSync(__dirname+'/../web/phone.html','utf8');
const parsers=html.slice(html.indexOf('function canonicalUsTicker'),html.indexOf('function render(){'));
// Only parse helper declarations; skip unrelated pointer handlers between parseInput and render.
const begin=parsers.indexOf('let drag=');
vm.runInThisContext(begin>=0?parsers.slice(0,begin):parsers);

assert.deepEqual(parseInput('AAPL NVDA TSLA','us'),['usAAPL','usNVDA','usTSLA']);
assert.deepEqual(parseInput('SHOP USB shop usb','us'),['usSHOP','usUSB']);
assert.deepEqual(parseInput('usBMNR hk00700 sh600519 sz159934','us'),['usBMNR','hk00700','sh600519','sz159934']);
assert.deepEqual(parseInput('AAPL.US 0700.HK NASDAQ:NVDA','us'),['usAAPL','hk00700','usNVDA']);
assert.deepEqual(parseInput('700 9988','hk'),['hk00700','hk09988']);
assert.deepEqual(parseInput('600519 300750 159934','cn'),['sh600519','sz300750','sz159934']);
assert.deepEqual(parseInput('AAPL，NVDA;AAPL\nMSFT','us'),['usAAPL','usNVDA','usMSFT']);
assert.throws(()=>parseInput('AAPL&x=1','us'));console.log('PASS: mobile import for US/HK/CN, mixed codes, suffixes, separators, deduplication, SHOP/USB prefix ambiguity');

assert.deepEqual(parseInput("BMNU.AM usBMNU.AM BMNU SPY.AM usBRK.B.N BRK.B BRK.A", "us"),["usBMNU","usSPY","usBRK.B","usBRK.A"]);
for(const suffix of ['AM','OQ','N','O','PS','PK'])assert.equal(normalizeCode('BMNU.'+suffix,'us'),'usBMNU');
assert.equal(normalizeCode('BRK.B','us'),'usBRK.B');assert.equal(normalizeCode('BRK.A','us'),'usBRK.A');
