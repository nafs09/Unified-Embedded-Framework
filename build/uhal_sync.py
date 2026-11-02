from pathlib import Path
import importlib.util
import json
import hashlib
root = Path(__file__).resolve().parents[1]
def read(p): return json.loads((root / p).read_text(encoding='utf-8'))
def write(p, d): (root / p).write_text(json.dumps(d, indent=2, ensure_ascii=False) + '\n', encoding='utf-8', newline='\n')

grammar = read('uef-gen/src/uef_gen/language/registry/registry.json')
grammar['constructs']['hardware'] = {'lowering_rule': 'physical_target_profile', 'description': 'Optional literal physical assembly, compiler/SDK and programming settings; development boards are presets.',
    'syntax': 'hardware { build.backend = cmake; build.toolchain = "arm-none-eabi"; flash.backend = openocd; flash.toolchain = openocd; }', 'parse_rule': 'property_block', 'category': 'hardware', 'contract_source': 'UEF hardware project schema and UETD target/tool registries'}
grammar['grammar']['productions']['hardware_decl'] = "'hardware' property_block" if 'productions' in grammar.get('grammar', {}) else ''
write('uef-gen/src/uef_gen/language/registry/registry.json', grammar)

snippets = read('uef-gen/editors/vscode/snippets/uef.json')
snippets['UEF-PL project shell']['body'][0] = '// Select a chip or an optional development-board preset; custom wiring belongs in hardware.'
snippets['UEF-PL custom hardware profile'] = {'prefix': 'uef-hardware', 'body': [
    'hardware {', '  build.backend = ${1:cmake};', '  build.toolchain = "${2:arm-none-eabi}";',
    '  flash.backend = ${3:openocd};', '  flash.toolchain = "${4:openocd}";',
    '  flash.interface = "${5:swd}";', '  reserved_pins = [${6:"PA13", "PA14"}];', '}'],
    'description': 'Describe a bare chip or custom assembly without requiring a development-board record.'}
write('uef-gen/editors/vscode/snippets/uef.json', snippets)

# Update only UHAL's generated routes using the repository generator's functions.
path = root / 'UETD/tools/generate_module_templates.py'
spec = importlib.util.spec_from_file_location('uhal_template_sync', path)
tool = importlib.util.module_from_spec(spec); spec.loader.exec_module(tool)
index = read('UETD/templates/modules/index.json')
registry = read('UETD/registry/modules/registry.json')
for name in ['uhal/arm_cm', 'uhal/x86', 'uhal/avr', 'uhal/esp_idf']:
    manifest = registry['modules'][name]['manifest']
    record = read('UETD/' + manifest)
    sources, headers, dependencies = record['sources'], record['headers'], record['dependencies']
    relative = tool._output_relative_path(name, manifest)
    output = root / 'UETD/templates/modules' / relative
    content = tool._template_source(name, sources, headers, dependencies)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(content, encoding='utf-8', newline='\n')
    index['modules'][name] = {'template': 'templates/modules/' + relative.as_posix(), 'template_sha256': hashlib.sha256(content.encode()).hexdigest(),
        'detail_manifest': manifest, 'sources': sources, 'headers': headers, 'dependencies': dependencies,
        'future_scope': False, 'scaffold_only': bool(record.get('scaffold_only')), 'available_for_generation': record.get('available_for_generation'),
        'authoring_status': None, 'generation_status': None, 'missing_resources': []}
write('UETD/templates/modules/index.json', index)

# Refresh only declarations in headers changed by the UHAL API migration. Keep
# all other work-in-progress catalogue records byte-for-byte as JSON data.
spec = importlib.util.spec_from_file_location('uhal_type_sync', root / 'UETD/tools/generate_project_language_catalogues.py')
types_tool = importlib.util.module_from_spec(spec); spec.loader.exec_module(types_tool)
catalogue = read('UETD/registry/types/registry.json')
files = [*list((root / 'UETD/include/uhal').rglob('*.h')),
    *[p for p in (root / 'UETD/include/upal').rglob('*.h') if 'uhal_irq_t' in p.read_text(encoding='utf-8')]]
print('Template routes synchronized; type catalogue generator signature:', types_tool.collect_types.__annotations__)
print('UHAL/UPAL headers to refresh:', len(files))
previous = {(item['c_name'], item['header']): item for item in catalogue['types']}
fresh = types_tool.collect_types(files, previous)
covered = {p.relative_to(root / 'UETD').as_posix() for p in files}
catalogue['types'] = sorted([item for item in catalogue['types'] if item['header'] not in covered] + fresh, key=lambda item: (item['c_name'], item['header']))
write('UETD/registry/types/registry.json', catalogue)
api = read('UETD/api.json')
api['uhal_api']['contract_version'] = '2.0'
api['uhal_api']['composition'] = {'architecture_sources': 'src/uhal/arch', 'platform_sources': 'src/uhal/platform',
    'cmake_selector': 'cmake/Uhal.cmake', 'custom_hardware_config': 'target.hardware', 'development_board_required': False,
    'migration_guide': 'docs/design/UHAL_PORTING.md'}
write('UETD/api.json', api)
