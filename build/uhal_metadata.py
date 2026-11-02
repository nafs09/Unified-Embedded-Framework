from pathlib import Path
import json
root = Path(__file__).resolve().parents[1]

def read(name): return json.loads((root / name).read_text(encoding='utf-8'))
def write(name, data): (root / name).write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8', newline='\n')

registry = read('UETD/registry/modules/registry.json')
headers = read('UETD/registry/modules/manifest/uhal/arm_cm.json')['headers']
for variant, architecture, platform, reason in [
    ('avr', 'AVR', 'CUSTOM', 'AVR device interrupt maps, exact timer, GPIO/clock/reset and build integration are not implemented.'),
    ('esp_idf', 'XTENSA_OR_RISCV', 'ESP_IDF', 'ESP-IDF wrappers and SDK-owned UOS/build integration require a pinned SDK and verified SoC port.'),
]:
    manifest = f'UETD/registry/modules/manifest/uhal/{variant}.json'
    write(manifest, {'sources': [], 'dependencies': ['ucore'], 'headers': headers,
        'variant_of': 'uhal', 'target': variant.upper(), 'architecture': architecture, 'platform': platform,
        'scaffold_only': True, 'available_for_generation': False, 'contract_status': 'target_integration_incomplete', 'scaffold_reason': reason})
    registry['modules'][f'uhal/{variant}'] = {'manifest': manifest.removeprefix('UETD/'), 'DSL_descriptor': variant}
    registry['selection_groups']['uhal']['options'][variant.upper()] = f'uhal/{variant}'
registry['modules'] = dict(sorted(registry['modules'].items()))
write('UETD/registry/modules/registry.json', registry)
arm = read('UETD/registry/modules/manifest/uhal/arm_cm.json')
arm['architecture'] = 'CORTEX_M'
arm['sources'] = [source for source in arm['sources'] if '/arch/cortex_m/' in source]
if 'src/uhal/arch/cortex_m/cmsis.h' not in arm['headers']:
    arm['headers'].append('src/uhal/arch/cortex_m/cmsis.h')
arm['platform'] = 'CUSTOM'
arm['defines'] = ['UETD_TARGET_CORTEX_M=1', 'UETD_ARCH_CORTEX_M=1']
arm['scaffold_reason'] = 'CPU primitives are implemented, but an exact SoC platform for clock/GPIO/memory/time and startup/linking is required. A development board is optional.'
write('UETD/registry/modules/manifest/uhal/arm_cm.json', arm)
host = read('UETD/registry/modules/manifest/uhal/x86.json')
host['architecture'] = 'HOST'
host['platform'] = 'HOST'
host['defines'] = ['UHAL_HOST_SIMULATION=1', 'UETD_ARCH_HOST=1']
write('UETD/registry/modules/manifest/uhal/x86.json', host)

# Inline custom target settings use the same data shapes as optional board presets.
schema = read('uef-gen/src/uef_gen/schemas/project.schema.json')
board = read('UETD/registry/targets/development_boards/record.schema.json')
keys = ['clock_sources', 'memory_regions', 'reserved_pins', 'module', 'build', 'flash', 'debug_console']
properties = {key: board['properties'][key] for key in keys}
for key in ['build', 'flash']:
    properties[key]['required'] = ['backend', 'toolchain']
properties['build']['description'] = 'Target compiler/SDK adapter and pinned UETD toolchain; independent of board selection.'
properties['flash']['description'] = 'Target programming adapter, toolchain and transport settings; required by flashing, not compilation.'
schema['properties']['target']['properties']['hardware'] = {'type': 'object', 'properties': properties, 'additionalProperties': False,
    'description': 'Physical target and build/programming settings for a bare chip or custom assembly; optional board settings provide defaults.'}
schema['properties']['target']['properties']['toolchain']['description'] = 'Optional toolchain override; otherwise use resolved target.hardware.build.toolchain or its board-preset default.'
write('uef-gen/src/uef_gen/schemas/project.schema.json', schema)
print('Updated target schemas and UHAL module registry without enabling unimplemented targets.')
