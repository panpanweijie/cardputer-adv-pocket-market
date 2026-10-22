const fs=require('fs'),vm=require('vm'),assert=require('assert');
class El{constructor(){this.children=[];this.dataset={};this.value='';this.classList={toggle(){},add(){}}}append(...nodes){for(const n of nodes){if(n.parentElement)n.parentElement.children=n.parentElement.children.filter(x=>x!==n);n.parentElement=this;this.children.push(n)}}replaceChildren(){this.children=[]}setAttribute(){}querySelector(){return this.children[0]}focus(){}setPointerCapture(){}hasPointerCapture(){return false}getBoundingClientRect(){return {top:600}}}
const els={},document={getElementById:id=>els[id]??=new El(),createElement:()=>new El(),querySelectorAll:()=>[],querySelector:()=>new El(),elementFromPoint:()=>null};
let posted='',fail=false,release;const ctx=vm.createContext({document,window:{scrollBy(){}},requestAnimationFrame:()=>1,cancelAnimationFrame(){},URLSearchParams,fetch:async(url,opts={})=>{if(opts.method==='POST'){posted=opts.body.get('symbols');await new Promise(r=>release=r);return {ok:!fail,json:async()=>fail?{error:'测试失败'}:{count:3}}}return {ok:true,json:async()=>({csrf:'test',symbols:'usAAPL,hk00700,sh600519',names:{usAAPL:'Apple',hk00700:'腾讯控股',usZZTEST:'自定义名称 <示例>'}})}}});
const html=fs.readFileSync(__dirname+'/../web/phone.html','utf8');vm.runInContext(html.match(/<script>([\s\S]*?)<\/script>/)[1],ctx);
(async()=>{await new Promise(setImmediate);assert.equal(vm.runInContext('loaded',ctx),true);
assert.equal(els.selected.children[0].children[1].textContent,'usAAPL · 苹果');
assert.equal(els.selected.children[1].children[1].textContent,'hk00700 · 腾讯');
assert.equal(vm.runInContext("stockLabel('usZZTEST')",ctx),'usZZTEST · 自定义名称 <示例>');
assert.equal(vm.runInContext("stockLabel('usUNKNOWN')",ctx),'usUNKNOWN');
vm.runInContext("moveCode('sh600519','usAAPL');syncOrder()",ctx);assert.deepEqual(els.selected.children.map(c=>c.dataset.code),['sh600519','usAAPL','hk00700']);
// Keyboard reorder uses same real array and DOM.
els.selected.children[0].children[0].onkeydown({key:'ArrowDown',preventDefault(){}});assert.deepEqual(els.selected.children.map(c=>c.dataset.code),['usAAPL','sh600519','hk00700']);
// Cancellation restores the original order.
vm.runInContext("drag={original:[...selected],handle:document.createElement('button'),id:1,frame:1};moveCode('hk00700','usAAPL');finishDrag(true)",ctx);assert.deepEqual(els.selected.children.map(c=>c.dataset.code),['usAAPL','sh600519','hk00700']);
let saving=els.save.onclick();await new Promise(setImmediate);assert.equal(posted,'usAAPL,sh600519,hk00700');assert.match(els.savefeedback.textContent,/正在保存/);assert.equal(els.save.textContent,'保存中…');release();await saving;assert.match(els.savefeedback.textContent,/已保存 3/);assert.equal(els.status.textContent,els.savefeedback.textContent);
fail=true;saving=els.save.onclick();await new Promise(setImmediate);release();await saving;assert.match(els.savefeedback.textContent,/测试失败/);assert.equal(els.save.textContent,'保存到设备');assert.match(html,/touch-action:none/);assert.match(html,/id="savefeedback" role="status"/);console.log('PASS actual phone page: reorder, keyboard, cancel, submitted order, saving/success/failure feedback');})();
