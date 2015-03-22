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
set tabstop=4
set softtabstop=4
set expandtab
set shiftwidth=4
set textwidth=76
set nomodeline
set tags=tags
set number
set nojoinspaces

syntax on

setlocal spell spelllang=en_us
setlocal spell spellfile=~/.spell.add

map Q {gq}

fun! Manual()
  let s:word = expand('<cword>')
  :exe ":!tmux split-window -h 'man " . s:word . "'"
endfun
map K :call Manual()<CR>
