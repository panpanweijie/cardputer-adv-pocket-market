"""Generate a small curated security-name index; constant data stays in Flash."""
from pathlib import Path
import json,re
root=Path(__file__).resolve().parents[1];rows=[]
for line in (root/'data/stock_search_aliases.tsv').read_text().splitlines():
 if not line.strip() or line.startswith('#'):continue
 code,name,pinyin,initials,extra=line.split('|');assert re.fullmatch(r'(us|hk|sh|sz)[A-Z0-9.^-]+',code)
 rows.append([code,name,pinyin,initials,extra])
assert len({r[0] for r in rows})==len(rows)
text='#pragma once\n// Generated curated securities, not a full-market directory.\nstruct StockAlias{const char*code;const char*name;const char*pinyin;const char*initials;const char*aliases;};\nstatic const StockAlias STOCK_ALIASES[]={\n'
text+=''.join('{'+','.join(json.dumps(v,ensure_ascii=False) for v in row)+'},\n' for row in rows)+'};\n'
(root/'src/stock_aliases.h').write_text(text);print(len(rows),'stock aliases generated')
