import gdb
import re

ns_pattern = re.compile(r'nlohmann(::json_abi(?P<tags>\w*)(_v(?P<v_major>\d+)_(?P<v_minor>\d+)_(?P<v_patch>\d+))?)?::(?P<name>.+)')
class JsonValuePrinter:
    "Print a json-value"

    def __init__(self, val, is_null=False):
        self.val = val
        self.is_null = is_null

    def to_string(self):
        if self.is_null:
            return "null"
        if self.val.type.strip_typedefs().code == gdb.TYPE_CODE_FLT:
            # 格式化为最多6位小数，但保证至少保留1位小数
            formatted = ("%.6f" % float(self.val))
            # 移除多余的0，但确保至少保留一个小数位
            if '.' in formatted:
                integer_part, decimal_part = formatted.split('.')
                decimal_part = decimal_part.rstrip('0')
                if not decimal_part:
                    decimal_part = '0'  # 至少保留一位小数
                return f"{integer_part}.{decimal_part}"
            return formatted
        return self.val

def json_lookup_function(val):
    if m := ns_pattern.fullmatch(str(val.type.strip_typedefs().name)):
        name = m.group('name')
        if name and name.startswith('basic_json<') and name.endswith('>'):
            m_data = val['m_data']
            m_type = m_data['m_type']
            m = ns_pattern.fullmatch(str(m_type))
            t = m.group('name')
            prefix = 'detail::value_t::'
            if t and t.startswith(prefix):
                # 检查是否为null类型
                if t == prefix + 'null':
                    return JsonValuePrinter(m_type, True)
                try:
                    union_val = m_data['m_value'][t.replace(prefix, '', 1)]
                    if union_val.type.code == gdb.TYPE_CODE_PTR:
                        return gdb.default_visualizer(union_val.dereference())
                    else:
                        return JsonValuePrinter(union_val)
                except Exception:
                    return JsonValuePrinter(m_type)

gdb.pretty_printers.append(json_lookup_function)