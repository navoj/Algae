#!/usr/bin/env python3
"""
Convert K&R (old-style) C function definitions to ANSI C style.

Pattern:
    return_type func_name (param1, param2, ...)
         type1 param1;
         type2 param2;
    {

Becomes:
    return_type func_name (type1 param1, type2 param2, ...)
    {

Also converts functions with no params:  func() { -> func(void) {
"""

import re
import sys
import os

# Regex for a K&R function definition header
# Matches: [qualifiers] return_type func_name ( param_list )
# Followed by parameter declarations and then {

def convert_file(filepath):
    with open(filepath, 'r') as f:
        lines = f.readlines()

    result = []
    i = 0
    changes = 0

    while i < len(lines):
        # Try to detect K&R function definition
        converted = try_convert_knr(lines, i)
        if converted:
            new_lines, consumed = converted
            result.extend(new_lines)
            i += consumed
            changes += 1
        else:
            result.append(lines[i])
            i += 1

    if changes > 0:
        with open(filepath, 'w') as f:
            f.writelines(result)
        print(f"  {filepath}: {changes} conversions")

    return changes


def try_convert_knr(lines, start):
    """
    Try to detect and convert a K&R function definition starting at line `start`.
    Returns (new_lines, num_consumed) or None.
    """
    # We need to find a pattern like:
    #   [return_type] func_name (param_names)
    #        type1 param_name1;
    #        type2 param_name2;
    #   {
    #
    # The function header might span multiple lines.
    # The return type might be on the preceding line.

    # First, try to find the function header with parenthesized param list
    # Look for lines ending with ) possibly followed by whitespace

    # Collect potential header lines (might span 1-3 lines)
    header_text = ""
    header_lines = 0

    # Look ahead up to 5 lines for the closing )
    for j in range(start, min(start + 5, len(lines))):
        header_text += lines[j]
        header_lines += 1
        if ')' in lines[j]:
            break
    else:
        return None

    # Strip comments from header for analysis (simple -- doesn't handle all cases)
    header_clean = re.sub(r'/\*.*?\*/', '', header_text, flags=re.DOTALL)

    # Check if this looks like a function definition header
    # Pattern: optional_qualifiers type name ( params )
    # Must NOT be: if, while, for, switch, return, #define, typedef, etc.
    header_stripped = header_clean.strip()

    # Skip preprocessor directives
    if header_stripped.startswith('#'):
        return None

    # Skip control structures
    control_keywords = ['if', 'while', 'for', 'switch', 'return', 'sizeof',
                        'typedef', 'else', 'do', 'case', 'goto']
    first_word_parts = header_stripped.split('(')[0].strip().split() if '(' in header_stripped else []
    first_word = first_word_parts[-1] if first_word_parts else ''
    if first_word in control_keywords:
        return None

    # Must have balanced parentheses with a simple param list
    m = re.match(
        r'^(.*?)\(\s*(.*?)\s*\)\s*$',
        header_clean.strip(),
        re.DOTALL
    )
    if not m:
        return None

    func_prefix = m.group(1).strip()
    param_names_str = m.group(2).strip()

    # func_prefix should look like "[qualifiers] [return_type] func_name"
    # It should NOT contain { } ; = (would indicate this is not a function def)
    if any(c in func_prefix for c in '{}=;'):
        return None

    # Skip if it looks like a function call (no type before the name)
    # The prefix must have at least a function name, and ideally a return type
    prefix_parts = func_prefix.split()
    if len(prefix_parts) == 0:
        return None

    # Check that the func name is a valid identifier
    func_name = prefix_parts[-1].lstrip('*')
    if not re.match(r'^[a-zA-Z_]\w*$', func_name):
        return None

    # Skip certain patterns that aren't function defs
    if func_name in ['defined', 'sizeof']:
        return None

    # Now look at what follows the header: parameter declarations and then {
    param_end = start + header_lines
    param_decls = []
    brace_line = None

    # Handle empty parameter list: func() -> func(void)
    if param_names_str == '' or param_names_str == 'void':
        # Check if next non-blank line starts with {
        j = param_end
        while j < len(lines) and lines[j].strip() == '':
            j += 1
        if j < len(lines) and lines[j].strip().startswith('{'):
            if param_names_str == '':
                # Convert func() to func(void)
                new_header = header_text.rstrip()
                # Replace the last () with (void)
                new_header = re.sub(r'\(\s*\)\s*$', '(void)', new_header.rstrip())
                new_lines = [new_header + '\n']
                # Add any blank lines between header and {
                for k in range(param_end, j):
                    new_lines.append(lines[k])
                new_lines.append(lines[j])
                return (new_lines, j - start + 1)
        return None

    # Parse parameter names
    param_names = [p.strip() for p in param_names_str.split(',')]
    if not all(re.match(r'^[a-zA-Z_]\w*$', p) for p in param_names):
        return None

    # Now collect parameter declarations
    # They should be lines like: int a; or int a, b; or char *p;
    # ending when we hit {
    j = param_end
    decl_lines = []

    while j < len(lines) and j < param_end + 30:
        line = lines[j].strip()
        if line == '' or line.startswith('/*') or line.startswith('//'):
            # skip blanks and comments, but keep them
            j += 1
            continue
        if line.startswith('{'):
            brace_line = j
            break
        if line.startswith('#'):
            # preprocessor directive in param decls - bail
            return None
        # Should be a parameter declaration ending with ;
        if line.endswith(';'):
            decl_lines.append(line)
            j += 1
        else:
            # Might be a multi-line declaration
            # Collect until ;
            multi = line
            j += 1
            while j < len(lines):
                next_line = lines[j].strip()
                multi += ' ' + next_line
                j += 1
                if next_line.endswith(';'):
                    break
            decl_lines.append(multi)
    else:
        return None

    if brace_line is None:
        return None

    if len(decl_lines) == 0:
        return None

    # Parse the declarations into a map: param_name -> full type string
    param_types = {}
    for decl in decl_lines:
        parsed = parse_param_decl(decl)
        if parsed is None:
            return None
        param_types.update(parsed)

    # Check that all parameter names have types
    for pname in param_names:
        if pname not in param_types:
            return None

    # Build the new ANSI function header
    ansi_params = ', '.join(param_types[p] for p in param_names)

    # Reconstruct the header
    # Preserve the original formatting style (indentation etc.)
    # Use the original prefix (qualifiers + return type + name)
    # The header might span multiple lines; flatten to one logical line
    func_prefix_original = ''
    for k in range(start, start + header_lines):
        func_prefix_original += lines[k]

    # Extract everything before the (
    paren_idx = func_prefix_original.index('(')
    before_paren = func_prefix_original[:paren_idx].rstrip()

    # Remove trailing newlines from before_paren, preserve leading indentation
    bp_lines = before_paren.split('\n')
    if len(bp_lines) > 1:
        # Multi-line return type + name
        new_header = '\n'.join(bp_lines) + ' (' + ansi_params + ')\n'
    else:
        new_header = before_paren + ' (' + ansi_params + ')\n'

    new_lines = [new_header]
    new_lines.append(lines[brace_line])

    return (new_lines, brace_line - start + 1)


def parse_param_decl(decl):
    """
    Parse a K&R parameter declaration like:
      int a;
      char *p;
      double *a, *b;
      int a, b;
      ENTITY *p;
      register int i;
      int (*fp)();
      va_list arg;
      FILE *stream;

    Returns dict: {param_name: "type param_name"} or None on failure.
    """
    # Remove trailing ;
    decl = decl.rstrip().rstrip(';').strip()

    if not decl:
        return None

    # Handle function pointer params: type (*name)(args)
    # Pattern: type (*name)(stuff)
    fp_match = re.match(r'^(.*?)\(\s*\*\s*(\w+)\s*\)\s*\((.*?)\)$', decl)
    if fp_match:
        base_type = fp_match.group(1).strip()
        name = fp_match.group(2)
        fp_args = fp_match.group(3)
        return {name: f"{base_type} (*{name})({fp_args})"}

    # Handle array params: type name[size] or type name[]
    arr_match = re.match(r'^(.*?)\s+(\w+)\s*(\[.*\])$', decl)
    if arr_match:
        base_type = arr_match.group(1).strip()
        name = arr_match.group(2)
        arr_part = arr_match.group(3)
        return {name: f"{base_type} {name}{arr_part}"}

    # Split into base type and declarators
    # Find the split point between type and names
    # The type is everything up to the last sequence of [*]name[, [*]name]...

    # Try to split on the last identifier(s)
    # Pattern: type_tokens name1, name2, *name3
    # We need to find where the type ends and the names begin

    parts = decl.split()
    if len(parts) < 2:
        return None

    # Strategy: The type is all tokens except the last comma-separated list of
    # identifiers (possibly with * prefixes)

    # Find the first token that could be a parameter name (not a type keyword)
    # by checking from the end
    type_keywords = {'int', 'char', 'short', 'long', 'float', 'double', 'void',
                     'signed', 'unsigned', 'const', 'volatile', 'register',
                     'static', 'extern', 'struct', 'union', 'enum'}

    # Rejoin and use regex to split type from declarator list
    # Pattern: base_type   declarator_list
    # where declarator_list is: [*]name [, [*]name]...
    m = re.match(r'^(.*?)\s+([\*]*\w+(?:\s*,\s*[\*]*\w+)*)$', decl)
    if not m:
        return None

    base_type = m.group(1).strip()
    declarators = m.group(2).strip()

    if not base_type:
        return None

    result = {}
    for d in declarators.split(','):
        d = d.strip()
        # Count leading *
        stars = ''
        while d.startswith('*'):
            stars += '*'
            d = d[1:].strip()
        name = d
        if not re.match(r'^[a-zA-Z_]\w*$', name):
            return None
        if stars:
            result[name] = f"{base_type} {stars}{name}"
        else:
            result[name] = f"{base_type} {name}"

    return result


def main():
    if len(sys.argv) < 2:
        print("Usage: knr2ansi.py file1.c [file2.c ...]")
        sys.exit(1)

    total = 0
    for filepath in sys.argv[1:]:
        if os.path.isfile(filepath):
            total += convert_file(filepath)
        else:
            print(f"  Skipping {filepath}: not a file")

    print(f"Total: {total} conversions")


if __name__ == '__main__':
    main()
