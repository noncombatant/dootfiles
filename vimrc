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
set background=light
set tabstop=2
set softtabstop=2
set expandtab
set shiftwidth=2
set textwidth=80
set nomodeline
set tags=tags
set nonumber
set nojoinspaces
set laststatus=2
set statusline=%F
set statusline+=\ \ %l/%L:%c\ \ %P\ \ %m
highlight StatusLine ctermbg=white ctermfg=blue

" this looks unsafe!
"autocmd BufReadPost,FileReadPost,BufNewFile * call system("tmux rename-window " . expand("%"))

syntax on

autocmd BufReadPost *.content set syntax=html
autocmd BufReadPost *.go set textwidth=0
autocmd BufReadPost *.go set noexpandtab

setlocal spell spelllang=
setlocal spell spellfile=~/.spell.add

map Q {gq}

function! Manual()
  ":exe ":!tmux split-window -h 'man " . expand('<cword>') . "'"
  :execute ":!man " . expand('<cword>')
endfun
map M :call Manual()<CR>

function! System()
  :execute ":!" . expand('<cword>')
endfun
