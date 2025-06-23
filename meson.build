project('clapi', ['cpp'],
        default_options:['cpp_std=c++26'])

cl_dep = dependency('OpenCL')

deps = [cl_dep]

message('Building in:' + get_option('cpp_std') + ' mode')

assert(get_option('cpp_std') in ['c++23', 'c++26'])

cxx = meson.get_compiler('cpp')

has_ranges_concat = cxx.compiles(
  '''
#include <generator>
#include <ranges>
#include <string>
#include <vector>

auto check_concat()
{
   std::vector<char> ABC {'A', 'B', 'C'};
   std::string XYZ;

   return ::std::ranges::views::concat(ABC, XYZ);
}

  ''',
  name: 'Check for std::ranges::views::concat_view')

cxxflags = []

if not has_ranges_concat
  deps += dependency('range-v3', required: true, method:'cmake')
  cxxflags += ['-D_clapi_MISSING_RANGES_CONCAT=1']
endif
srcs = [
  'cmd_line_parse.cc',
  'clapi.cc',
# static_asserts()
  'qa'/'deduced_asserts.cc',
  'qa'/'fun_ptr_asserts.cc'
]

clapi_private_inc = include_directories('private_include')
clapi_inc = include_directories('include')

clapi = executable('clapi',
                    srcs,
                    cpp_args: cxxflags,
                    include_directories: [clapi_private_inc, clapi_inc],
                    dependencies: deps)


