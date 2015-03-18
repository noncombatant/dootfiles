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

map Q {gq}

fun! ReadMan()
  let s:word = expand('<cword>')
  " Open a new window:
  :exe ":wincmd n"
  " Read in the manual page:
  :exe ":r!man " . s:word . " | col -b"
  " Goto first line:
  :exe ":goto"
endfun
map K :call ReadMan()<CR>
