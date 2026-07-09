# TIED: The Terminaly Ill editor

NO AI WAS USED IN THE MAKING OF THIS EDITOR! :D

This text editor is rubbish. 

This editor doesn't work with unicode.

it has windows support! :D
this editor works best on linux.


##Installation
```
    git clone https://github.com/schnerg/tied
    cd tied/
    make
    make install
```
make sure to change location of install in Makefile


##Usage
Normal Mode:
- o : new line and insert mode
- i : insert mode
- / : search: limitied to 40 chars
- ctrl + u : undo
- ctrl + r : redo
- ctrl + q : quit
- ctrl + s : save 
- ctrl + x : quit without saving

- ctrl + f : open file tree
- ctrl + h : switch to file tree mode

Insert Mode:
- esc = switch to normal mode

Filetree mode:
- ctrl + l : switch to normal mode
- ctrl + c : change directory to directory at cursor 
- ctrl + r : refresh file tree
- Enter    : switch to file or expand directory
 

##Contributing

Contributions are always welcome. :) 
