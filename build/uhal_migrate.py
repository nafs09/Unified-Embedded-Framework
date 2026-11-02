from pathlib import Path
import shutil

root = Path(__file__).resolve().parents[1]
assert root == Path(r'C:\NEXUS\Unified Embedded Framework')
moves = {}
for old in (root / 'UETD/src/uhal/x86').glob('*.c'):
    moves[old] = root / 'UETD/src/uhal/platform/host' / old.name
for old in (root / 'UETD/src/uhal/arm_cm').glob('*.c'):
    scope = 'arch/cortex_m' if old.stem in {'atomic', 'cache', 'core', 'fault', 'irq'} else 'platform/cortex_m_scaffold'
    moves[old] = root / 'UETD/src/uhal' / scope / old.name
for folder, scope in [('esp_idf', 'platform/esp_idf'), ('avr', 'arch/avr')]:
    for old in (root / 'UETD/src/uhal' / folder).glob('*'):
        moves[old] = root / 'UETD/src/uhal' / scope / old.name
replacements = {old.relative_to(root / 'UETD').as_posix(): new.relative_to(root / 'UETD').as_posix() for old, new in moves.items()}
for old, new in moves.items():
    assert old.is_relative_to(root) and new.is_relative_to(root)
    assert not new.exists(), new
    new.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(old, root / 'build/uhal-architecture' / ('before-' + old.parent.name + '-' + old.name))
    old.rename(new)
for folder in ['UETD/registry/modules', 'UETD/templates/modules', 'UETD/docs', 'UETD/include', 'uef-gen']:
    for path in (root / folder).rglob('*'):
        if path.suffix not in {'.json', '.md', '.tmpl', '.py', '.h'} or any(p in path.parts for p in ['bin', 'obj', '__pycache__']):
            continue
        content = path.read_text(encoding='utf-8')
        changed = content
        for old, new in replacements.items():
            changed = changed.replace(old, new)
        if changed != content:
            path.write_text(changed, encoding='utf-8', newline='\n')
for new in moves.values():
    if new.suffix == '.c':
        content = new.read_text(encoding='utf-8')
        for old_path, new_path in replacements.items():
            content = content.replace(old_path, new_path)
        new.write_text(content, encoding='utf-8', newline='\n')
print(f'Moved {len(moves)} UHAL files; updated source references.')
