set-default-mode fill
set-default-mode indent
#set-default-mode notab
set-fill-column 76
set-prefix-string "> "

#global-set-key "\^w\^w" other-window
global-set-key "]" blink-and-insert

auto-execute .+\.[ch]$ set-prefix-string //
