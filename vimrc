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
set statusline+=%F
set statusline+=\ \ %l/%L:%c\ \ %P\ \ %m
highlight StatusLine ctermbg=white ctermfg=blue

" this looks unsafe!
"autocmd BufReadPost,FileReadPost,BufNewFile * call system("tmux rename-window " . expand("%"))

syntax on

au BufReadPost *.content set syntax=html
au BufReadPost *.go set textwidth=0
au BufReadPost *.go set noexpandtab

setlocal spell spelllang=
setlocal spell spellfile=~/.spell.add

map Q {gq}

fun! Manual()
  let s:word = expand('<cword>')
"  :exe ":!tmux split-window -h 'man " . s:word . "'"
  :exe ":!man " . s:word
endfun

map M :call Manual()<CR>
