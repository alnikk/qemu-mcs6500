autocmd FileType c call ConsiderMesonForLinting()
function ConsiderMesonForLinting()
    if filereadable('meson.build')
        let g:syntastic_c_checkers = ['meson']
    endif
endfunction
