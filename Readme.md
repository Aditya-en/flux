  Flux - A Minimalist Terminal Text Editor in C

Flux
====

A minimalist terminal text editor written in C.

Flux is a small, educational project that explores the fundamental data structures and techniques used in real text editors, especially the **gap buffer**, a classic strategy for efficient text insertion and deletion.

The goal is simple:

> Build a practical text editor from scratch while keeping the code readable enough for anyone to learn from, modify, and extend.

This repository is intended as both a fun project and a hands-on guide for understanding how text editors manage text, cursor movement, and terminal rendering.

Features (Current)
------------------

*   Raw terminal mode using `termios`
*   Real-time character-by-character input
*   Gap buffer implementation for efficient editing
*   Multiple editable lines
*   Cursor navigation:
    *   Left / Right
    *   Up / Down
*   Backspace support
*   Screen rendering with ANSI escape sequences
*   `Ctrl + Q` to quit

Core Concepts Demonstrated
--------------------------

### Gap Buffer

A data structure widely used in text editors to make insertions and deletions near the cursor very fast.

### Raw Terminal Mode

Disables canonical input and echo so key presses are captured instantly.

### ANSI Escape Sequences

Used to clear the screen, move the cursor, and redraw the editor.

### Dynamic Memory Management

Buffers resize automatically as text grows.

### Multi-Line Text Representation

Each line maintains its own independent gap buffer.

Architecture
------------

    Text
     ├── lines[]           // Dynamic array of Line structures
     ├── num_lines
     └── active_line_index
    
    Line
     ├── dynamic_buffer    // Gap buffer storage
     ├── current_size
     ├── gap               // Size of the gap
     └── cursor            // Position of the cursor
    

Each line acts like its own tiny editable universe, with a movable empty pocket where new characters can land efficiently.

Build and Run
-------------

    gcc -Wall -Wextra -O2 -o flux main.c
    ./flux
    

Controls
--------

Key

Action

Printable characters

Insert text

Backspace

Delete character before cursor

Arrow Keys

Move cursor

Ctrl + Q

Quit editor

Why This Project Exists
-----------------------

Modern text editors can feel like towering cathedrals of software. Flux begins with a single brick.

By implementing one feature at a time, this project aims to show that:

*   Text editors are understandable
*   Data structures matter
*   Terminal programming is surprisingly approachable
*   Systems programming can be both educational and fun

Whether you're learning C, exploring terminal interfaces, or studying editor internals, Flux provides a compact and practical codebase to experiment with.

Planned Features
----------------

This is only the initial commit. Many improvements are planned, including:

*   Proper line insertion and deletion
*   Tab support
*   File opening and saving
*   Status bar
*   Syntax highlighting
*   Search functionality
*   Undo/redo
*   Better handling of terminal resizing
*   Scrolling for large files
*   Modular code organization

Flux is currently a sketchbook with working gears. More rooms will be added to the machine over time.

Contributing
------------

This project is intentionally educational and beginner-friendly.

Ideas, experiments, and pull requests are welcome. If you'd like to implement a feature or improve the code, feel free to fork the repository and build your own version.

Learning Resources
------------------

*   The Kilo Text Editor tutorial by Salvatore Sanfilippo
*   The Craft of Text Editing by Craig Finseth
*   POSIX `termios` documentation

License
-------

MIT License

Use it, study it, modify it, and build something wonderful.

Philosophy
----------

A text editor is, at heart, a conversation between a cursor and a buffer.

Flux is an attempt to make that conversation visible.
