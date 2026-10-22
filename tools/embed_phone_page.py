from pathlib import Path
try:
    Import('env')
    root = Path(env.subst('$PROJECT_DIR'))
except NameError:
    root = Path(__file__).resolve().parents[1]
html = (root / 'web/phone.html').read_text(encoding='utf-8')
assert ')PHONEPAGE"' not in html
content = '#pragma once\nstatic const char PHONE_PAGE[] PROGMEM = R"PHONEPAGE(' + html + ')PHONEPAGE";\n'
target = root / 'src/phone_page.h'
if not target.exists() or target.read_text(encoding='utf-8') != content:
    target.write_text(content, encoding='utf-8')
