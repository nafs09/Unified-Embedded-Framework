from pathlib import Path
import json
root = Path(__file__).resolve().parents[1]
p = root / 'UETD/registry/targets/chips/record.schema.json'
schema = json.loads(p.read_text(encoding='utf-8'))
resources = schema['$defs']['resources']['properties']
resources['memory_regions']['additionalProperties']['properties']['address_space'] = {
    'type': 'string', 'minLength': 1, 'description': 'Chip-owned physical address space; defaults to unified.'}
resources['uhal'] = {
    'type': 'object', 'required': ['default_platform', 'platforms'],
    'properties': {
        'default_platform': {'type': 'string', 'minLength': 1},
        'platforms': {'type': 'object', 'minProperties': 1, 'additionalProperties': {
            'type': 'object', 'required': ['architecture', 'module', 'build_backend'],
            'properties': {
                'architecture': {'enum': ['HOST', 'CORTEX_M', 'AVR', 'XTENSA', 'RISCV']},
                'module': {'type': 'string', 'pattern': '^uhal/[a-z0-9_/]+$'},
                'build_backend': {'type': 'string', 'minLength': 1},
                'framework': {'type': 'string', 'minLength': 1}},
            'additionalProperties': False}}},
    'additionalProperties': False}
p.write_text(json.dumps(schema, indent=2) + '\n', encoding='utf-8', newline='\n')
for p in (root / 'UETD/src/uhal').rglob('*.c.todo'):
    text = p.read_text(encoding='utf-8').replace('IRQn_Type', 'uhal_irq_t')
    for name in ['uhal_irq_enable', 'uhal_irq_disable', 'uhal_irq_set_pending', 'uhal_irq_clear_pending',
                 'uhal_cache_clean', 'uhal_cache_invalidate', 'uhal_cache_clean_invalidate']:
        text = text.replace('void ' + name + '(', 'uetd_status_t ' + name + '(')
    if '/esp_idf/' in p.as_posix() and p.name == 'fault.c.todo':
        text = '\n'.join(line for line in text.splitlines() if not any(handler in line for handler in ['HardFault_Handler(', 'MemManage_Handler(', 'BusFault_Handler(', 'UsageFault_Handler('])) + '\n'
    p.write_text(text, encoding='utf-8', newline='\n')
print('Added exact-chip UHAL bindings and address spaces to the target schema.')
