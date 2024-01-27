import re

# Input file path
input_file = 'shared_structs.h'

# Output file path
output_file = 'sdata_py.cpp'

# Regular expression pattern to match struct definitions
struct_pattern = r'struct\s+(\w+)\s*\{([\s\S]+?)\};'

# Regular expression pattern to match field declarations inside the struct
field_pattern = r'\s*(\w+)\s+(\w+)\s*;'

# Initialize the generated Python script
python_script = '''
#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
'''

# Read the input .h file
with open(input_file, 'r') as file:
    file_contents = file.read()

# Find all struct definitions in the file
struct_matches = re.finditer(struct_pattern, file_contents)

# Function to generate Python code for a struct and its fields
def generate_struct_code(struct_name, struct_fields):
    code = f'    py::class_<{struct_name}>(m, "{struct_name}")\n'
    code += '        .def(py::init<>())\n'

    field_matches = re.finditer(field_pattern, struct_fields)
    
    for field_match in field_matches:
        if code[-1] != '\n':
            code += '\n'
        field_type = field_match.group(1)
        field_name = field_match.group(2)
        code += f'        .def_readwrite("{field_name}", &{struct_name}::{field_name})'

    code += ';\n'
    code += '\n'


    return code

# Iterate through struct definitions
for match in struct_matches:
    struct_name = match.group(1)
    struct_fields = match.group(2)

    # Generate Python code for the main struct
    python_script += generate_struct_code(struct_name, struct_fields)

# finish the struct definition
python_script += '};\n\n'

# Write the generated Python script to the output file
with open(output_file, 'w') as output:
    output.write(python_script)

print(f'Generated Python script saved to {output_file}')
