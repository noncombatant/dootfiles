set nocompatible
set backspace=indent,eol,start
set nobackup
set history=20
set ruler
set showcmd
set incsearch
set ignorecase
set smartcase
set hlsearch
set background=dark
set tabstop=2
set softtabstop=2
set expandtab
set shiftwidth=2
set textwidth=80
set nomodeline
set tags=tags
set number
set nojoinspaces

au FileType go setl tabstop=4 shiftwidth=4 softtabstop=4 noexpandtab textwidth=80

syntax on

setlocal spell spelllang=en_us
setlocal spell spellfile=~/.spell.add

map Q {gq}

fun! Manual()
  let s:word = expand('<cword>')
  :exe ":!tmux split-window -h 'man " . s:word . "'"
endfun
map K :call Manual()<CR>
