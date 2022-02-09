set shiftwidth=2
set tabstop=2

set makeprg=ninja\ -C\ build\ -v
set wildignore+=*.o,*.a,build/*

set efm=%f:%l:%c:\ %tarning:\ %m,%f:%l:%c:\ %trror:\ %m,%Dninja:\ Entering\ directory\ `%f',%f:%l:%c:\ fatal\ %trror:\ %m,%f:%l.%c-%*[0-9]:\ %tarning:\ %m,%f:%l:%c:\ \ \ required\ from\ here

let g:rg_args='-g "*.h" -g "*.c" -g "*.cmake" -g "CMakeLists.txt" -g "!build/*" -g "*.md"'
