set background=dark
hi clear
if exists("syntax_on")
  syntax reset
endif
let g:colors_name = "chris"

hi VertSplit term=none ctermfg=gray ctermbg=gray
hi Comment term=none ctermfg=darkgray guifg=gray
hi Normal guifg=black guibg=white
hi Constant term=none ctermfg=darkMagenta guifg=darkMagenta
hi Special term=bold ctermfg=Magenta guifg=Magenta
hi Identifier term=none ctermfg=cyan guifg=Blue
hi Statement term=bold ctermfg=darkyellow gui=NONE guifg=Brown
hi PreProc term=none ctermfg=darkcyan guifg=darkcyan
hi Type term=none ctermfg=darkgreen gui=NONE guifg=darkgreen
hi Visual term=none ctermfg=black ctermbg=gray gui=NONE guifg=Black guibg=gray
hi Search term=reverse ctermfg=Black ctermbg=darkyellow gui=NONE guifg=Black guibg=darkyellow
hi Tag term=bold ctermfg=DarkGreen guifg=DarkGreen
hi Error term=reverse ctermfg=15 ctermbg=9 guibg=Red guifg=White
hi Todo term=reverse ctermbg=Yellow ctermfg=Black guifg=Blue guibg=Yellow
hi StatusLine term=bold cterm=NONE ctermfg=gray ctermbg=darkyellow gui=NONE guifg=darkred guibg=gray
hi StatusLineNC term=bold cterm=NONE ctermfg=darkyellow ctermbg=darkgray gui=NONE guifg=darkred guibg=gray
hi LineNr ctermfg=darkgray
hi! link MoreMsg Comment
hi! link ErrorMsg Visual
hi! link WarningMsg ErrorMsg
hi! link Question Comment
hi link String	Constant
hi link Character	Constant
hi link Number	Constant
hi link Boolean	Constant
hi link Float		Number
hi link Function	Identifier
hi link Conditional	Statement
hi link Repeat	Statement
hi link Label		Statement
hi link Operator	Statement
hi link Keyword	Statement
hi link Exception	Statement
hi link Include	PreProc
hi link Define	PreProc
hi link Macro		PreProc
hi link PreCondit	PreProc
hi link StorageClass	Type
hi link Structure	Type
hi link Typedef	Type
hi link SpecialChar	Special
hi link Delimiter	Special
hi link SpecialComment Special
hi link Debug		Special
