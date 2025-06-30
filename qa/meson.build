fs = import('fs')

if not get_option('enable-qa-hdrs-sanity')
  subdir_done()
endif

prog_find = find_program('find', required: true)
prog_git = find_program('git', required: true)

hdr_qa_srcs = []

foreach mod : ['fwd'/'clapi',
               'clapi'/'etc',
               'clapi'/'deduced',
               'clapi'/'transforms',
               'clapi']
  r = run_command(prog_find, [clapi_inc/mod,
                              '-iname', '*.hh',
                              '-maxdepth', '1',
                              '-type', 'f'],
                  check: true)
  paths = r.stdout().split()
  debug(f'All module @mod@ files: @paths@')
  foreach p : paths
    in_git = run_command(prog_git, 'ls-files',
                         '--error-unmatch', p,
                         check: false,
                         capture: false)
    if 0 == in_git.returncode()
      hdr_path = fs.relative_to(p, path)
      src_path = fs.replace_suffix(hdr_path, '.cc')
      debug(f'For header: @hdr_path@')
      chk_path = 'headers-work'/src_path
      debug(f' header include sanity @chk_path@ '
             + (fs.exists(chk_path)? 'exists' : 'is missing'))
      if not fs.exists(chk_path)
        error(f'Missing include sanity file: `@chk_path@` for `@hdr_path@`')
      endif
      hdr_qa_srcs += files(chk_path)
    else
      warning('ignoring untracked header `@p@`')
    endif
  endforeach
endforeach

static_library('qa-hdrs', hdr_qa_srcs,
               include_directories: clapi_inc)

