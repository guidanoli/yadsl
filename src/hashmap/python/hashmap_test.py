from pyhashmap import HashMap
import pytest

def test_empty():
    m = HashMap()
    with pytest.raises(RuntimeError):
        m.get('key')
    with pytest.raises(RuntimeError):
        m.remove('key')

def test_one_key():
    m = HashMap()
    m.add('key', 10)
    assert m.get('key') == 10
    with pytest.raises(RuntimeError):
        m.add('key', 10)
    with pytest.raises(RuntimeError):
        m.add('key', 20)
    m.remove('key')
    with pytest.raises(RuntimeError):
        m.get('key')
    with pytest.raises(RuntimeError):
        m.remove('key')
    m.add('key', 10)
    assert m.get('key') == 10

def test_multiple_keys():
    m = HashMap()
    for i in range(100):
        m.add(str(i), i)
    for i in range(100):
        assert m.get(str(i)) == i
    for i in range(100):
        m.remove(str(i))
    for i in range(100):
        with pytest.raises(RuntimeError):
            m.get(str(i))