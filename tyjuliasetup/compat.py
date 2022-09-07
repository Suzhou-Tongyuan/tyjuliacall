"""This file is copied from CPython project:
    https://github.com/python/cpython/blob/0d04b8d9e1953d2311f78b772f21c9e07fbcbb6d/Lib/shlex.py

A lexical analyzer class for simple shell-like syntaxes."""
# Module and documentation by Eric S. Raymond, 21 Dec 1998
# Input stacking and error message cleanup added by ESR, March 2000
# push_source() and pop_source() made explicit by ESR, January 2001.
# Posix compliance, split(), string arguments, and
# iterator interface by Gustavo Niemeyer, April 2003.
# changes to tokenize more like Posix shells by Vinay Sajip, July 2016.

import sys

if sys.version_info < (3, 8):
    import re
    import shlex

    _find_unsafe = re.compile(r"[^\w@%+=:,./-]", re.ASCII).search

    def quote(s):
        """Return a shell-escaped version of the string *s*."""
        if not s:
            return "''"
        if _find_unsafe(s) is None:
            return s

        # use single quotes, and put single quotes into double quotes
        # the string $'b is then quoted as '$'"'"'b'
        return "'" + s.replace("'", "'\"'\"'") + "'"

    def join(split_command):
        """Return a shell-escaped string from *split_command*."""
        return " ".join(quote(arg) for arg in split_command)

    shlex.join = join
