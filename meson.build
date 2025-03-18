
project(
  'Vamp Lossy Encoding Detector',
  'cpp',
  # For a Vamp plugin the version must be a single integer
  version: '1',
  license: 'MIT',
  default_options: [
    'cpp_std=c++17',
    'warning_level=2',
    'buildtype=release',
    'b_ndebug=if-release',
  ],
)

conf = configuration_data()
conf.set('version', meson.project_version())
configure_file(input: 'version.h.in', output: 'version.h', configuration: conf)

system = host_machine.system()

plugin_files = [
  'LossyDetector.cpp',
  'QuickLossyDetector.cpp',
  'detect.cpp',
  'weights.cpp',
  'plugins.cpp',
]

vamp_sdk_files = [
  'vamp-plugin-sdk/src/vamp-sdk/FFT.cpp',
  'vamp-plugin-sdk/src/vamp-sdk/PluginAdapter.cpp',
  'vamp-plugin-sdk/src/vamp-sdk/RealTime.cpp',
]

vamp_symbol_args = []

if system == 'linux'
  vamp_symbol_args += [
    '-Wl,--version-script=' + meson.current_source_dir() / 'vamp-plugin.map'
  ]
elif system == 'darwin'
  vamp_symbol_args += [
    '-exported_symbols_list', meson.current_source_dir() / 'vamp-plugin.list'
  ]
elif system == 'windows'
  vamp_symbol_args += [
    '-EXPORT:vampGetPluginDescriptor'
  ]
endif

plugin_library = shared_library(
  'vamp-lossy-encoding-detector',
  plugin_files,
  vamp_sdk_files,
  include_directories: [
    'vamp-plugin-sdk',
  ],
  link_args: [
    vamp_symbol_args,
  ],
  name_prefix: '',
  install: true,
)  

