[![Build Status](https://circleci.com/gh/equalsraf/neovim-qt.svg?style=svg)](https://circleci.com/gh/equalsraf/neovim-qt)
[![Build status](https://ci.appveyor.com/api/projects/status/c252f54mfjcuud8x/branch/master?svg=true)](https://ci.appveyor.com/project/equalsraf/neovim-qt/branch/master)
[![Build Status](https://travis-ci.org/equalsraf/neovim-qt.svg?branch=master)](https://travis-ci.org/equalsraf/neovim-qt)
[![codecov Status](https://codecov.io/gh/equalsraf/neovim-qt/branch/master/graph/badge.svg)](https://codecov.io/gh/equalsraf/neovim-qt)
[![Build Status](https://dev.azure.com/equalsraf/neovim-qt/_apis/build/status/equalsraf.neovim-qt?branchName=master)](https://dev.azure.com/equalsraf/neovim-qt/_build/latest?definitionId=1&branchName=master)
[![Downloads](https://img.shields.io/github/downloads/equalsraf/neovim-qt/total.svg?maxAge=2592000)](https://github.com/equalsraf/neovim-qt/releases)

# Neovim Qt
Neovim Qt is a basic GUI for Neovim written in Qt and C++. It was designed to provide a fast, consistent experience across all platforms.

![NeovimQt Screenshot](https://user-images.githubusercontent.com/11207308/108295028-f79f1b80-7164-11eb-8420-e9950fa97cd0.png)


## Installing Neovim Qt
Neovim Qt is released to all major platforms. 

### Windows
A Neovim Qt is bundled with all releases of Neovim on Windows.

See the Neovim release page:

 - [Nighly Release](https://github.com/neovim/neovim/releases/nightly)
 - [Stable Release](https://github.com/neovim/neovim/releases/latest)

Alternatively, you can use a package manager such as Chocolatey:

See [Neovim Chocolatey Package](https://chocolatey.org/packages/neovim/):
```
choco install neovim
```

### MacOS

You can use HomeBrew to install Neovim Qt:
```
$ brew tap equalsraf/neovim-qt
$ brew install neovim-qt
```

See: https://github.com/equalsraf/homebrew-neovim-qt

### Linux

Use your favorite package manager. If your favorite distro is not listed, please add it!

#### ArchLinux
NeovimQt is available from this [AUR Package](https://archlinux.org/packages/community/x86_64/neovim-qt/).

#### Gentoo
You can download/copy the standalone Ebuild here:
https://github.com/jgehrig/gentoo/tree/master/app-editors/neovim-qt

Alternatively, you can add the entire overlay:
```
$ eselect repository add jgehrig git https://github.com/jgehrig/gentoo.git
$ emerge --sync
$ emerge -av neovim-qt
```

#### OpenSuse
A community maintained package can be found here:
https://build.opensuse.org/package/show/home%3AAptrug/neovim-qt

### Ubuntu
`apt-get install neovim-qt`


## Configuration

NeovimQt can be configured through the `ginit.vim` file.

The default locations are:
**Windows:** %LOCALAPPDATA%\nvim\ginit.vim
**MacOS:** ~/.config/nvim/ginit.vim
**Linux:** ~/.config/nvim/ginit.vim

Recommended `ginit.vim`
```
" Enable Mouse
set mouse=a

" Set Editor Font
if exists(':GuiFont')
	" Use GuiFont! to ignore font errors
	GuiFont {font_name}:h{size}
endif

" Disable GUI Tabline
if exists(':GuiTabline')
	GuiTabline 0
endif

" Disable GUI Popupmenu
if exists(':GuiPopupmenu')
	GuiPopupmenu 0
endif

" Enable GUI ScrollBar
if exists(':GuiScrollBar')
	GuiScrollBar 1
endif

" Right Click Context Menu (Copy-Cut-Paste)
nnoremap <silent><RightMouse> :call GuiShowContextMenu()<CR>
inoremap <silent><RightMouse> <Esc>:call GuiShowContextMenu()<CR>
vnoremap <silent><RightMouse> :call GuiShowContextMenu()<CR>gv

```

For more options, try `:help nvim_gui_shim` and scroll down to `Commands`.

More detailed information can be fond on the [Configuration Wiki Page](https://github.com/equalsraf/neovim-qt/wiki/Configuration-Options).


## Frequently Asked Questions

### Why are the :Gui... commands missing?
You need to load the NeovimQt runtime/plugin to register commands like `:GuiFont`.

You can manually specify the path with `NVIM_QT_RUNTIME_PATH`.

Alternatively, you can install the plugin/runtime:
`Plugin 'equalsraf/neovim-gui-shim`

In recent versions, run `nvim-qt --version` to check if the runtime is loaded:
```
$ nvim-qt --version
NVIM-QT v0.2.16.1
Build type: Release
Compilation:-march=native -O2 -pipe -Wall -Wextra -Wno-unused-parameter -Wunused-variable -std=c++11
Qt Version: 5.15.2
Environment:
  nvim: nvim
    args: --cmd let &rtp.=',/usr/share/nvim-qt/runtime' --cmd set termguicolors
      runtime: /usr/share/nvim-qt/runtime
...
```

Notice `runtime:` is non-empty and points to a folder containing `nvim_gui_shim.vim`.

### Why does :Gui... not work in init.vim?

The `:Gui...` commands such as `:GuiFont` are not available in init.vim. The Gui runtime shim has not been loaded yet in this context.

These options should be configured in `ginit.vim`. This file can be placed in the same directory as `init.vim`.

Alternatively, you can use the standard vim options such as`:set guifont=...` directly in `init.vim`.

### How do I disable the GUI Tabs?
`:GuiTabline 0`

To prevent startup flicker, see [Wiki - Configuration Options](https://github.com/equalsraf/neovim-qt/wiki/Configuration-Options)

### Why does the popup menu look different?
The popup/completion menu is rendered in Qt, instead of Neovim's canvas.

You can disable this feature:
`:GuiPopupmenu 0`

### How do I change the font?
`:GuiFont Fira Code:h12`
`:set guifont=Hack:h12`

### Why does guifont throw an error?

You may see these errors:
	- `{Font Name} is not a fixed pitch Font`
	- ` Warning: Font {Font Name} reports bad fixed pitch metrics`

You can override this warning with `:GuiFont! {Font Name}`.

The warnings are displayed to inform you your font is not a perfect monospace font. Some characters different widths.

If you know the font you've selected is monospace font, this is usually safe to ignore.

### Why is nvim unable to start?

The `nvim` binary must be in your `$PATH`.

You can manually provide a path to Neovim with `nvim-qt --nvim {path_to_nvim}`.

In recent versions, run `nvim-qt --version` to check which `nvim` binary is loaded:
```
$ nvim-qt --version
...
Environment:
  nvim: nvim
...
```


## Building From Source

Detailed build instructions can be found at the [Wiki](https://github.com/equalsraf/neovim-qt/wiki/Build).

Here is a simplified set of build commands:
```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build .
$ NVIM_QT_RUNTIME_PATH=../src/gui/runtime bin/nvim-qt
```

Note, the environment variable `NVIM_QT_RUNTIME` must be set for commands like `:GuiFont` to work.
