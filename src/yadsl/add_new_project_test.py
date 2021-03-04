import add_new_project as m
import pytest

@pytest.mark.parametrize(
    "argv, args, kwargs", [
    (
        [],    # argv
        [], {} # args, kwargs
    ),
    (
        # List of positional arguments
        ['a', 'b', 'c'],
        ['a', 'b', 'c'], {}
    ),
    (
        # List of valid Python expressions
        ['%True%', '%0%', '%123+321%', '%set%', '%"set"%', "%'set'%"],
        [True, 0, 444, set, 'set', 'set'], {}
    ),
    (
        # List of character flag keyword arguments
        ['-abc', '-d', '-ee', '-fgg', '-gh'],
        [], {c: True for c in 'abcdefgh'}
    ),
    (
        # List of string flag keyword arguments
        ['--k', '--key', '--0'],
        [], {k: True for k in ['k', 'key', '0']}
    ),
    (
        # List of key-value keyword arguments
        ['--k=v', '--key=value', '--x=', '--y=10'],
        [], {'k': 'v', 'key': 'value', 'x': '', 'y': '10'}
    ),
    (
        # Mix of all types of arguments
        ['a', '-b', '--cd', 'd', '--e=%0%', '--f=g', '%True%'],
        ['a', 'd', True], {'b': True, 'cd': True, 'e': 0, 'f': 'g'}
    ),
    (
        # Previously initialized keyword arguments (not supported)
        ['--e', '--f=%10%', '--g=e', '--h=f'],
        [], {'e': True, 'f': 10, 'g': 'e', 'h': 'f'}
    ),
])
def test_process_argv(argv, args, kwargs):
    assert m.process_argv(argv) == (args, kwargs)