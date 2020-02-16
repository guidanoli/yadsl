from pygraph import Graph

def test_directed():
    g = Graph(directed=True)
    assert g.is_directed()

def test_undirected():
    g = Graph(directed=False)
    assert not g.is_directed()

def test_add_remove_vertex():
    def _test(g, o):
        g.add_vertex(o)
        assert o in g
        g.remove_vertex(o)
        assert o not in g
    g = Graph()
    _test(g, 42)
    _test(g, True)
    _test(g, 'string')
    _test(g, None)
    _test(g, g)

def test_recursion():
    g = Graph()
    class X:
        def __eq__(self, o):
            g.add_vertex(self)
    try:
        g.add_vertex(0)
        g.add_vertex(X())
    except RecursionError as e:
        return
    assert False