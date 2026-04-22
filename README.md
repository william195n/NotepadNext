# Notepad Next

![Build Notepad Next](https://github.com/dail8859/NotepadNext/workflows/Build%20Notepad%20Next/badge.svg)

A cross-platform, reimplementation of Notepad++.

## Fork Information

This is an enhanced fork of the original [Notepad Next](https://github.com/dail8859/NotepadNext) project with additional features and improvements.

- **Original Repository**: https://github.com/dail8859/NotepadNext
- **This Fork**: https://github.com/william195n/NotepadNext

Though the application overall is stable and usable, it should not be considered safe for critically important work.

There are numerous bugs and half working implementations. Pull requests are greatly appreciated.

![screenshot](/doc/screenshot.png)

## Enhanced Features

This fork includes the following improvements over the original:

### Performance Optimizations
- **Smart Highlighting for Large Files**: Optimized performance with 150ms delay timer and visible-area-only highlighting for files >1MB
- **Memory Management**: Improved memory handling with std::unique_ptr in QRegexSearch and fixed MacroManager memory leaks
- **File Loader Refactoring**: Extracted FileLoader class for better separation of concerns (BOM detection, encoding, chunked reading)

### User Experience Enhancements
- **Case-Insensitive Smart Highlighting**: Smart highlighting now works regardless of case (e.g., selecting "ABc" highlights "abc", "ABC", etc.)
- **Auto-Completion Toggle**: Added option to enable/disable auto-completion (default: disabled)
- **Improved Save Dialog**: Changed save prompt to Yes/No/Cancel with Y/N/C keyboard shortcuts
- **Configurable Recent Files**: Maximum recent files count is now configurable (default: 10)
- **Configurable Auto-Completion Threshold**: Trigger threshold adjustable from 1-5 characters (default: 3)
- **Text Case Conversion**: Added UPPERCASE and lowercase options to right-click context menu
- **Session Restoration**: Enabled session restoration and snapshots by default
- **Wider Scrollbars**: Increased scrollbar width/height from 12px to 18px for better usability
- **New Tab Button**: Added '+' button at the end of tab bar for quick new file creation (equivalent to Ctrl+N)

### Stability & Safety
- **Large File Warning**: Added protection with configurable threshold (default: 100MB)
- **Session Bookmark Validation**: Type validation prevents crashes from corrupted session data
- **UTF-16 Support**: Automatic conversion to UTF-8 for both UTF-16 LE and BE files

### Developer Tools
- **Unified Error Handler**: Consistent error reporting with structured logging and user-friendly messages
- **Translation Debug Logging**: Comprehensive debug logging to diagnose Qt locale issues

# Installation

Packages are available for Windows, Linux, and MacOS.

Below are the supported distribution mechanisms. There may be other ways to download/install the application, but this project will likely not be able to offer any support for those since they are made available by other individuals.

## Windows
Windows packages are available as an installer or a stand-alone zip file on the [release](https://github.com/dail8859/NotepadNext/releases) page. The installer provides additional components such as an auto-updater and Windows context menu integration. You can easily install it with Winget:

```powershell
winget install dail8859.NotepadNext
```

## Linux
Linux packages can be obtained by downloading the stand-alone AppImage on the [release](https://github.com/dail8859/NotepadNext/releases) page or by installing the [flatpak](https://flathub.org/apps/details/com.github.dail8859.NotepadNext) by executing:

```bash
flatpak install flathub com.github.dail8859.NotepadNext
```

## MacOS
MacOS disk images can be downloaded from the [release](https://github.com/dail8859/NotepadNext/releases) page.

It can also be installed using brew:
```bash
brew tap dail8859/notepadnext
brew install --no-quarantine notepadnext
```

#### MacOS Tweaks

By default, MacOS enables font smoothing which causes text to appear quite differently from the Windows version. This can be disabled system-wide using the following command:

```bash
defaults -currentHost write -g AppleFontSmoothing -int 0
```

A restart is required for this to take effect.

# Development
Current development is done using QtCreator with the Microsoft Visual C++ (msvc) compiler. Qt 6.5 is the currently supported Qt version. Older versions of Qt are likely to work but are not tested. Any fixes for older versions will be accepted as long as they do not introduce complex fixes. This application is also known to build successfully on various Linux distributions and macOS. Other platforms/compilers should be usable with minor modifications.

If you are familiar with building C++ Qt desktop applications with Qt Creator, then this should be as simple as opening `CMakeLists` and build/run the project.

If you are new to building C++ Qt desktop applications, there is a more detailed guide [here](/doc/Building.md).


# License
This code is released under the [GNU General Public License version 3](https://www.gnu.org/licenses/gpl-3.0.txt).
