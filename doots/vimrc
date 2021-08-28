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
highlight StatusLine ctermbg=black ctermfg=yellow
highlight StatusLineNC ctermbg=yellow ctermfg=black

syntax on

autocmd BufReadPost *.content set syntax=html
autocmd BufReadPost *.go,*.js set textwidth=0
autocmd BufReadPost *.go set noexpandtab
autocmd BufReadPost *.rs set tabstop=4
autocmd BufReadPost *.rs set softtabstop=4
autocmd BufReadPost *.rs set shiftwidth=4

setlocal spell spelllang=
setlocal spell spellfile=~/.spell.add

" Keyboard shortcuts:
map Q {gq}
map W :execute ":!wc < " . expand("%:p")<CR>
":execute ":!tmux split-window -h 'man " . expand('<cword>') . "'"
autocmd FileType c map M :execute ":!man -S 2:3 " . expand('<cword>')<CR>
autocmd FileType sh map M :execute ":!man -S 1 " . expand('<cword>')<CR>
autocmd FileType go map M :execute ":!go doc " . expand('<cword>')<CR>
