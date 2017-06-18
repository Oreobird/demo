"mkdir -p ~/.vim/bundle/ && cd ~/.vim/bundle/ && git clone https://github.com/gmarik/vundle.git
set rtp+=~/.vim/bundle/vundle
call vundle#rc()

Bundle 'gmarik/vundle'

"original repos on github
Bundle 'altercation/vim-colors-solarized'

Bundle 'taglist.vim'
let Tlist_Ctags_Cmd='ctags'
let Tlist_Show_One_File=1
let Tlist_WinWidt=28
let Tlist_Exit_OnlyWindow=1
let Tlist_Use_Left_Window=1
map <F2> :TlistToggle<CR>

Bundle 'The-NERD-tree'
let NERDTreeWinPos='right'
let NERDTreeWinSize=30
map <F3> :NERDTreeToggle<CR>

Bundle 'a.vim'

"non githug repos
"Bundle 'git://***.git'

"Brief help of vundle
":BundleList
":BundleInstall
":BundleSearch
":BundleClean
":help vundle
"End brief help

set nocompatible
filetype on
set ruler
set number
set showcmd
set showmode
set scrolloff=3

set noerrorbells

syntax on
colorscheme desert
set background=dark
set cursorline
set backspace=2
set whichwrap+=<,>,h,l

set expandtab
set smarttab
set shiftwidth=4
set tabstop=4
set autoindent
set cindent

set nobackup
set noswapfile
set autoread
set autowrite
set autochdir
set fileencodings=utf-8
set fileformats=unix,dos,mac
filetype plugin on
filetype indent on
set showmatch
set matchtime=2
set hlsearch
set incsearch
set ignorecase
set smartcase
set magic
set lazyredraw
set nowrapscan

