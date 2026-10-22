"""Generate the small, manually curated stock pinyin dictionary (no external corpus)."""
from pathlib import Path
import json,re
root=Path(__file__).resolve().parents[1]
rows=[];seen=set()
for line in (root/'data/stock_pinyin.tsv').read_text().splitlines():
    if not line.strip() or line.lstrip().startswith('#'):continue
    parts=line.split();key=parts[0]
    assert re.fullmatch('[a-z]+',key),key
    for word in parts[1:]:
        assert any('\u4e00'<=c<='\u9fff' for c in word),(key,word)
        if (key,word) not in seen:rows.append((key,word));seen.add((key,word))
text='#pragma once\n// Generated from data/stock_pinyin.tsv; constant data stays in Flash.\nstruct PinyinWord {const char* key;const char* text;};\nstatic const PinyinWord PINYIN_WORDS[]={\n'
text+=''.join('{'+json.dumps(k)+','+json.dumps(v,ensure_ascii=False)+'},\n' for k,v in rows)
text+='};\n'
(root/'src/pinyin_words.h').write_text(text)
print(f'{len(rows)} stock pinyin entries generated')
