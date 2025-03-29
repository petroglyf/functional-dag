project(
    'functional-dag',
    'cpp',
    default_options: [
        'c_std=c23',
        'cpp_std=c++23',
        'warning_level=3',
        'werror=true',
    ],
    version: '1.0rc1',
    license: 'MIT',
    meson_version : '>= 1.1',
    license_files: 'LICENSE',
)

if build_machine.subsystem() == 'macos'
    include_install_dir = 'include'
    shared_lib_dir = 'lib'
    pkg_config_install_dir = shared_lib_dir + '/pkgconfig'
else
    include_install_dir = '/usr/include'
    shared_lib_dir = '/usr/lib'
    pkg_config_install_dir = '/usr/share/pkgconfig'
endif

##########################################
####### Find the needed packages #########
##########################################
catch_dep = dependency('catch2-with-main', required : false)
flatbuffers_dep = dependency('flatbuffers')
pkg_fn = import('pkgconfig')
fs_dep = import('fs')

if fs_dep.is_dir('build/include') == false
    run_command(find_program('mkdir'), '-p', 'build/include', check: true)
endif

########################################
####### Building specification #########
########################################
libspec_gen_h = custom_target(
    'lib_spec_generate',
    output : 'lib_spec_generated.h',
    input : 'flatbufs/lib_spec.fbs',
    command : ['flatc', '--reflect-types', '-o', 'include/functional_dag/', '--cpp', '@INPUT@'],
)

libspec_bfbs = custom_target(
    'lib_spec_schema',
    output : 'lib_spec.bfbs',
    input : 'flatbufs/lib_spec.fbs',
    command : ['flatc', '--bfbs-gen-embed', '--schema', '-b', '@INPUT@'],
)

guid_gen_h = custom_target(
    'guid_generate',
    output : 'guid_generated.h',
    input : 'flatbufs/guid.fbs',
    command : ['flatc', '--reflect-types', '-o', 'include/functional_dag/', '--cpp', '@INPUT@'],
)

error_codes_h = custom_target(
    'error_codes_generate',
    output : 'error_codes.h',
    input : 'flatbufs/error_codes.fbs',
    command : ['flatc', '--reflect-types', '-o', 'include/functional_dag/', '--no-prefix', '--filename-suffix', '', '--cpp', '@INPUT@'],
)

generated_dep = declare_dependency (
    include_directories: ['include/'], 
    sources: [error_codes_h, libspec_gen_h, guid_gen_h], 
    dependencies: [flatbuffers_dep],
)

functional_dag_lib = shared_library(
    'functional_dag',
    ['src/functional_dag/libutils.cpp', libspec_gen_h, guid_gen_h, libspec_bfbs, error_codes_h],
    cpp_args: ['-DSCHEMA_FILE='+libspec_bfbs.full_path()],
    include_directories: ['include/'],
    dependencies: [flatbuffers_dep, generated_dep],
    install: true,
    install_dir: shared_lib_dir,
)

########################################
####### Build the testing files ########
########################################
if catch_dep.found()
  dag_tests = executable(
      'dag_tests',
      ['test/functional_dag/dag_tests.cpp', libspec_gen_h, guid_gen_h, error_codes_h],
      include_directories: ['include/'],
      dependencies: [catch_dep, flatbuffers_dep, generated_dep],
  )
  lib_tests = executable(
      'lib_tests',
      ['test/functional_dag/lib_tests.cpp', error_codes_h],
      include_directories: ['include/'],
      dependencies: [catch_dep, flatbuffers_dep, generated_dep],
      cpp_args: ['-DSCHEMA_FILE='+libspec_bfbs.full_path()+''],
      link_with: [functional_dag_lib],
  )

  guid_tests = executable(
      'guid_tests',
      ['test/functional_dag/guid_tests.cpp', error_codes_h],
      include_directories: ['include/'],
      dependencies: [catch_dep, flatbuffers_dep, generated_dep],
      link_with: [functional_dag_lib],
  )

  test('dag_tests', dag_tests)
  test('lib_tests', lib_tests)
  test('guid_tests', guid_tests)
endif

########################################
####### Lint command (optional) ########
########################################
clangtidy = find_program('clang-tidy', required: false)
if clangtidy.found()
    checks_str = '-clang-diagnostic-*,modernize-*,cppcoreguidelines-*,-readability-*,llvm-header-guard,misc-noexcept-move-constructor,misc-move-const-arg'
    run_target(
        'lint',
        command: [
            clangtidy,
            '-p',
            meson.project_build_root(),
        ] + ['../src/functional_dag/libutils.cpp', '-checks='+checks_str, '-warnings-as-errors=*'],
    )
endif

########################################
####### Install specification ##########
########################################
install_subdir('include/functional_dag', install_dir: include_install_dir)
install_subdir(meson.current_build_dir()+'/include/functional_dag', install_dir: include_install_dir)
pkg_fn.generate(
    functional_dag_lib,
    description: 'A directed acyclic graph that runs a function on each node.',
    install_dir: pkg_config_install_dir,
)

########################################
##### Packaging specifications #########
########################################
if build_machine.subsystem() != 'macos'
    arch_id = get_option('ARCH_ID')

    control_cp = fs_dep.copyfile(
        'debian/control',
        install: true,
        install_dir: '../../DEBIAN/',
    )
    deb_package_name = meson.project_name() + '-' + meson.project_version() + '.' + arch_id + '.deb'
    mytarget = custom_target(
        'debpackage',
        # input: 'DEBIAN/control',
        output: deb_package_name,
        command: ['dpkg-deb', '-b', 'install/', '@OUTPUT@'],
    )
endif
