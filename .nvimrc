autocmd FileType c call ConsiderMesonForLinting()
function ConsiderMesonForLinting()
    if filereadable('meson.build')
        let g:syntastic_c_checkers = ['meson']
    endif
endfunction
autocmd FileType c call ConsiderMesonForLinting()

"let g:run_cmd = "/sources/perso/qemu-mcs6500/build/mcs6500-softmmu/qemu-system-mcs6500"
"let g:run_default_option = "-machine nes -bios /sources/perso/test.bin"
"function Run()
"    call inputsave()
"    let args = input('Enter args: ')
"    call inputrestore()
"    10split running
"    normal! ggdGbd
"    call termopen(g:run_cmd . " " . g:run_default_option . " " . args)
"endfunction
"
"let g:previous_window = -1
"function SmartInsert()
"  if &buftype == 'terminal'
"    if g:previous_window != winnr()
"      startinsert
"    endif
"    let g:previous_window = winnr()
"  else
"    let g:previous_window = -1
"  endif
"endfunction
"au BufEnter * call SmartInsert()
"
"nnoremap <F5> :call Run()<CR>
